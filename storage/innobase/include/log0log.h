/*****************************************************************************

Copyright (c) 1995, 2022, Oracle and/or its affiliates.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/**************************************************/ /**
 @file include/log0log.h

 Redo log - the main header.

 Basic types are defined inside log0types.h.

 Constant values are defined inside log0constants.h, but that
 file should only be included by log0types.h.

 The log_sys is defined in log0sys.h.

 Functions related to the log buffer are declared in log0buf.h.

 Functions related to the checkpoints are declared in log0chkp.h.

 Functions related to the writer/flusher are declared in log0write.h.

 Functions computing capacity of redo and related margins are declared
 in log0files_capacity.h.

 Functions doing IO to log files and formatting log blocks are declared
 in log0files_io.h.

 *******************************************************/

#ifndef log0log_h
#define log0log_h

#include "log0files_capacity.h"
#include "log0files_dict.h"
#include "log0files_finder.h"
#include "log0files_governor.h"
#include "log0files_io.h"
#include "log0sys.h"
#include "log0types.h"

/**************************************************/ /**

 @name Log - LSN computations.

 *******************************************************/

/** @{ */

/** Calculates lsn value for given sn value. Sequence of sn values
enumerate all data bytes in the redo log. Sequence of lsn values
enumerate all data bytes and bytes used for headers and footers
of all log blocks in the redo log. For every LOG_BLOCK_DATA_SIZE
bytes of data we have OS_FILE_LOG_BLOCK_SIZE bytes in the redo log.
NOTE that LOG_BLOCK_DATA_SIZE + LOG_BLOCK_HDR_SIZE + LOG_BLOCK_TRL_SIZE
== OS_FILE_LOG_BLOCK_SIZE. The calculated lsn value will always point
to some data byte (will be % OS_FILE_LOG_BLOCK_SIZE >= LOG_BLOCK_HDR_SIZE,
and < OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE).

@param[in]      sn      sn value
@return lsn value for the provided sn value */
constexpr inline lsn_t log_translate_sn_to_lsn(sn_t sn) {
  return sn / LOG_BLOCK_DATA_SIZE * OS_FILE_LOG_BLOCK_SIZE +
         sn % LOG_BLOCK_DATA_SIZE + LOG_BLOCK_HDR_SIZE;
}

/** Calculates sn value for given lsn value.
@see log_translate_sn_to_lsn
@param[in]      lsn     lsn value
@return sn value for the provided lsn value */
inline sn_t log_translate_lsn_to_sn(lsn_t lsn) {
  /* Calculate sn of the beginning of log block, which contains
  the provided lsn value. */
  const sn_t sn = lsn / OS_FILE_LOG_BLOCK_SIZE * LOG_BLOCK_DATA_SIZE;

  /* Calculate offset for the provided lsn within the log block.
  The offset includes LOG_BLOCK_HDR_SIZE bytes of block's header. */
  const uint32_t diff = lsn % OS_FILE_LOG_BLOCK_SIZE;

  if (diff < LOG_BLOCK_HDR_SIZE) {
    /* The lsn points to some bytes inside the block's header.
    Return sn for the beginning of the block. Note, that sn
    values don't enumerate bytes of blocks' headers, so the
    value of diff does not matter at all. */
    return sn;
  }

  if (diff > OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE) {
    /* The lsn points to some bytes inside the block's footer.
    Return sn for the beginning of the next block. Note, that
    sn values don't enumerate bytes of blocks' footer, so the
    value of diff does not matter at all. */
    return sn + LOG_BLOCK_DATA_SIZE;
  }

  /* Add the offset but skip bytes of block's header. */
  return sn + diff - LOG_BLOCK_HDR_SIZE;
}

/** Validates a given lsn value. Checks if the lsn value points to data
bytes inside log block (not to some bytes in header/footer). It is used
by assertions.
@return true if lsn points to data bytes within log block */
inline bool log_is_data_lsn(lsn_t lsn) {
  const uint32_t offset = lsn % OS_FILE_LOG_BLOCK_SIZE;

  return lsn >= LOG_START_LSN && offset >= LOG_BLOCK_HDR_SIZE &&
         offset < OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE;
}

/** @} */

#ifndef UNIV_HOTBACKUP

/**************************************************/ /**

 @name Log - general functions.

 *******************************************************/

/** @{ */

/** @return consistent sn value for locked state */
static inline sn_t log_get_sn(const log_t &log) {
  const sn_t sn = log.sn.load();
  if ((sn & SN_LOCKED) != 0) {
    return log.sn_locked.load();
  } else {
    return sn;
  }
}

/** Gets the current lsn value. This value points to the first non
reserved data byte in the redo log. When next user thread reserves
space in the redo log, it starts at this lsn.

If the last reservation finished exactly before footer of log block,
this value points to the first byte after header of the next block.

@note It is possible that the current lsn value does not fit free
space in the log files or in the log buffer. In such case, user
threads need to wait until the space becomes available.

@return current lsn */
inline lsn_t log_get_lsn(const log_t &log) {
  return log_translate_sn_to_lsn(log_get_sn(log));
}

/** Waits until there is free space for range of sn values ending
at the provided sn, in both the log buffer and in the log files.
@param[in]      log       redo log
@param[in]      end_sn    end of the range of sn values */
void log_wait_for_space(log_t &log, sn_t end_sn);

<<<<<<< HEAD
/** Computes capacity of redo log available until log_free_check()
reaches point where it needs to wait.
@param[in]  log       redo log
@return lsn capacity up to free_check_wait happens */
lsn_t log_get_free_check_capacity(const log_t &log);

/** When the oldest dirty page age exceeds this value, we start
an asynchronous preflush of dirty pages.
@param[in]  log       redo log
@return age of dirty page at which async preflush is started */
lsn_t log_get_max_modified_age_async(const log_t &log);

/** Waits until there is free space in log files which includes
concurrency margin required for all threads. You should rather
use log_free_check().
*/
MY_COMPILER_DIAGNOSTIC_PUSH()
MY_COMPILER_CLANG_WORKAROUND_REF_DOCBUG()
/**
@see @ref sect_redo_log_reclaim_space
*/
MY_COMPILER_DIAGNOSTIC_POP()
/**
@param[in]     log   redo log */
void log_free_check_wait(log_t &log);

/** Updates limits related to free space in redo log files:
log.available_for_checkpoint_lsn and log.free_check_limit_sn.
@param[in,out]  log         redo log */
void log_update_limits(log_t &log);

/** Updates limit used when writing to log buffer. Note that the
log buffer may have space for log records for which we still do
not have space in log files (for larger lsn values).
@param[in,out]   log        redo log */
void log_update_buf_limit(log_t &log);

/** Updates limit used when writing to log buffer, according to provided
write_lsn. It must be <= log.write_lsn.load() to protect from log buffer
overwrites.
@param[in,out]   log        redo log
@param[in]       write_lsn  value <= log.write_lsn.load() */
void log_update_buf_limit(log_t &log, lsn_t write_lsn);

/** Waits until the redo log is written up to a provided lsn.
@param[in]  log             redo log
@param[in]  lsn             lsn to wait for
@param[in]  flush_to_disk   true: wait until it is flushed
@return statistics about waiting inside */
Wait_stats log_write_up_to(log_t &log, lsn_t lsn, bool flush_to_disk);

/* Read the first log file header to get the encryption
information if it exist.
@return true if success */
bool log_read_encryption();

/** Write the encryption info into the log file header(the 3rd block).
It just need to flush the file header block with current master key.
@param[in]      key     encryption key
@param[in]      iv      encryption iv
@return true if success. */
bool log_write_encryption(byte *key, byte *iv);

/** Rotate the redo log encryption
It will re-encrypt the redo log encryption metadata and write it to
redo log file header.
@return true if success. */
bool log_rotate_encryption();

/** Computes lsn up to which sync flush should be done or returns 0
if there is no need to execute sync flush now.
@param[in,out]  log  redo log
@return lsn for which we want to have oldest_lsn >= lsn in each BP,
        or 0 if there is no need for sync flush */
lsn_t log_sync_flush_lsn(log_t &log);

/** Requests a sharp checkpoint write for provided or greater lsn.
@param[in,out]  log     redo log
@param[in]      sync    true -> wait until it is finished
@param[in]  lsn   lsn for which we need checkpoint (or greater chkp) */
void log_request_checkpoint(log_t &log, bool sync, lsn_t lsn);

/** Requests a fuzzy checkpoint write (for lsn currently available
for checkpointing).
@param[in,out]  log     redo log
@param[in]      sync    true -> wait until it is finished */
void log_request_checkpoint(log_t &log, bool sync);

/** Make a checkpoint at the current lsn. Reads current lsn and waits
until all dirty pages have been flushed up to that lsn. Afterwards
requests a checkpoint write and waits until it is finished.
@param[in,out]  log     redo log
@return true iff current lsn was greater than last checkpoint lsn */
bool log_make_latest_checkpoint(log_t &log);

/** Make a checkpoint at the current lsn. Reads current lsn and waits
until all dirty pages have been flushed up to that lsn. Afterwards
requests a checkpoint write and waits until it is finished.
@return true iff current lsn was greater than last checkpoint lsn */
bool log_make_latest_checkpoint();

/** Reads a log file header page to log.checkpoint_buf.
@param[in,out]  log     redo log
@param[in]      header  0 or LOG_CHECKPOINT_1 or LOG_CHECKPOINT2 */
void log_files_header_read(log_t &log, uint32_t header);

/** Fill redo log header.
@param[out]     buf             filled buffer
@param[in]      start_lsn       log start LSN
@param[in]      creator         creator of the header
@param[in]      no_logging      redo logging is disabled
@param[in]      crash_unsafe    it is not safe to crash */
void log_files_header_fill(byte *buf, lsn_t start_lsn, const char *creator,
                           bool no_logging, bool crash_unsafe);

/** Writes a log file header to the log file space.
@param[in]      log             redo log
@param[in]      nth_file        header for the nth file in the log files
@param[in]      start_lsn       log file data starts at this lsn */
void log_files_header_flush(log_t &log, uint32_t nth_file, lsn_t start_lsn);

/** Changes format of redo files to previous format version.
@param[in]      log             redo log
@param[in]      log_format      previous format version */
void log_files_downgrade(log_t &log, uint32_t log_format);

/** Writes the next checkpoint info to header of the first log file.
Note that two pages of the header are used alternately for consecutive
checkpoints. If we crashed during the write, we would still have the
previous checkpoint info and recovery would work.
@param[in,out]  log                     redo log
@param[in]      next_checkpoint_lsn     writes checkpoint at this lsn */
void log_files_write_checkpoint(log_t &log, lsn_t next_checkpoint_lsn);

/** Updates current_file_lsn and current_file_real_offset to correspond
to a given lsn. For this function to work, the values must already be
initialized to correspond to some lsn, for instance, a checkpoint lsn.
@param[in,out]  log     redo log
@param[in]      lsn     log sequence number to set files_start_lsn at */
void log_files_update_offsets(log_t &log, lsn_t lsn);

/** Acquires the log buffer x-lock.
@param[in,out]  log     redo log */
void log_buffer_x_lock_enter(log_t &log);

/** Releases the log buffer x-lock.
@param[in,out]  log     redo log */
void log_buffer_x_lock_exit(log_t &log);
#endif /* !UNIV_HOTBACKUP */

/** Calculates offset within log files, excluding headers of log files.
@param[in]      log             redo log
@param[in]      offset          real offset (including log file headers)
@return size offset excluding log file headers (<= offset) */
uint64_t log_files_size_offset(const log_t &log, uint64_t offset);

/** Calculates offset within log files, including headers of log files.
@param[in]      log             redo log
@param[in]      offset          size offset (excluding log file headers)
@return real offset including log file headers (>= offset) */
uint64_t log_files_real_offset(const log_t &log, uint64_t offset);

/** Calculates offset within log files, including headers of log files,
for the provided lsn value.
@param[in]      log     redo log
@param[in]      lsn     log sequence number
@return real offset within the log files */
uint64_t log_files_real_offset_for_lsn(const log_t &log, lsn_t lsn);
#ifndef UNIV_HOTBACKUP

/** Changes size of the log buffer. This is a thread-safe version.
It is used by SET GLOBAL innodb_log_buffer_size = X.
@param[in,out]  log             redo log
@param[in]      new_size        requested new size
@return true iff succeeded in resize */
bool log_buffer_resize(log_t &log, size_t new_size);

/** Changes size of the log buffer. This is a non-thread-safe version
which might be invoked only when there are no concurrent possible writes
to the log buffer. It is used in log_buffer_reserve() when a requested
size to reserve is larger than size of the log buffer.
@param[in,out]  log             redo log
@param[in]      new_size        requested new size
@param[in]      end_lsn         maximum lsn written to log buffer
@return true iff succeeded in resize */
bool log_buffer_resize_low(log_t &log, size_t new_size, lsn_t end_lsn);

/** Resizes the write ahead buffer in the redo log.
@param[in,out]  log             redo log
@param[in]      new_size        new size (in bytes) */
void log_write_ahead_resize(log_t &log, size_t new_size);

/** Updates the field log.dict_max_allowed_checkpoint_lsn.
@param[in,out]  log      redo log
@param[in]      max_lsn  new value for the field */
void log_set_dict_max_allowed_checkpoint_lsn(log_t &log, lsn_t max_lsn);

/** Updates log.dict_persist_margin and recompute free check limit.
@param[in,out]  log     redo log
@param[in]      margin  new value for log.dict_persist_margin */
void log_set_dict_persist_margin(log_t &log, sn_t margin);

/** Increase concurrency_margin used inside log_free_check() calls. */
void log_increase_concurrency_margin(log_t &log);

||||||| 8d8c986e571
/** Computes capacity of redo log available until log_free_check()
reaches point where it needs to wait.
@param[in]  log       redo log
@return lsn capacity up to free_check_wait happens */
lsn_t log_get_free_check_capacity(const log_t &log);

/** When the oldest dirty page age exceeds this value, we start
an asynchronous preflush of dirty pages.
@param[in]  log       redo log
@return age of dirty page at which async preflush is started */
lsn_t log_get_max_modified_age_async(const log_t &log);

/** Waits until there is free space in log files which includes
concurrency margin required for all threads. You should rather
use log_free_check().
*/
MY_COMPILER_DIAGNOSTIC_PUSH()
MY_COMPILER_CLANG_WORKAROUND_REF_DOCBUG()
/**
@see @ref sect_redo_log_reclaim_space
*/
MY_COMPILER_DIAGNOSTIC_POP()
/**
@param[in]     log   redo log */
void log_free_check_wait(log_t &log);

/** Updates limits related to free space in redo log files:
log.available_for_checkpoint_lsn and log.free_check_limit_sn.
@param[in,out]  log         redo log */
void log_update_limits(log_t &log);

/** Updates limit used when writing to log buffer. Note that the
log buffer may have space for log records for which we still do
not have space in log files (for larger lsn values).
@param[in,out]   log        redo log */
void log_update_buf_limit(log_t &log);

/** Updates limit used when writing to log buffer, according to provided
write_lsn. It must be <= log.write_lsn.load() to protect from log buffer
overwrites.
@param[in,out]   log        redo log
@param[in]       write_lsn  value <= log.write_lsn.load() */
void log_update_buf_limit(log_t &log, lsn_t write_lsn);

/** Waits until the redo log is written up to a provided lsn.
@param[in]  log             redo log
@param[in]  lsn             lsn to wait for
@param[in]  flush_to_disk   true: wait until it is flushed
@return statistics about waiting inside */
Wait_stats log_write_up_to(log_t &log, lsn_t lsn, bool flush_to_disk);

/* Read the first log file header to get the encryption
information if it exist.
@return true if success */
bool log_read_encryption();

/** Write the encryption info into the log file header(the 3rd block).
It just need to flush the file header block with current master key.
@param[in]      key     encryption key
@param[in]      iv      encryption iv
@return true if success. */
bool log_write_encryption(byte *key, byte *iv);

/** Rotate the redo log encryption
It will re-encrypt the redo log encryption metadata and write it to
redo log file header.
@return true if success. */
bool log_rotate_encryption();

/** Computes lsn up to which sync flush should be done or returns 0
if there is no need to execute sync flush now.
@param[in,out]  log  redo log
@return lsn for which we want to have oldest_lsn >= lsn in each BP,
        or 0 if there is no need for sync flush */
lsn_t log_sync_flush_lsn(log_t &log);

/** Requests a sharp checkpoint write for provided or greater lsn.
@param[in,out]  log     redo log
@param[in]      sync    true -> wait until it is finished
@param[in]  lsn   lsn for which we need checkpoint (or greater chkp) */
void log_request_checkpoint(log_t &log, bool sync, lsn_t lsn);

/** Requests a fuzzy checkpoint write (for lsn currently available
for checkpointing).
@param[in,out]  log     redo log
@param[in]      sync    true -> wait until it is finished */
void log_request_checkpoint(log_t &log, bool sync);

/** Make a checkpoint at the current lsn. Reads current lsn and waits
until all dirty pages have been flushed up to that lsn. Afterwards
requests a checkpoint write and waits until it is finished.
@param[in,out]  log     redo log
@return true iff current lsn was greater than last checkpoint lsn */
bool log_make_latest_checkpoint(log_t &log);

/** Make a checkpoint at the current lsn. Reads current lsn and waits
until all dirty pages have been flushed up to that lsn. Afterwards
requests a checkpoint write and waits until it is finished.
@return true iff current lsn was greater than last checkpoint lsn */
bool log_make_latest_checkpoint();

/** Reads a log file header page to log.checkpoint_buf.
@param[in,out]  log     redo log
@param[in]      header  0 or LOG_CHECKPOINT_1 or LOG_CHECKPOINT2 */
void log_files_header_read(log_t &log, uint32_t header);

/** Fill redo log header.
@param[out]     buf             filled buffer
@param[in]      start_lsn       log start LSN
@param[in]      creator         creator of the header
@param[in]      no_logging      redo logging is disabled
@param[in]      crash_unsafe    it is not safe to crash */
void log_files_header_fill(byte *buf, lsn_t start_lsn, const char *creator,
                           bool no_logging, bool crash_unsafe);

/** Writes a log file header to the log file space.
@param[in]      log             redo log
@param[in]      nth_file        header for the nth file in the log files
@param[in]      start_lsn       log file data starts at this lsn */
void log_files_header_flush(log_t &log, uint32_t nth_file, lsn_t start_lsn);

/** Changes format of redo files to previous format version.

@note Note this will work between the two formats 5_7_9 & current because
the only change is the version number */
void log_files_downgrade(log_t &log);

/** Writes the next checkpoint info to header of the first log file.
Note that two pages of the header are used alternately for consecutive
checkpoints. If we crashed during the write, we would still have the
previous checkpoint info and recovery would work.
@param[in,out]  log                     redo log
@param[in]      next_checkpoint_lsn     writes checkpoint at this lsn */
void log_files_write_checkpoint(log_t &log, lsn_t next_checkpoint_lsn);

/** Updates current_file_lsn and current_file_real_offset to correspond
to a given lsn. For this function to work, the values must already be
initialized to correspond to some lsn, for instance, a checkpoint lsn.
@param[in,out]  log     redo log
@param[in]      lsn     log sequence number to set files_start_lsn at */
void log_files_update_offsets(log_t &log, lsn_t lsn);

/** Acquires the log buffer x-lock.
@param[in,out]  log     redo log */
void log_buffer_x_lock_enter(log_t &log);

/** Releases the log buffer x-lock.
@param[in,out]  log     redo log */
void log_buffer_x_lock_exit(log_t &log);
#endif /* !UNIV_HOTBACKUP */

/** Calculates offset within log files, excluding headers of log files.
@param[in]      log             redo log
@param[in]      offset          real offset (including log file headers)
@return size offset excluding log file headers (<= offset) */
uint64_t log_files_size_offset(const log_t &log, uint64_t offset);

/** Calculates offset within log files, including headers of log files.
@param[in]      log             redo log
@param[in]      offset          size offset (excluding log file headers)
@return real offset including log file headers (>= offset) */
uint64_t log_files_real_offset(const log_t &log, uint64_t offset);

/** Calculates offset within log files, including headers of log files,
for the provided lsn value.
@param[in]      log     redo log
@param[in]      lsn     log sequence number
@return real offset within the log files */
uint64_t log_files_real_offset_for_lsn(const log_t &log, lsn_t lsn);
#ifndef UNIV_HOTBACKUP

/** Changes size of the log buffer. This is a thread-safe version.
It is used by SET GLOBAL innodb_log_buffer_size = X.
@param[in,out]  log             redo log
@param[in]      new_size        requested new size
@return true iff succeeded in resize */
bool log_buffer_resize(log_t &log, size_t new_size);

/** Changes size of the log buffer. This is a non-thread-safe version
which might be invoked only when there are no concurrent possible writes
to the log buffer. It is used in log_buffer_reserve() when a requested
size to reserve is larger than size of the log buffer.
@param[in,out]  log             redo log
@param[in]      new_size        requested new size
@param[in]      end_lsn         maximum lsn written to log buffer
@return true iff succeeded in resize */
bool log_buffer_resize_low(log_t &log, size_t new_size, lsn_t end_lsn);

/** Resizes the write ahead buffer in the redo log.
@param[in,out]  log             redo log
@param[in]      new_size        new size (in bytes) */
void log_write_ahead_resize(log_t &log, size_t new_size);

/** Updates the field log.dict_max_allowed_checkpoint_lsn.
@param[in,out]  log      redo log
@param[in]      max_lsn  new value for the field */
void log_set_dict_max_allowed_checkpoint_lsn(log_t &log, lsn_t max_lsn);

/** Updates log.dict_persist_margin and recompute free check limit.
@param[in,out]  log     redo log
@param[in]      margin  new value for log.dict_persist_margin */
void log_set_dict_persist_margin(log_t &log, sn_t margin);

/** Increase concurrency_margin used inside log_free_check() calls. */
void log_increase_concurrency_margin(log_t &log);

=======
>>>>>>> mysql-8.0.30
/** Prints information about important lsn values used in the redo log,
and some statistics about speed of writing and flushing of data.
@param[in]      log     redo log for which print information
@param[out]     file    file where to print */
void log_print(const log_t &log, FILE *file);

/** Refreshes the statistics used to print per-second averages in log_print().
@param[in,out]  log     redo log */
void log_refresh_stats(log_t &log);

void log_update_exported_variables(const log_t &log);

/** @} */

/**************************************************/ /**

 @name Log - initialization of the redo log system.

 *******************************************************/

/** @{ */

/** Initializes log_sys and finds existing redo log files, or creates a new
set of redo log files.

New redo log files are created in following cases:
  - there are no existing redo log files in the log directory,
  - existing set of redo log files is not marked as fully initialized
    (flag LOG_HEADER_FLAG_NOT_INITIALIZED exists in the newest file).

After this call, the log_sys global variable is allocated and initialized.
InnoDB might start recovery then.

@remarks
The redo log files are not resized in this function, because before resizing
log files, InnoDB must run recovery and ensure log files are logically empty.
The redo resize is currently the only scenario in which the initialized log_sys
might become closed by log_sys_close() and then re-initialized by another call
to log_sys_init().

@note Note that the redo log system is NOT ready for user writes after this
call is finished. The proper order of calls looks like this:
        - log_sys_init(),
        - log_start(),
        - log_start_background_threads()
and this sequence is executed inside srv_start() in srv0start.cc (interleaved
with remaining logic of the srv_start())

@param[in]    expect_no_files   true means we should return DB_ERROR if log
                                files are present in the directory before
                                proceeding any further
@param[in]    flushed_lsn       lsn at which new redo log files might be
                                started if they had to be created during
                                this call; this should be lsn stored in
                                the system tablespace header at offset
                                FIL_PAGE_FILE_FLUSH_LSN if the data
                                directory has been initialized;
@param[out]   new_files_lsn     updated to the lsn of the first checkpoint
                                created in the new log files if new log files
                                are created; else: 0
@return DB_SUCCESS or error */
dberr_t log_sys_init(bool expect_no_files, lsn_t flushed_lsn,
                     lsn_t &new_files_lsn);

/** Starts the initialized redo log system using a provided
checkpoint_lsn and current lsn. Block for current_lsn must
be properly initialized in the log buffer prior to calling
this function. Therefore a proper value of first_rec_group
must be set for that block before log_start is called.
@param[in,out]  log                redo log
@param[in]      checkpoint_lsn     checkpoint lsn
@param[in]      start_lsn          current lsn to start at
@param[in]      first_block        data block (with start_lsn)
                                   to copy into the log buffer;
                                   nullptr if no reason to copy
@param[in]      allow_checkpoints  true iff allows writing newer checkpoints
@return DB_SUCCESS or error */
dberr_t log_start(log_t &log, lsn_t checkpoint_lsn, lsn_t start_lsn,
                  byte first_block[OS_FILE_LOG_BLOCK_SIZE],
                  bool allow_checkpoints = true);

/** Close the log system and free all the related memory. */
void log_sys_close();

/** Resizes the write ahead buffer in the redo log.
@param[in,out]  log       redo log
@param[in]      new_size  new size (in bytes) */
void log_write_ahead_resize(log_t &log, size_t new_size);

/** @} */

/**************************************************/ /**

 @name Log - the log threads and mutexes

 *******************************************************/

/** @{ */

/** Validates that all the log background threads are active.
Used only to assert, that the state is correct.
@param[in]      log     redo log */
void log_background_threads_active_validate(const log_t &log);

/** Validates that all the log background threads are inactive.
Used only to assert, that the state is correct. */
void log_background_threads_inactive_validate();

/** Starts all the log background threads. This can be called only,
when the threads are inactive. This should never be called concurrently.
This may not be called during read-only mode.
@param[in,out]  log     redo log */
void log_start_background_threads(log_t &log);

/** Stops all the log background threads. This can be called only,
when the threads are active. This should never be called concurrently.
This may not be called in read-only mode. Note that is is impossible
to start log background threads in such case.
@param[in,out]  log     redo log */
void log_stop_background_threads(log_t &log);

/** Marks the flag which tells log threads to stop and wakes them.
Does not wait until they are stopped.
@param[in,out]  log     redo log */
void log_stop_background_threads_nowait(log_t &log);

/** Function similar to @see log_stop_background_threads() except that it
stops all the log threads in such a way, that the redo log will be logically
empty after the threads are stopped.
@note It is caller responsibility to ensure that all threads other than the
log_files_governor cannot produce new redo log records when this function
is being called. */
void log_make_empty_and_stop_background_threads(log_t &log);

/** Wakes up all log threads which are alive.
@param[in,out]  log     redo log */
void log_wake_threads(log_t &log);

#define log_limits_mutex_enter(log) mutex_enter(&((log).limits_mutex))

#define log_limits_mutex_exit(log) mutex_exit(&((log).limits_mutex))

#define log_limits_mutex_own(log) mutex_own(&(log).limits_mutex)

/** @} */

/**************************************************/ /**

 @name Log - the log position locking.

 *******************************************************/

/** @{ */

/** Lock redo log. Both current lsn and checkpoint lsn will not change
until the redo log is unlocked.
@param[in,out]  log     redo log to lock */
void log_position_lock(log_t &log);

/** Unlock the locked redo log.
@param[in,out]  log     redo log to unlock */
void log_position_unlock(log_t &log);

/** Collect coordinates in the locked redo log.
@param[in]      log             locked redo log
@param[out]     current_lsn     stores current lsn there
@param[out]     checkpoint_lsn  stores checkpoint lsn there */
void log_position_collect_lsn_info(const log_t &log, lsn_t *current_lsn,
                                   lsn_t *checkpoint_lsn);

/** @} */

/**************************************************/ /**

 @name Log - persisting the flags.

 *******************************************************/

/** @{ */

/** Disable redo logging and persist the information.
@param[in,out]  log     redo log */
void log_persist_disable(log_t &log);

/** Enable redo logging and persist the information.
@param[in,out]  log     redo log */
void log_persist_enable(log_t &log);

/** Persist the information that it is safe to restart server.
@param[in,out]  log     redo log */
void log_persist_crash_safe(log_t &log);

/** Marks the redo log files as belonging to the initialized data directory
with initialized set of redo log files. Flushes the log_flags without the
flag LOG_HEADER_FLAG_NOT_INITIALIZED to the newest redo log file.
@param[in,out]  log   redo log */
void log_persist_initialized(log_t &log);

/** Asserts that the log is not marked as crash-unsafe.
@param[in,out]  log   redo log */
void log_crash_safe_validate(log_t &log);

/** @} */

#endif /* !UNIV_HOTBACKUP */

#endif /* !log0log_h */
