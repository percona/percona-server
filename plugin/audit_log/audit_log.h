/* Copyright (c) 2015-2016 Percona LLC and/or its affiliates. All rights reserved.

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

#ifndef AUDIT_LOG_H_INCLUDED
#define AUDIT_LOG_H_INCLUDED

#include <m_ctype.h>

extern uint64 audit_log_buffer_size_overflow;

#ifdef __cplusplus
extern "C" {
#endif

extern MYSQL_PLUGIN_IMPORT CHARSET_INFO *system_charset_info;

#ifdef __cplusplus
}
#endif

#define AUDIT_LOG_PSI_CATEGORY "audit_log"

#endif /* AUDIT_LOG_H_INCLUDED */
