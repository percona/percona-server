/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
// vim: ft=cpp:expandtab:ts=8:sw=4:softtabstop=4:
#ident "$Id$"
/*======
This file is part of TokuDB


Copyright (c) 2006, 2015, Percona and/or its affiliates. All rights reserved.

    TokuDBis is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2,
    as published by the Free Software Foundation.

    TokuDB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with TokuDB.  If not, see <http://www.gnu.org/licenses/>.

======= */

#ident \
    "Copyright (c) 2006, 2015, Percona and/or its affiliates. All rights reserved."

#ifndef _TOKUDB_SYSVARS_H
#define _TOKUDB_SYSVARS_H

extern "C" {
extern uint force_recovery;
}

namespace tokudb {
namespace sysvars {

enum analyze_mode_t {
  TOKUDB_ANALYZE_STANDARD = 0,
  TOKUDB_ANALYZE_RECOUNT_ROWS = 1,
  TOKUDB_ANALYZE_CANCEL = 2
};

enum empty_scan_mode_t {
  TOKUDB_EMPTY_SCAN_DISABLED = 0,
  TOKUDB_EMPTY_SCAN_LR = 1,
  TOKUDB_EMPTY_SCAN_RL = 2,
};

enum row_format_t {
  SRV_ROW_FORMAT_UNCOMPRESSED = 0,
  SRV_ROW_FORMAT_ZLIB = 1,
  SRV_ROW_FORMAT_SNAPPY = 2,
  SRV_ROW_FORMAT_QUICKLZ = 3,
  SRV_ROW_FORMAT_LZMA = 4,
  SRV_ROW_FORMAT_FAST = 5,
  SRV_ROW_FORMAT_SMALL = 6,
  SRV_ROW_FORMAT_DEFAULT = 7
};

#define DEFAULT_TOKUDB_CLEANER_ITERATIONS 5
#define DEFAULT_TOKUDB_CLEANER_PERIOD 1
#define DEFAULT_TOKUDB_KILLED_TIME 4000   // milliseconds
#define DEFAULT_TOKUDB_LOCK_TIMEOUT 4000  // milliseconds

// globals
extern ulonglong cache_size;
extern uint cachetable_pool_threads;
extern int cardinality_scale_percent;
extern bool checkpoint_on_flush_logs;
extern uint checkpoint_pool_threads;
extern uint checkpointing_period;
extern ulong cleaner_iterations;
extern ulong cleaner_period;
extern uint client_pool_threads;
extern bool compress_buffers_before_eviction;
extern char *data_dir;
extern ulong debug;
extern bool directio;
extern bool enable_partial_eviction;
extern bool enabled;
extern int fs_reserve_percent;
extern uint fsync_log_period;
extern char *log_dir;
extern ulonglong max_lock_memory;
extern uint read_status_frequency;
extern bool strip_frm_data;
extern char *tmp_dir;
extern uint write_status_frequency;
extern bool dir_per_db;
extern char *version;
extern bool check_jemalloc;

#if defined(TOKUDB_DEBUG)
// used to control background job manager
extern bool debug_pause_background_job_manager;
#endif  // defined(TOKUDB_DEBUG)

// session/thread
bool alter_print_error(THD *thd);
double analyze_delete_fraction(THD *thd);
bool analyze_in_background(THD *thd);
analyze_mode_t analyze_mode(THD *thd);
ulonglong analyze_throttle(THD *thd);
ulonglong analyze_time(THD *thd);
ulonglong auto_analyze(THD *thd);
uint block_size(THD *thd);
bool bulk_fetch(THD *thd);
bool commit_sync(THD *thd);
bool create_index_online(THD *thd);
bool disable_hot_alter(THD *thd);
bool disable_prefetching(THD *thd);
bool disable_slow_alter(THD *thd);
#if defined(TOKU_INCLUDE_UPSERT) && TOKU_INCLUDE_UPSERT
bool enable_fast_update(THD *thd);
bool enable_fast_upsert(THD *thd);
#endif  // defined(TOKU_INCLUDE_UPSERT) && TOKU_INCLUDE_UPSERT
empty_scan_mode_t empty_scan(THD *thd);
uint fanout(THD *thd);
bool hide_default_row_format(THD *thd);
ulonglong killed_time(THD *thd);
bool load_save_space(THD *thd);
char *last_lock_timeout(THD *thd);
void set_last_lock_timeout(THD *thd, char *last);
ulonglong loader_memory_size(THD *thd);
ulonglong lock_timeout(THD *thd);
uint lock_timeout_debug(THD *thd);
double optimize_index_fraction(THD *thd);
const char *optimize_index_name(THD *thd);
ulonglong optimize_throttle(THD *thd);
void set_pk_insert_mode(THD *thd, uint mode);
bool prelock_empty(THD *thd);
uint read_block_size(THD *thd);
uint read_buf_size(THD *thd);
row_format_t row_format(THD *thd);
#if defined(TOKU_INCLUDE_RFR) && TOKU_INCLUDE_RFR
bool rpl_check_readonly(THD *thd);
bool rpl_lookup_rows(THD *thd);
ulonglong rpl_lookup_rows_delay(THD *thd);
bool rpl_unique_checks(THD *thd);
ulonglong rpl_unique_checks_delay(THD *thd);
#endif  // defined(TOKU_INCLUDE_RFR) && TOKU_INCLUDE_RFR

extern SYS_VAR *system_variables[];

}  // namespace sysvars
}  // namespace tokudb

#endif  // _TOKUDB_SYSVARS_H
