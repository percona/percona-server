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

#include <my_global.h>
#include <my_sys.h>
#include <my_user.h>
#include <m_ctype.h>
#include <mysql_com.h>
#include <hash.h>
#include <string.h>
#include <stdlib.h>

#include "filter.h"
#include "audit_log.h"

typedef struct
{
  /* user + '@' + host + '\0' */
  char name[USERNAME_LENGTH + HOSTNAME_LENGTH + 2];
  size_t length;
} account;

typedef struct
{
  char name[NAME_LEN + 1];
  size_t length;
} database;

typedef struct
{
  /* has to be enought to hold one of the com_status_vars names */
  char name[100];
  size_t length;
} command;

static HASH include_accounts;
static HASH exclude_accounts;

static HASH include_databases;
static HASH exclude_databases;

static HASH include_commands;
static HASH exclude_commands;

#if defined(HAVE_PSI_INTERFACE)

static PSI_rwlock_key key_LOCK_account_list;
static PSI_rwlock_key key_LOCK_database_list;
static PSI_rwlock_key key_LOCK_command_list;
static PSI_rwlock_info all_rwlock_list[]=
{{ &key_LOCK_account_list, "audit_log_filter::account_list", PSI_FLAG_GLOBAL},
 { &key_LOCK_database_list, "audit_log_filter::database_list", PSI_FLAG_GLOBAL},
 { &key_LOCK_account_list, "audit_log_filter::command_list", PSI_FLAG_GLOBAL}};

#endif

static mysql_rwlock_t LOCK_account_list;
static mysql_rwlock_t LOCK_database_list;
static mysql_rwlock_t LOCK_command_list;

/*
  Initialize account
*/
static
void account_init(account *acc, const char *user, size_t user_length,
                                const char *host, size_t host_length)
{
  DBUG_ASSERT(user_length + host_length + 2 <= sizeof(acc->name));

  memcpy(acc->name, user, user_length);
  memcpy(acc->name + user_length + 1, host, host_length);
  acc->name[user_length]= '@';
  acc->name[user_length + host_length + 1]= 0;
  acc->length= user_length + host_length + 1;
}

/*
  Allocate memory and initialize new account
*/

static
account *account_create(const char *user, size_t user_length,
                        const char *host, size_t host_length)
{
  account *acc= (account *) my_malloc(key_memory_audit_log_accounts,
                                      sizeof(account), MYF(MY_FAE));

  account_init(acc, user, user_length, host, host_length);

  return acc;
}

/*
  Get account key
*/
static
uchar *account_get_key(const account *acc, size_t *length,
                       my_bool not_used MY_ATTRIBUTE((unused)))
{
  *length= acc->length;
  return (uchar*) acc->name;
}

/*
  Initialize database
*/
static
void database_init(database *db, const char *name, size_t length)
{
  DBUG_ASSERT(length + 1 <= sizeof(db->name));

  memcpy(db->name, name, length);
  db->name[length]= 0;
  db->length= length;
}

/*
  Allocate memory and initialize new database
*/

static
database *database_create(const char *name, size_t length)
{
  database *db= (database *) my_malloc(key_memory_audit_log_databases,
                                       sizeof(database), MYF(MY_FAE));

  database_init(db, name, length);

  return db;
}

/*
  Get database key
*/
static
uchar *database_get_key(const database *db, size_t *length,
                        my_bool not_used MY_ATTRIBUTE((unused)))
{
  *length= db->length;
  return (uchar*) db->name;
}

/*
  Initialize command
*/
static
void command_init(command *cmd, const char *name, size_t length)
{
  DBUG_ASSERT(length + 1 <= sizeof(cmd->name));

  memcpy(cmd->name, name, length);
  cmd->name[length]= 0;
  cmd->length= length;
}

/*
  Allocate memory and initialize new command
*/

static
command *command_create(const char *name, size_t length)
{
  command *cmd= (command *) my_malloc(key_memory_audit_log_commands,
                                      sizeof(command), MYF(MY_FAE));

  command_init(cmd, name, length);

  return cmd;
}

/*
  Get command key
*/
static
uchar *command_get_key(const command *acc, size_t *length,
                       my_bool not_used MY_ATTRIBUTE((unused)))
{
  *length= acc->length;
  return (uchar*) acc->name;
}

/*
  Remove enclosing quotes from string if any.
*/
static
void unquote_string(char *string, size_t *string_length)
{
  if (string[0] == '\'' && string[*string_length - 1] == '\'')
  {
    *string_length-= 2;
    memmove(string, string + 1, *string_length);
    string[*string_length]= 0;
  }
}

/*
  Parse comma-separated list of accounts and add it into account list.
  Empty user name is allowed.
*/
static
void account_list_from_string(HASH *hash, const char *string)
{
  char *string_copy= my_strdup(PSI_NOT_INSTRUMENTED, string, MYF(MY_FAE));
  char *entry= string_copy;
  int string_length= strlen(string_copy);
  char user[USERNAME_LENGTH + 1], host[HOSTNAME_LENGTH + 1];
  size_t user_length, host_length;

  my_hash_reset(hash);

  while (entry - string_copy < string_length)
  {
    size_t entry_length= 0;
    my_bool quote= FALSE;
    account *acc;

    while (*entry == ' ')
      entry++;

    entry_length= 0;
    while (((entry[entry_length] != ' ' && entry[entry_length] != ',') || quote)
           && entry[entry_length] != 0)
    {
      if (entry[entry_length] == '\'')
        quote= !quote;
      entry_length++;
    }

    entry[entry_length]= 0;

    parse_user(entry, entry_length, user, &user_length, host, &host_length);
    unquote_string(user, &user_length);
    unquote_string(host, &host_length);
    my_casedn_str(system_charset_info, host);

    acc= account_create(user, user_length, host, host_length);
    if (my_hash_insert(hash, (uchar*) acc))
      my_free(acc);

    entry+= entry_length + 1;
  }

  my_free(string_copy);
}

static
void database_list_from_string(HASH *hash, const char *string)
{
  const char *entry= string;

  my_hash_reset(hash);

  while (*entry)
  {
    size_t entry_length= 0;
    my_bool quote= FALSE;
    char name[NAME_LEN + 1];
    size_t name_length= 0;

    while (*entry == ' ')
      entry++;

    while (((entry[entry_length] != ' ' && entry[entry_length] != ',') || quote)
           && entry[entry_length] != 0)
    {
      if (quote && entry[entry_length] == '`' && entry[entry_length + 1] == '`')
      {
        name[name_length++]= '`';
        entry_length+= 1;
      }
      else if (entry[entry_length] == '`')
        quote= !quote;
      else if (name_length < sizeof(name))
        name[name_length++]= entry[entry_length];
      entry_length++;
    }

    if (name_length > 0)
    {
      database *db;
      name[name_length]= 0;
      db= database_create(name, name_length);
      if (my_hash_insert(hash, (uchar*) db))
        my_free(db);
    }

    entry+= entry_length;

    if (*entry == ',')
      entry++;
  }
}

/*
  Parse comma-separated list of command and add it into command hash.
*/
static
void command_list_from_string(HASH *hash, const char *string)
{
  const char *entry= string;

  my_hash_reset(hash);

  while (*entry)
  {
    size_t len= 0;

    while (*entry == ' ' || *entry == ',')
      entry++;

    while (entry[len] != ' ' && entry[len] != ',' && entry[len] != 0)
      len++;

    if (len > 0)
    {
      command *cmd= command_create(entry, len);
      my_casedn_str(&my_charset_utf8_general_ci, cmd->name);
      if (my_hash_insert(hash, (uchar*) cmd))
        my_free(cmd);
    }

    entry+= len;
  }
}

/* public interface */

void audit_log_filter_init()
{
#ifdef HAVE_PSI_INTERFACE
  mysql_rwlock_register(AUDIT_LOG_PSI_CATEGORY, all_rwlock_list,
                        array_elements(all_rwlock_list));
#endif /* HAVE_PSI_INTERFACE */
  mysql_rwlock_init(key_LOCK_account_list, &LOCK_account_list);
  mysql_rwlock_init(key_LOCK_database_list, &LOCK_database_list);
  mysql_rwlock_init(key_LOCK_command_list, &LOCK_command_list);

  my_hash_init(&include_accounts, &my_charset_bin,
               20, 0, 0,
               (my_hash_get_key) account_get_key,
               my_free, HASH_UNIQUE, key_memory_audit_log_accounts);

  my_hash_init(&exclude_accounts, &my_charset_bin,
               20, 0, 0,
               (my_hash_get_key) account_get_key,
               my_free, HASH_UNIQUE, key_memory_audit_log_accounts);

  my_hash_init(&include_databases, &my_charset_bin,
               20, 0, 0,
               (my_hash_get_key) database_get_key,
               my_free, HASH_UNIQUE, key_memory_audit_log_databases);

  my_hash_init(&exclude_databases, &my_charset_bin,
               20, 0, 0,
               (my_hash_get_key) database_get_key,
               my_free, HASH_UNIQUE, key_memory_audit_log_databases);

  my_hash_init(&include_commands, &my_charset_bin,
               20, 0, 0,
               (my_hash_get_key) command_get_key,
               my_free, HASH_UNIQUE, key_memory_audit_log_commands);

  my_hash_init(&exclude_commands, &my_charset_bin,
               20, 0, 0,
               (my_hash_get_key) command_get_key,
               my_free, HASH_UNIQUE, key_memory_audit_log_commands);
}

void audit_log_filter_destroy()
{
  my_hash_free(&include_accounts);
  my_hash_free(&exclude_accounts);
  my_hash_free(&include_databases);
  my_hash_free(&exclude_databases);
  my_hash_free(&include_commands);
  my_hash_free(&exclude_commands);
  mysql_rwlock_destroy(&LOCK_account_list);
  mysql_rwlock_destroy(&LOCK_database_list);
  mysql_rwlock_destroy(&LOCK_account_list);
  mysql_rwlock_destroy(&LOCK_command_list);
}

/*
  Parse and store the list of included accounts.
*/
void audit_log_set_include_accounts(const char *val)
{
  mysql_rwlock_wrlock(&LOCK_account_list);
  account_list_from_string(&include_accounts, val);
  mysql_rwlock_unlock(&LOCK_account_list);
}

/*
  Parse and store the list of excluded accounts.
*/
void audit_log_set_exclude_accounts(const char *val)
{
  mysql_rwlock_wrlock(&LOCK_account_list);
  account_list_from_string(&exclude_accounts, val);
  mysql_rwlock_unlock(&LOCK_account_list);
}

/*
  Check if account has to be included.
*/
my_bool audit_log_check_account_included(const char *user, size_t user_length,
                                         const char *host, size_t host_length)
{
  account acc;
  my_bool res;

  account_init(&acc, user, user_length, host, host_length);

  if (acc.length == 0)
    return FALSE;

  mysql_rwlock_rdlock(&LOCK_account_list);

  res= my_hash_search(&include_accounts,
                      (const uchar*) acc.name, acc.length) != NULL;

  mysql_rwlock_unlock(&LOCK_account_list);
  return res;
}

/*
  Check if account has to be excluded.
*/
my_bool audit_log_check_account_excluded(const char *user, size_t user_length,
                                         const char *host, size_t host_length)
{
  account acc;
  my_bool res;

  account_init(&acc, user, user_length, host, host_length);

  if (acc.length == 0)
    return FALSE;

  mysql_rwlock_rdlock(&LOCK_account_list);

  res= my_hash_search(&exclude_accounts,
                      (const uchar*) acc.name, acc.length) != NULL;
  mysql_rwlock_unlock(&LOCK_account_list);
  return res;
}

/*
  Parse and store the list of included databases.
*/
void audit_log_set_include_databases(const char *val)
{
  mysql_rwlock_wrlock(&LOCK_database_list);
  database_list_from_string(&include_databases, val);
  mysql_rwlock_unlock(&LOCK_database_list);
}

/*
  Parse and store the list of excluded databases.
*/
void audit_log_set_exclude_databases(const char *val)
{
  mysql_rwlock_wrlock(&LOCK_database_list);
  database_list_from_string(&exclude_databases, val);
  mysql_rwlock_unlock(&LOCK_database_list);
}

/*
  Check if database has to be included.
*/
my_bool audit_log_check_database_included(const char *name, size_t length)
{
  my_bool res;

  if (length == 0)
    return FALSE;

  mysql_rwlock_rdlock(&LOCK_database_list);

  res= my_hash_search(&include_databases,
                      (const uchar*) name, length) != NULL;

  mysql_rwlock_unlock(&LOCK_database_list);
  return res;
}

/*
  Check if database has to be excluded.
*/
my_bool audit_log_check_database_excluded(const char *name, size_t length)
{
  my_bool res;

  if (length == 0)
    return FALSE;

  mysql_rwlock_rdlock(&LOCK_database_list);

  res= my_hash_search(&exclude_databases,
                      (const uchar*) name, length) != NULL;
  mysql_rwlock_unlock(&LOCK_database_list);
  return res;
}


/*
  Parse and store the list of included commands.
*/
void audit_log_set_include_commands(const char *val)
{
  mysql_rwlock_wrlock(&LOCK_command_list);
  command_list_from_string(&include_commands, val);
  mysql_rwlock_unlock(&LOCK_command_list);
}

/*
  Parse and store the list of excluded commands.
*/
void audit_log_set_exclude_commands(const char *val)
{
  mysql_rwlock_wrlock(&LOCK_command_list);
  command_list_from_string(&exclude_commands, val);
  mysql_rwlock_unlock(&LOCK_command_list);
}

/*
  Check if command has to be included.
*/
my_bool audit_log_check_command_included(const char *name, size_t length)
{
  my_bool res;

  if (length == 0)
    return FALSE;

  mysql_rwlock_rdlock(&LOCK_command_list);
  res= my_hash_search(&include_commands, (const uchar*) name, length) != NULL;
  mysql_rwlock_unlock(&LOCK_command_list);

  return res;
}

/*
  Check if command has to be excluded.
*/
my_bool audit_log_check_command_excluded(const char *name, size_t length)
{
  my_bool res;

  if (length == 0)
    return FALSE;

  mysql_rwlock_rdlock(&LOCK_command_list);
  res= my_hash_search(&exclude_commands, (const uchar*) name, length) != NULL;
  mysql_rwlock_unlock(&LOCK_command_list);

  return res;
}
