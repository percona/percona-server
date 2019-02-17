#ifndef _PLUGIN_DATA_MASKING_PLUGIN_MEMORY_H
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

#define _PLUGIN_DATA_MASKING_PLUGIN_MEMORY_H

#include "plugin/data_masking/include/plugin.h"

t_mask_dict *g_data_masking_dict;

PSI_rwlock_key key_data_masking_rwlock;
mysql_rwlock_t g_data_masking_dict_rwlock;

#ifdef HAVE_PSI_INTERFACE
PSI_memory_key key_memory_data_masking;
#endif  // HAVE_PSI_INTERFACE

void init_data_masking_memory();
void deinit_data_masking_memory();

#endif  // _PLUGIN_DATA_MASKING_PLUGIN_MEMORY_H
