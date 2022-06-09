/* Copyright (c) 2016 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include "filter.h"

#include <algorithm>
#include "audit_log.h"
#include "m_ctype.h"
#include "map_helpers.h"
#include "my_hostname.h"
#include "my_sys.h"
#include "my_user.h"
#include "mysql/psi/mysql_rwlock.h"
#include "mysql_com.h"

static std::string make_account_string(const char *user, size_t user_length,
                                       const char *host, size_t host_length) {
  std::string result(user, user_length);
  result.reserve(user_length + host_length + 2);
  result.append(1, '@');
  result.append(host, host_length);
  result.append(1, '\0');
  return result;
}

static std::string make_command_string(const char *name, size_t length) {
  std::string result(name, length);
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  result.shrink_to_fit();
  return result;
}

using account_set_t = collation_unordered_set<std::string>;

static account_set_t *include_accounts;
static account_set_t *exclude_accounts;

using db_set_t = collation_unordered_set<std::string>;

static db_set_t *include_databases;
static db_set_t *exclude_databases;

using command_set_t = malloc_unordered_set<std::string>;

static command_set_t *include_commands;
static command_set_t *exclude_commands;

#if defined(HAVE_PSI_INTERFACE)

static PSI_rwlock_key key_LOCK_account_list;
static PSI_rwlock_key key_LOCK_database_list;
static PSI_rwlock_key key_LOCK_command_list;
static PSI_rwlock_info all_rwlock_list[] = {
    {&key_LOCK_account_list, "audit_log_filter::account_list",
     PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME},
    {&key_LOCK_database_list, "audit_log_filter::database_list",
     PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME},
    {&key_LOCK_account_list, "audit_log_filter::command_list",
     PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME}};

#endif

static mysql_rwlock_t LOCK_account_list;
static mysql_rwlock_t LOCK_database_list;
static mysql_rwlock_t LOCK_command_list;

/*
  Allocate memory and initialize new command
*/

/*
  Remove enclosing quotes from string if any.
*/
static void unquote_string(char *string, size_t *string_length) noexcept {
  if (string[0] == '\'' && string[*string_length - 1] == '\'') {
    *string_length -= 2;
    memmove(string, string + 1, *string_length);
    string[*string_length] = 0;
  }
}

/*
  Parse comma-separated list of accounts and add it into account list.
  Empty user name is allowed.
*/
static void account_list_from_string(account_set_t *account_set,
                                     const char *string) {
  account_set->clear();

  char *string_copy = my_strdup(PSI_NOT_INSTRUMENTED, string, MYF(MY_FAE));
  const size_t string_length = strlen(string_copy);
  char *entry = string_copy;
  while (entry - string_copy < static_cast<ptrdiff_t>(string_length)) {
    size_t entry_length = 0;
    bool quote = false;

    while (*entry == ' ') entry++;

    entry_length = 0;
    while (
        ((entry[entry_length] != ' ' && entry[entry_length] != ',') || quote) &&
        entry[entry_length] != 0) {
      if (entry[entry_length] == '\'') quote = !quote;
      entry_length++;
    }

    entry[entry_length] = 0;

    char user[USERNAME_LENGTH + 1], host[HOSTNAME_LENGTH + 1];
    size_t user_length, host_length;
    parse_user(entry, entry_length, user, &user_length, host, &host_length);
    unquote_string(user, &user_length);
    unquote_string(host, &host_length);
    my_casedn_str(system_charset_info, host);

    account_set->emplace(
        make_account_string(user, user_length, host, host_length));

    entry += entry_length + 1;
  }

  my_free(string_copy);
}

static void database_list_from_string(db_set_t *db_set, const char *string) {
  const char *entry = string;

  db_set->clear();

  while (*entry) {
    while (*entry == ' ') entry++;

    size_t entry_length = 0;
    bool quote = false;
    char name[NAME_LEN + 1];
    size_t name_length = 0;
    while (
        ((entry[entry_length] != ' ' && entry[entry_length] != ',') || quote) &&
        entry[entry_length] != 0) {
      if (quote && entry[entry_length] == '`' &&
          entry[entry_length + 1] == '`') {
        name[name_length++] = '`';
        entry_length += 1;
      } else if (entry[entry_length] == '`')
        quote = !quote;
      else if (name_length < sizeof(name))
        name[name_length++] = entry[entry_length];
      entry_length++;
    }

    if (name_length > 0) {
      name[name_length] = 0;
      db_set->emplace(name, name_length);
    }

    entry += entry_length;

    if (*entry == ',') entry++;
  }
}

/*
  Parse comma-separated list of command and add it into command hash.
*/
static void command_list_from_string(command_set_t *command_set,
                                     const char *string) {
  std::string lcase_str(string);
  std::transform(lcase_str.begin(), lcase_str.end(), lcase_str.begin(),
                 ::tolower);

  command_set->clear();

  auto it = lcase_str.cbegin();
  while (it != lcase_str.cend()) {
    std::string::size_type len = 0;
    while (it != lcase_str.cend() && (*it == ' ' || *it == ',')) it++;
    while (it + len != lcase_str.cend() && it[len] != ' ' && it[len] != ',')
      len++;
    if (len > 0) {
      command_set->emplace(&(*it), len);
      it += len;
    }
  }
}

/* public interface */

void audit_log_filter_init() {
#ifdef HAVE_PSI_INTERFACE
  mysql_rwlock_register(AUDIT_LOG_PSI_CATEGORY, all_rwlock_list,
                        array_elements(all_rwlock_list));
#endif /* HAVE_PSI_INTERFACE */
  mysql_rwlock_init(key_LOCK_account_list, &LOCK_account_list);
  mysql_rwlock_init(key_LOCK_database_list, &LOCK_database_list);
  mysql_rwlock_init(key_LOCK_command_list, &LOCK_command_list);

  include_accounts =
      new account_set_t(&my_charset_bin, key_memory_audit_log_accounts);

  exclude_accounts =
      new account_set_t(&my_charset_bin, key_memory_audit_log_accounts);

  include_databases =
      new db_set_t(&my_charset_bin, key_memory_audit_log_databases);

  exclude_databases =
      new db_set_t(&my_charset_bin, key_memory_audit_log_databases);

  include_commands = new command_set_t(key_memory_audit_log_commands);

  exclude_commands = new command_set_t(key_memory_audit_log_commands);
}

void audit_log_filter_destroy() noexcept {
  delete include_accounts;
  delete exclude_accounts;
  delete include_databases;
  delete exclude_databases;
  delete include_commands;
  delete exclude_commands;
  mysql_rwlock_destroy(&LOCK_account_list);
  mysql_rwlock_destroy(&LOCK_database_list);
  mysql_rwlock_destroy(&LOCK_account_list);
  mysql_rwlock_destroy(&LOCK_command_list);
}

/*
  Parse and store the list of included accounts.
*/
void audit_log_set_include_accounts(const char *val) {
  mysql_rwlock_wrlock(&LOCK_account_list);
  account_list_from_string(include_accounts, val);
  mysql_rwlock_unlock(&LOCK_account_list);
}

/*
  Parse and store the list of excluded accounts.
*/
void audit_log_set_exclude_accounts(const char *val) {
  mysql_rwlock_wrlock(&LOCK_account_list);
  account_list_from_string(exclude_accounts, val);
  mysql_rwlock_unlock(&LOCK_account_list);
}

/*
  Check if account has to be included.
*/
bool audit_log_check_account_included(const char *user, size_t user_length,
                                      const char *host, size_t host_length) {
  const std::string acc{
      make_account_string(user, user_length, host, host_length)};

  mysql_rwlock_rdlock(&LOCK_account_list);
  const auto &it = include_accounts->find(acc);
  const bool res = it != include_accounts->cend();
  mysql_rwlock_unlock(&LOCK_account_list);

  return res;
}

/*
  Check if account has to be excluded.
*/
bool audit_log_check_account_excluded(const char *user, size_t user_length,
                                      const char *host, size_t host_length) {
  const std::string acc{
      make_account_string(user, user_length, host, host_length)};

  mysql_rwlock_rdlock(&LOCK_account_list);
  const auto &it = exclude_accounts->find(acc);
  const bool res = it != exclude_accounts->cend();
  ;
  mysql_rwlock_unlock(&LOCK_account_list);

  return res;
}

/*
  Parse and store the list of included databases.
*/
void audit_log_set_include_databases(const char *val) {
  mysql_rwlock_wrlock(&LOCK_database_list);
  database_list_from_string(include_databases, val);
  mysql_rwlock_unlock(&LOCK_database_list);
}

/*
  Parse and store the list of excluded databases.
*/
void audit_log_set_exclude_databases(const char *val) {
  mysql_rwlock_wrlock(&LOCK_database_list);
  database_list_from_string(exclude_databases, val);
  mysql_rwlock_unlock(&LOCK_database_list);
}

/*
  Check if database has to be included.
*/
bool audit_log_check_database_included(const char *name, size_t length) {
  if (length == 0) return false;

  const std::string db(name, length);

  mysql_rwlock_rdlock(&LOCK_database_list);
  const auto &it = include_databases->find(db);
  const bool res = it != include_databases->cend();
  mysql_rwlock_unlock(&LOCK_database_list);

  return res;
}

/*
  Check if database has to be excluded.
*/
bool audit_log_check_database_excluded(const char *name, size_t length) {
  if (length == 0) return false;

  const std::string db(name, length);

  mysql_rwlock_rdlock(&LOCK_database_list);
  const auto &it = exclude_databases->find(db);
  const bool res = it != exclude_databases->cend();
  mysql_rwlock_unlock(&LOCK_database_list);

  return res;
}

/*
  Parse and store the list of included commands.
*/
void audit_log_set_include_commands(const char *val) {
  mysql_rwlock_wrlock(&LOCK_command_list);
  command_list_from_string(include_commands, val);
  mysql_rwlock_unlock(&LOCK_command_list);
}

/*
  Parse and store the list of excluded commands.
*/
void audit_log_set_exclude_commands(const char *val) {
  mysql_rwlock_wrlock(&LOCK_command_list);
  command_list_from_string(exclude_commands, val);
  mysql_rwlock_unlock(&LOCK_command_list);
}

/*
  Check if command has to be included.
*/
bool audit_log_check_command_included(const char *name, size_t length) {
  if (length == 0) return false;

  const std::string cmd{make_command_string(name, length)};

  mysql_rwlock_rdlock(&LOCK_command_list);
  const auto &it = include_commands->find(cmd);
  const bool res = it != include_commands->cend();
  mysql_rwlock_unlock(&LOCK_command_list);

  return res;
}

/*
  Check if command has to be excluded.
*/
bool audit_log_check_command_excluded(const char *name, size_t length) {
  if (length == 0) return false;

  const std::string cmd{make_command_string(name, length)};

  mysql_rwlock_rdlock(&LOCK_command_list);
  const auto &it = exclude_commands->find(cmd);
  const bool res = it != exclude_commands->cend();
  mysql_rwlock_unlock(&LOCK_command_list);

  return res;
}
