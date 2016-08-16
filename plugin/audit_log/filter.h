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

#ifndef AUDIT_LOG_FILTER_INCLUDED
#define AUDIT_LOG_FILTER_INCLUDED

#include <my_global.h>

#ifdef __cplusplus
extern "C" {
#endif

void audit_log_set_include_accounts(const char *val);
void audit_log_set_exclude_accounts(const char *val);
my_bool audit_log_check_account_included(const char *user, size_t user_length,
                                         const char *host, size_t host_length);
my_bool audit_log_check_account_excluded(const char *user, size_t user_length,
                                         const char *host, size_t host_length);

void audit_log_set_include_commands(const char *val);
void audit_log_set_exclude_commands(const char *val);
my_bool audit_log_check_command_included(const char *command, size_t length);
my_bool audit_log_check_command_excluded(const char *command, size_t length);

void audit_log_filter_init();
void audit_log_filter_destroy();

#ifdef __cplusplus
}
#endif

#endif
