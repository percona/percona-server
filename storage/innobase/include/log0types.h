/*****************************************************************************

Copyright (c) 2013, 2021, Oracle and/or its affiliates.

Portions of this file contain modifications contributed and copyrighted by
Google, Inc. Those modifications are gratefully acknowledged and are described
briefly in the InnoDB documentation. The contributions by Google are
incorporated with their permission, and subject to the conditions contained in
the file COPYING.Google.

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

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA

*****************************************************************************/

/**************************************************//**
@file include/log0types.h
Log types

Created 2013-03-15 Sunny Bains
*******************************************************/

#ifndef log0types_h
#define log0types_h

#include "univ.i"

/* Type used for all log sequence number storage and arithmetics */
typedef	ib_uint64_t		lsn_t;

#define LSN_MAX			IB_UINT64_MAX

#define LSN_PF			UINT64PF

/** The redo log manager */
struct RedoLog;

/** The recovery implementation */
struct redo_recover_t;

/** Hashed page file address struct */
struct recv_addr_t;

#endif /* log0types_h */
