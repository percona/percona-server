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

#ifndef AUDIT_LOG_FILTER_PSI_INFO_H_INCLUDED
#define AUDIT_LOG_FILTER_PSI_INFO_H_INCLUDED

#include "mysql/psi/psi_memory.h"

namespace audit_log_filter {

extern PSI_memory_key key_memory_audit_log_filter_logger_handle;
extern PSI_memory_key key_memory_audit_log_filter_handler;
extern PSI_memory_key key_memory_audit_log_filter_buffer;
extern PSI_memory_key key_memory_audit_log_filter_read_buffer;
extern PSI_memory_key key_memory_audit_log_filter_accounts;
extern PSI_memory_key key_memory_audit_log_filter_databases;
extern PSI_memory_key key_memory_audit_log_filter_commands;
extern PSI_memory_key key_memory_audit_log_filter_password_buffer;

static const constexpr auto AUDIT_LOG_FILTER_PSI_CATEGORY = "audit_filter";

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_PSI_INFO_H_INCLUDED
