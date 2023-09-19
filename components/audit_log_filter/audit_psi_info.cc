/* Copyright (c) 2022 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include "components/audit_log_filter/audit_psi_info.h"

namespace audit_log_filter {

PSI_memory_key key_memory_audit_log_filter_logger_handle;
PSI_memory_key key_memory_audit_log_filter_handler;
PSI_memory_key key_memory_audit_log_filter_buffer;
PSI_memory_key key_memory_audit_log_filter_read_buffer;
PSI_memory_key key_memory_audit_log_filter_accounts;
PSI_memory_key key_memory_audit_log_filter_databases;
PSI_memory_key key_memory_audit_log_filter_commands;
PSI_memory_key key_memory_audit_log_filter_password_buffer;

PSI_memory_info *get_all_memory_info() noexcept {
  static PSI_memory_info all_audit_log_filter_memory[] = {
      {&key_memory_audit_log_filter_logger_handle,
       "audit_log_filter_logger_handle", PSI_FLAG_ONLY_GLOBAL_STAT,
       PSI_VOLATILITY_UNKNOWN, PSI_DOCUMENT_ME},
      {&key_memory_audit_log_filter_handler, "audit_log_filter_handler",
       PSI_FLAG_ONLY_GLOBAL_STAT, PSI_VOLATILITY_UNKNOWN, PSI_DOCUMENT_ME},
      {&key_memory_audit_log_filter_buffer, "audit_log_filter_buffer",
       PSI_FLAG_ONLY_GLOBAL_STAT, PSI_VOLATILITY_UNKNOWN, PSI_DOCUMENT_ME},
      {&key_memory_audit_log_filter_read_buffer, "audit_log_filter_read_buffer",
       PSI_FLAG_ONLY_GLOBAL_STAT, PSI_VOLATILITY_UNKNOWN, PSI_DOCUMENT_ME},
      {&key_memory_audit_log_filter_accounts, "audit_log_filter_accounts",
       PSI_FLAG_ONLY_GLOBAL_STAT, PSI_VOLATILITY_UNKNOWN, PSI_DOCUMENT_ME},
      {&key_memory_audit_log_filter_databases, "audit_log_filter_databases",
       PSI_FLAG_ONLY_GLOBAL_STAT, PSI_VOLATILITY_UNKNOWN, PSI_DOCUMENT_ME},
      {&key_memory_audit_log_filter_commands, "audit_log_filter_commands",
       PSI_FLAG_ONLY_GLOBAL_STAT, PSI_VOLATILITY_UNKNOWN, PSI_DOCUMENT_ME},
      {&key_memory_audit_log_filter_password_buffer,
       "audit_log_filter_password_buffer", PSI_FLAG_ONLY_GLOBAL_STAT,
       PSI_VOLATILITY_UNKNOWN, PSI_DOCUMENT_ME},
  };

  return all_audit_log_filter_memory;
}

}  // namespace audit_log_filter
