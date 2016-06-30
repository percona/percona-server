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

static HASH include_accounts;
static HASH exclude_accounts;

#if defined(HAVE_PSI_INTERFACE)

static PSI_rwlock_key key_LOCK_account_list;
static PSI_rwlock_info all_rwlock_list[]=
{{ &key_LOCK_account_list, "audit_log_filter::account_list", PSI_FLAG_GLOBAL}};

#endif

mysql_rwlock_t LOCK_account_list;

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
uchar *account_get_key(const account *acc, size_t *length,
                       my_bool not_used __attribute__((unused)))
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

    my_hash_insert(hash,
        (uchar*) account_create(user, user_length, host, host_length));

    entry+= entry_length + 1;
  }

  my_free(string_copy);
}

/* public interface */

void audit_log_filter_init()
{
#ifdef HAVE_PSI_INTERFACE
  mysql_rwlock_register(AUDIT_LOG_PSI_CATEGORY, all_rwlock_list,
                        array_elements(all_rwlock_list));
#endif /* HAVE_PSI_INTERFACE */
  mysql_rwlock_init(key_LOCK_account_list, &LOCK_account_list);

  my_hash_init(&include_accounts, &my_charset_utf8_general_ci,
               20, 0, 0,
               (my_hash_get_key) account_get_key,
               my_free, HASH_UNIQUE, key_memory_audit_log_accounts);

  my_hash_init(&exclude_accounts, &my_charset_utf8_general_ci,
               20, 0, 0,
               (my_hash_get_key) account_get_key,
               my_free, HASH_UNIQUE, key_memory_audit_log_accounts);
}

void audit_log_filter_destroy()
{
  my_hash_free(&include_accounts);
  my_hash_free(&exclude_accounts);
  mysql_rwlock_destroy(&LOCK_account_list);
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

  mysql_rwlock_rdlock(&LOCK_account_list);

  res= my_hash_search(&exclude_accounts,
                      (const uchar*) acc.name, acc.length) != NULL;
  mysql_rwlock_unlock(&LOCK_account_list);
  return res;
}
