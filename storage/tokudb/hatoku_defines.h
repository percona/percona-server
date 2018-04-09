/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
// vim: ft=cpp:expandtab:ts=8:sw=4:softtabstop=4:
#ident "$Id$"
/*======
This file is part of TokuDB


Copyright (c) 2006, 2015, Percona and/or its affiliates. All rights reserved.

    TokuDB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2,
    as published by the Free Software Foundation.

    TokuDB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with TokuDB.  If not, see <http://www.gnu.org/licenses/>.

======= */

#ident "Copyright (c) 2006, 2015, Percona and/or its affiliates. All rights reserved."

#ifndef _HATOKU_DEFINES_H
#define _HATOKU_DEFINES_H

#include <my_config.h>
#define MYSQL_SERVER 1
#include "mysql_version.h"
#include "sql_table.h"
#include "handler.h"
#include "table.h"
#include "log.h"
#include "sql_class.h"
#include "sql_show.h"
#include "item_cmpfunc.h"
#include <binlog.h>
#include "debug_sync.h"

#undef PACKAGE
#undef VERSION
#undef HAVE_DTRACE
#undef _DTRACE_VERSION

/* We define DTRACE after mysql_priv.h in case it disabled dtrace in the main server */
#ifdef HAVE_DTRACE
#define _DTRACE_VERSION 1
#else
#endif

#include <mysql/plugin.h>

#include <ctype.h>
#include <stdint.h>
#if !defined(__STDC_FORMAT_MACROS)
#define __STDC_FORMAT_MACROS
#endif  // !defined(__STDC_FORMAT_MACROS)
#include <inttypes.h>
#if defined(_WIN32)
#include "misc.h"
#endif

#include <string>
#include <unordered_map>

#include "db.h"
#include "toku_os.h"
#include "toku_time.h"
#include "partitioned_counter.h"

#ifdef USE_PRAGMA_INTERFACE
#pragma interface               /* gcc class implementation */
#endif

// TOKU_INCLUDE_WRITE_FRM_DATA and TOKU_INCLUDE_DISCOVER_FRM work together as
// two opposing sides of the same functionality. The 'WRITE' includes
// functionality to write a copy of every tables .frm data into the tables
// status dictionary on CREATE or ALTER. When WRITE is in, the .frm data is
// also verified whenever a table is opened.
//
// The 'DISCOVER' then implements the MySQL table discovery API which reads
// this same data and returns it back to MySQL.
// In most cases, they should all be in or out without mixing. There may be
// extreme cases though where one side (WRITE) is supported but perhaps
// 'DISCOVERY' may not be, thus the need for individual indicators.
#define TOKU_USE_DB_TYPE_TOKUDB 1           // has DB_TYPE_TOKUDB patch
#define TOKU_INCLUDE_ROW_TYPE_COMPRESSION 1 // has tokudb row format compression patch
#if defined(HTON_SUPPORTS_EXTENDED_KEYS)
#define TOKU_INCLUDE_EXTENDED_KEYS 1
#endif
#define TOKU_OPTIMIZE_WITH_RECREATE 1
#define TOKU_INCLUDE_WRITE_FRM_DATA 1
#define TOKU_INCLUDE_DISCOVER_FRM 1
#define TOKU_INCLUDE_RFR 1
#define TOKU_INCLUDE_UPSERT 1

#if defined(TOKU_INCLUDE_DISCOVER_FRM) && TOKU_INCLUDE_DISCOVER_FRM
#include "discover.h"
#endif // defined(TOKU_INCLUDE_DISCOVER_FRM) && TOKU_INCLUDE_DISCOVER_FRM 

// MySQL does not support thdvar memalloc correctly
// see http://bugs.mysql.com/bug.php?id=71759
#define TOKU_THDVAR_MEMALLOC_BUG 1

#if !defined(HA_CLUSTERING)
#define HA_CLUSTERING 0
#endif

#if !defined(HA_CLUSTERED_INDEX)
#define HA_CLUSTERED_INDEX 0
#endif

#if !defined(HA_CAN_WRITE_DURING_OPTIMIZE)
#define HA_CAN_WRITE_DURING_OPTIMIZE 0
#endif

#if !defined(HA_OPTION_CREATE_FROM_ENGINE)
#define HA_OPTION_CREATE_FROM_ENGINE 0
#endif

//
// returns maximum length of dictionary name, such as key-NAME
// NAME_CHAR_LEN is max length of the key name, and have upper bound of 10 for key-
//
#define MAX_DICT_NAME_LEN NAME_CHAR_LEN + 10

// QQQ how to tune these?
#define HA_TOKUDB_RANGE_COUNT   100
/* extra rows for estimate_rows_upper_bound() */
#define HA_TOKUDB_EXTRA_ROWS    100

/* Bits for share->status */
#define STATUS_PRIMARY_KEY_INIT 0x1

#if defined(TOKUDB_VERSION_MAJOR) && defined(TOKUDB_VERSION_MINOR)
#define TOKUDB_PLUGIN_VERSION ((TOKUDB_VERSION_MAJOR << 8) + TOKUDB_VERSION_MINOR)
#else
#define TOKUDB_PLUGIN_VERSION 0
#endif

// Branch prediction macros.
// If supported by the compiler, will hint in instruction caching for likely
// branching. Should only be used where there is a very good idea of the correct
// branch heuristics as determined by profiling. Mostly copied from InnoDB.
// Use:
//   "if (TOKUDB_LIKELY(x))" where the chances of "x" evaluating true are higher
//   "if (TOKUDB_UNLIKELY(x))" where the chances of "x" evaluating false are higher
#if defined(__GNUC__) && (__GNUC__ > 2) && ! defined(__INTEL_COMPILER)

// Tell the compiler that 'expr' probably evaluates to 'constant'.
#define TOKUDB_EXPECT(expr,constant) __builtin_expect(expr, constant)

#else

#error "No TokuDB branch prediction operations in use!"
#define TOKUDB_EXPECT(expr,constant) (expr)

#endif // defined(__GNUC__) && (__GNUC__ > 2) && ! defined(__INTEL_COMPILER)

// Tell the compiler that cond is likely to hold
#define TOKUDB_LIKELY(cond) TOKUDB_EXPECT(cond, 1)

// Tell the compiler that cond is unlikely to hold
#define TOKUDB_UNLIKELY(cond) TOKUDB_EXPECT(cond, 0)

// Tell the compiler that the function/argument is unused
#define TOKUDB_UNUSED(_uu) _uu __attribute__((unused))
// mysql 5.6.15 removed the test macro, so we define our own
#define tokudb_test(e) ((e) ? 1 : 0)

inline const char* tokudb_thd_get_proc_info(const THD* thd) {
    return thd->proc_info;
}
inline void tokudb_thd_set_proc_info(THD* thd, const char* proc_info) {
    thd_proc_info(thd, proc_info);
}

// uint3korr reads 4 bytes and valgrind reports an error, so we use this function instead
inline uint tokudb_uint3korr(const uchar *a) {
    uchar b[4] = {};
    memcpy(b, a, 3);
    return uint3korr(b);
}

typedef unsigned int pfs_key_t;

#if defined(SAFE_MUTEX) || defined(HAVE_PSI_MUTEX_INTERFACE)
#define mutex_t_lock(M) M.lock(__FILE__, __LINE__)
#else  // SAFE_MUTEX || HAVE_PSI_MUTEX_INTERFACE
#define mutex_t_lock(M) M.lock()
#endif  // SAFE_MUTEX || HAVE_PSI_MUTEX_INTERFACE

#if defined(SAFE_MUTEX)
#define mutex_t_unlock(M) M.unlock(__FILE__, __LINE__)
#else  // SAFE_MUTEX
#define mutex_t_unlock(M) M.unlock()
#endif  // SAFE_MUTEX

#if defined(HAVE_PSI_RWLOCK_INTERFACE)
#define rwlock_t_lock_read(M) M.lock_read(__FILE__, __LINE__)
#define rwlock_t_lock_write(M) M.lock_write(__FILE__, __LINE__)
#else  // HAVE_PSI_RWLOCK_INTERFACE
#define rwlock_t_lock_read(M) M.lock_read()
#define rwlock_t_lock_write(M) M.lock_write()
#endif  // HAVE_PSI_RWLOCK_INTERFACE

#endif  // _HATOKU_DEFINES_H
