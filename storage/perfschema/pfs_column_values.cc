/* Copyright (c) 2008, 2021, Oracle and/or its affiliates.

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

/**
  @file storage/perfschema/pfs_column_values.cc
  Literal values for columns used in the performance
  schema tables (implementation).
*/

#include "my_global.h"
#include "pfs_column_values.h"

LEX_STRING PERFORMANCE_SCHEMA_str=
{ C_STRING_WITH_LEN("performance_schema") };

LEX_STRING mutex_instrument_prefix=
{ C_STRING_WITH_LEN("wait/synch/mutex/") };

LEX_STRING rwlock_instrument_prefix=
{ C_STRING_WITH_LEN("wait/synch/rwlock/") };

LEX_STRING sxlock_instrument_prefix=
{ C_STRING_WITH_LEN("wait/synch/sxlock/") };

LEX_STRING cond_instrument_prefix=
{ C_STRING_WITH_LEN("wait/synch/cond/") };

LEX_STRING thread_instrument_prefix=
{ C_STRING_WITH_LEN("thread/") };

LEX_STRING file_instrument_prefix=
{ C_STRING_WITH_LEN("wait/io/file/") };

LEX_STRING stage_instrument_prefix=
{ C_STRING_WITH_LEN("stage/") };

LEX_STRING statement_instrument_prefix=
{ C_STRING_WITH_LEN("statement/") };

LEX_STRING transaction_instrument_prefix=
{ C_STRING_WITH_LEN("transaction") };

LEX_STRING socket_instrument_prefix=
{ C_STRING_WITH_LEN("wait/io/socket/") };

LEX_STRING memory_instrument_prefix=
{ C_STRING_WITH_LEN("memory/") };

