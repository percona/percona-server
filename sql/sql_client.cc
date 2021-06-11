/* Copyright (c) 2003, 2021, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA */

/*
  This files defines some MySQL C API functions that are server specific
*/

#include "sql_class.h"                          // system_variables

#include <algorithm>

using std::min;
using std::max;

/*
  Function called by my_net_init() to set some check variables
*/

extern "C" {
void my_net_local_init(NET *net)
{
#ifndef EMBEDDED_LIBRARY
  net->max_packet=   (uint) global_system_variables.net_buffer_length;

  net->read_timeout= net->write_timeout= 0;
  my_net_set_read_timeout(net, (uint)global_system_variables.net_read_timeout);
  my_net_set_write_timeout(net,
                           (uint)global_system_variables.net_write_timeout);

  net->retry_count=  (uint) global_system_variables.net_retry_count;
  net->max_packet_size= max<size_t>(global_system_variables.net_buffer_length,
                                    global_system_variables.max_allowed_packet);
#endif
}
}
