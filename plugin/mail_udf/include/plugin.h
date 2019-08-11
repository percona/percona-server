#ifndef _PLUGIN_MAIL_PLUGIN_H
/* Copyright (c) 2019 Francisco Miguel Biete Banon. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA
*/
#define _PLUGIN_MAIL_PLUGIN_H

#include <string>

#include "m_string.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "mysql/plugin.h"
#include "sql/log.h"
#include "sql/sql_plugin.h"

#define LOG_COMPONENT_TAG "mail_udf"

#include "mysql/components/services/log_builtins.h"
#include "mysqld_error.h"

#include <curl/curl.h>

#endif  //_PLUGIN_MAIL_PLUGIN_H
