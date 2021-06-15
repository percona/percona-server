#ifndef _PLUGIN_DATA_MASKING_PLUGIN_H
/* Copyright (c) 2018, 2019 Francisco Miguel Biete Banon. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA */

#define _PLUGIN_DATA_MASKING_PLUGIN_H

#include <string>
#include <unordered_map>
#include <vector>

#include "m_string.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "mysql/plugin.h"
#include "mysql/psi/mysql_thread.h"
#include "mysql/psi/mysql_memory.h"
#include "log.h"
#include "sql_plugin.h"

#define PAN_LENGTH_1 15
#define PAN_LENGTH_2 16
#define SSN_LENGTH 11

typedef std::unordered_map<std::string, std::vector<std::string>> t_mask_dict;

extern t_mask_dict *g_data_masking_dict;
extern mysql_rwlock_t g_data_masking_dict_rwlock;
extern void init_data_masking_memory();
extern void deinit_data_masking_memory();

// Returns false and populates the msg with an error if the module has not been initialized
extern bool data_masking_is_inited(char *msg, size_t msg_len);

#endif  //_PLUGIN_DATA_MASKING_PLUGIN_H
