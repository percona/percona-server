/*****************************************************************************

Copyright (c) 1995, 2017, Oracle and/or its affiliates. All Rights Reserved.
Copyright (c) 2009, Google Inc.
Copyright (c) 2016, Percona Inc. All Rights Reserved.

Portions of this file contain modifications contributed and copyrighted by
Google, Inc. Those modifications are gratefully acknowledged and are described
briefly in the InnoDB documentation. The contributions by Google are
incorporated with their permission, and subject to the conditions contained in
the file COPYING.Google.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA

*****************************************************************************/

/**************************************************//**
@file log/log0log.cc
Database log

Created 12/9/1995 Heikki Tuuri
*******************************************************/

#include "ha_prototypes.h"
#include <debug_sync.h>

#include "log0log.h"

#ifdef UNIV_NONINL
#include "log0log.ic"
#endif

#include "mem0mem.h"
#include "buf0buf.h"
#ifndef UNIV_HOTBACKUP
#include "buf0flu.h"
#include "srv0srv.h"
#include "log0recv.h"
#include "lock0lock.h"
#include "fil0fil.h"
#include "fil0crypt.h"
#include "dict0boot.h"
#include "dict0stats_bg.h"
#include "srv0srv.h"
#include "srv0start.h"
#include "trx0sys.h"
#include "trx0trx.h"
#include "trx0roll.h"
#include "srv0mon.h"
#include "sync0sync.h"
#endif /* !UNIV_HOTBACKUP */

#include "system_key.h"

static redo_log_encrypt_enum found_log_encryption_mode = REDO_LOG_ENCRYPT_OFF;

/*
General philosophy of InnoDB redo-logs:

1) Every change to a contents of a data page must be done
through mtr, which in mtr_commit() writes log records
to the InnoDB redo log.

2) Normally these changes are performed using a mlog_write_ulint()
or similar function.

3) In some page level operations only a code number of a
c-function and its parameters are written to the log to
reduce the size of the log.

  3a) You should not add parameters to these kind of functions
  (e.g. trx_undo_header_create(), trx_undo_insert_header_reuse())

  3b) You should not add such functionality which either change
  working when compared with the old or are dependent on data
  outside of the page. These kind of functions should implement
  self-contained page transformation and it should be unchanged
  if you don't have very essential reasons to change log
  semantics or format.

*/

/** Redo log system */
log_t*	log_sys	= NULL;

/** Whether to generate and require checksums on the redo log pages */
my_bool	innodb_log_checksums;

/** Pointer to the log checksum calculation function */
log_checksum_func_t log_checksum_algorithm_ptr;

/* Next log block number to do dummy record filling if no log records written
for a while */
static ulint		next_lbn_to_pad = 0;

/* These control how often we print warnings if the last checkpoint is too
old */
bool	log_has_printed_chkp_warning = false;
time_t	log_last_warning_time;

bool	log_has_printed_chkp_margine_warning = false;
time_t	log_last_margine_warning_time;

/* A margin for free space in the log buffer before a log entry is catenated */
#define LOG_BUF_WRITE_MARGIN	(4 * OS_FILE_LOG_BLOCK_SIZE)

/* Margins for free space in the log buffer after a log entry is catenated */
#define LOG_BUF_FLUSH_RATIO	2
#define LOG_BUF_FLUSH_MARGIN	(LOG_BUF_WRITE_MARGIN + 4 * UNIV_PAGE_SIZE)

/* This parameter controls asynchronous making of a new checkpoint; the value
should be bigger than LOG_POOL_PREFLUSH_RATIO_SYNC */

#define LOG_POOL_CHECKPOINT_RATIO_ASYNC	32

/* This parameter controls synchronous preflushing of modified buffer pages */
#define LOG_POOL_PREFLUSH_RATIO_SYNC	16

/* The same ratio for asynchronous preflushing; this value should be less than
the previous */
#define LOG_POOL_PREFLUSH_RATIO_ASYNC	8

/* Codes used in unlocking flush latches */
#define LOG_UNLOCK_NONE_FLUSHED_LOCK	1
#define LOG_UNLOCK_FLUSH_LOCK		2

/** Event to wake up log_scrub_thread */
os_event_t      log_scrub_event;
/** Whether log_scrub_thread is active */
bool            log_scrub_thread_active;

extern "C"
os_thread_ret_t
DECLARE_THREAD(log_scrub_thread)(void*);


/******************************************************//**
Completes a checkpoint write i/o to a log file. */
static
void
log_io_complete_checkpoint(void);
/*============================*/

#ifndef UNIV_HOTBACKUP
/****************************************************************//**
Returns the oldest modified block lsn in the pool, or log_sys->lsn if none
exists.
@return LSN of oldest modification */
static
lsn_t
log_buf_pool_get_oldest_modification(void)
/*======================================*/
{
	lsn_t	lsn;

	ut_ad(log_mutex_own());

	lsn = buf_pool_get_oldest_modification();

	if (!lsn) {

		lsn = log_sys->lsn;
	}

	return(lsn);
}
#endif  /* !UNIV_HOTBACKUP */

/****************************************************************//**
Checks if the log groups have a big enough margin of free space in
so that a new log entry can be written without overwriting log data
that is not read by the changed page bitmap thread.
@return true if there is not enough free space. */
static
bool
log_check_tracking_margin(
	ulint	lsn_advance)	/*!< in: an upper limit on how much log data we
				plan to write.  If zero, the margin will be
				checked for the already-written log. */
{
	lsn_t	tracked_lsn;
	lsn_t	tracked_lsn_age;

	if (!srv_track_changed_pages) {
		return false;
	}

	ut_ad(mutex_own(&(log_sys->mutex)));

	tracked_lsn = log_get_tracked_lsn();
	tracked_lsn_age = log_sys->lsn - tracked_lsn;

	/* The overwrite would happen when log_sys->log_group_capacity is
	exceeded, but we use max_checkpoint_age for an extra safety margin. */
	return tracked_lsn_age + lsn_advance > log_sys->max_checkpoint_age;
}

/** Extends the log buffer.
@param[in]	len	requested minimum size in bytes */
void
log_buffer_extend(
	ulint	len)
{
	ulint	move_start;
	ulint	move_end;
	byte*	tmp_buf[OS_FILE_LOG_BLOCK_SIZE];

	log_mutex_enter_all();

	while (log_sys->is_extending) {
		/* Another thread is trying to extend already.
		Needs to wait for. */
		log_mutex_exit_all();

		log_buffer_flush_to_disk();

		log_mutex_enter_all();

		if (srv_log_buffer_size > len / UNIV_PAGE_SIZE) {
			/* Already extended enough by the others */
			log_mutex_exit_all();
			return;
		}
	}

	if (len >= log_sys->buf_size / 2) {
		DBUG_EXECUTE_IF("ib_log_buffer_is_short_crash",
				DBUG_SUICIDE(););

		/* log_buffer is too small. try to extend instead of crash. */
		ib::warn() << "The transaction log size is too large"
			" for innodb_log_buffer_size (" << len << " >= "
			<< LOG_BUFFER_SIZE << " / 2). Trying to extend it.";
	}

	log_sys->is_extending = true;

	while (ut_calc_align_down(log_sys->buf_free,
				  OS_FILE_LOG_BLOCK_SIZE)
	       != ut_calc_align_down(log_sys->buf_next_to_write,
				     OS_FILE_LOG_BLOCK_SIZE)) {
		/* Buffer might have >1 blocks to write still. */
		log_mutex_exit_all();

		log_buffer_flush_to_disk();

		log_mutex_enter_all();
	}

	move_start = ut_calc_align_down(
		log_sys->buf_free,
		OS_FILE_LOG_BLOCK_SIZE);
	move_end = log_sys->buf_free;

	/* store the last log block in buffer */
	ut_memcpy(tmp_buf, log_sys->buf + move_start,
		  move_end - move_start);

	log_sys->buf_free -= move_start;
	log_sys->buf_next_to_write -= move_start;

	/* reallocate log buffer */
	srv_log_buffer_size = len / UNIV_PAGE_SIZE + 1;
	ut_free(log_sys->buf_ptr);

	log_sys->buf_size = LOG_BUFFER_SIZE;

	log_sys->buf_ptr = static_cast<byte*>(
		ut_zalloc_nokey(log_sys->buf_size * 2
				+ MAX_SRV_LOG_WRITE_AHEAD_SIZE));
	log_sys->buf = static_cast<byte*>(
		ut_align(log_sys->buf_ptr, MAX_SRV_LOG_WRITE_AHEAD_SIZE));

	log_sys->first_in_use = true;

	log_sys->max_buf_free = log_sys->buf_size / LOG_BUF_FLUSH_RATIO
		- LOG_BUF_FLUSH_MARGIN;

	/* restore the last log block */
	ut_memcpy(log_sys->buf, tmp_buf, move_end - move_start);

	ut_ad(log_sys->is_extending);
	log_sys->is_extending = false;

	log_mutex_exit_all();

	ib::info() << "innodb_log_buffer_size was extended to "
		<< LOG_BUFFER_SIZE << ".";
}

#ifndef UNIV_HOTBACKUP
/** Calculate actual length in redo buffer and file including
block header and trailer.
@param[in]	len	length to write
@return actual length to write including header and trailer. */
static inline
ulint
log_calculate_actual_len(
	ulint len)
{
	ut_ad(log_mutex_own());

	/* actual length stored per block */
	const ulint	len_per_blk = OS_FILE_LOG_BLOCK_SIZE
		- (LOG_BLOCK_HDR_SIZE + LOG_BLOCK_TRL_SIZE);

	/* actual data length in last block already written */
	ulint	extra_len = (log_sys->buf_free % OS_FILE_LOG_BLOCK_SIZE);

	ut_ad(extra_len >= LOG_BLOCK_HDR_SIZE);
	extra_len -= LOG_BLOCK_HDR_SIZE;

	/* total extra length for block header and trailer */
	extra_len = ((len + extra_len) / len_per_blk)
		* (LOG_BLOCK_HDR_SIZE + LOG_BLOCK_TRL_SIZE);

	return(len + extra_len);
}

/** Check margin not to overwrite transaction log from the last checkpoint.
If would estimate the log write to exceed the log_group_capacity,
waits for the checkpoint is done enough.
@param[in]	len	length of the data to be written */

void
log_margin_checkpoint_age(
	ulint	len)
{
	ulint	margin = log_calculate_actual_len(len);

	ut_ad(log_mutex_own());

	if (margin > log_sys->log_group_capacity) {
		/* return with warning output to avoid deadlock */
		if (!log_has_printed_chkp_margine_warning
		    || difftime(time(NULL),
				log_last_margine_warning_time) > 15) {
			log_has_printed_chkp_margine_warning = true;
			log_last_margine_warning_time = time(NULL);

			ib::error() << "The transaction log files are too"
				" small for the single transaction log (size="
				<< len << "). So, the last checkpoint age"
				" might exceed the log group capacity "
				<< log_sys->log_group_capacity << ".";
		}

		return;
	}

	/* Our margin check should ensure that we never reach this condition.
	Try to do checkpoint once. We cannot keep waiting here as it might
	result in hang in case the current mtr has latch on oldest lsn */
	if (log_sys->lsn - log_sys->last_checkpoint_lsn + margin
	    > log_sys->log_group_capacity) {
		/* The log write of 'len' might overwrite the transaction log
		after the last checkpoint. Makes checkpoint. */

		bool	flushed_enough = false;

		if (log_sys->lsn - log_buf_pool_get_oldest_modification()
		    + margin
		    <= log_sys->log_group_capacity) {
			flushed_enough = true;
		}

		log_sys->check_flush_or_checkpoint = true;
		log_mutex_exit();

		DEBUG_SYNC_C("margin_checkpoint_age_rescue");

		if (!flushed_enough) {
			os_thread_sleep(100000);
		}
		log_checkpoint(true, false);

		log_mutex_enter();
	}

	return;
}
#endif /* !UNIV_HOTBACKUP */
/** Open the log for log_write_low. The log must be closed with log_close.
@param[in]	len	length of the data to be written
@return start lsn of the log record */
lsn_t
log_reserve_and_open(
	ulint	len)
{
	ulint	len_upper_limit;
	ulint	count			= 0;
	ulint	tcount			= 0;

loop:
	ut_ad(log_mutex_own());
	ut_ad(!recv_no_log_write);

	if (log_sys->is_extending) {
		log_mutex_exit();

		/* Log buffer size is extending. Writing up to the next block
		should wait for the extending finished. */

		os_thread_sleep(100000);

		ut_ad(++count < 50);

		log_mutex_enter();
		goto loop;
	}

	/* Calculate an upper limit for the space the string may take in the
	log buffer */

	len_upper_limit = LOG_BUF_WRITE_MARGIN + srv_log_write_ahead_size
			  + (5 * len) / 4;

	if (log_sys->buf_free + len_upper_limit > log_sys->buf_size) {
		log_mutex_exit();

		DEBUG_SYNC_C("log_buf_size_exceeded");

		/* Not enough free space, do a write of the log buffer */

		log_buffer_sync_in_background(false);

		srv_stats.log_waits.inc();

		ut_ad(++count < 50);

		log_mutex_enter();
		goto loop;
	}

	if (log_check_tracking_margin(len_upper_limit) &&
		(++tcount + count < 50)) {

		/* This log write would violate the untracked LSN free space
		margin.  Limit this to 50 retries as there might be situations
		where we have no choice but to proceed anyway, i.e. if the log
		is about to be overflown, log tracking or not. */
		log_mutex_exit();

		os_thread_sleep(10000);

		log_mutex_enter();
		goto loop;
	}

	return(log_sys->lsn);
}

/************************************************************//**
Writes to the log the string given. It is assumed that the caller holds the
log mutex. */
void
log_write_low(
/*==========*/
	const byte*	str,		/*!< in: string */
	ulint		str_len)	/*!< in: string length */
{
	log_t*	log	= log_sys;
	ulint	len;
	ulint	data_len;
	byte*	log_block;

	ut_ad(log_mutex_own());
part_loop:
	ut_ad(!recv_no_log_write);
	/* Calculate a part length */

	data_len = (log->buf_free % OS_FILE_LOG_BLOCK_SIZE) + str_len;

	if (data_len <= OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE) {

		/* The string fits within the current log block */

		len = str_len;
	} else {
		data_len = OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE;

		len = OS_FILE_LOG_BLOCK_SIZE
			- (log->buf_free % OS_FILE_LOG_BLOCK_SIZE)
			- LOG_BLOCK_TRL_SIZE;
	}

	ut_memcpy(log->buf + log->buf_free, str, len);

	str_len -= len;
	str = str + len;

	log_block = static_cast<byte*>(
		ut_align_down(
			log->buf + log->buf_free, OS_FILE_LOG_BLOCK_SIZE));

	log_block_set_data_len(log_block, data_len);

	if (data_len == OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE) {
		/* This block became full */
		log_block_set_data_len(log_block, OS_FILE_LOG_BLOCK_SIZE);
		log_block_set_checkpoint_no(log_block,
					    log_sys->next_checkpoint_no);
		len += LOG_BLOCK_HDR_SIZE + LOG_BLOCK_TRL_SIZE;

		log->lsn += len;

		/* Initialize the next block header */
		log_block_init(log_block + OS_FILE_LOG_BLOCK_SIZE, log->lsn);
	} else {
		log->lsn += len;
	}

	log->buf_free += len;

	ut_ad(log->buf_free <= log->buf_size);

	if (str_len > 0) {
		goto part_loop;
	}

	srv_stats.log_write_requests.inc();
}

/************************************************************//**
Closes the log.
@return lsn */
lsn_t
log_close(void)
/*===========*/
{
	byte*		log_block;
	ulint		first_rec_group;
	lsn_t		oldest_lsn;
	lsn_t		lsn;
	log_t*		log	= log_sys;
	lsn_t		checkpoint_age;

	ut_ad(log_mutex_own());
	ut_ad(!recv_no_log_write);

	lsn = log->lsn;

	log_block = static_cast<byte*>(
		ut_align_down(
			log->buf + log->buf_free, OS_FILE_LOG_BLOCK_SIZE));

	first_rec_group = log_block_get_first_rec_group(log_block);

	if (first_rec_group == 0) {
		/* We initialized a new log block which was not written
		full by the current mtr: the next mtr log record group
		will start within this block at the offset data_len */

		log_block_set_first_rec_group(
			log_block, log_block_get_data_len(log_block));
	}

	if (log->buf_free > log->max_buf_free) {

		log->check_flush_or_checkpoint = true;
	}

	if (srv_track_changed_pages) {

		lsn_t tracked_lsn = log_get_tracked_lsn();
		ut_ad(tracked_lsn > 0);
		lsn_t tracked_lsn_age = lsn - tracked_lsn;

		if (tracked_lsn_age >= log->log_group_capacity) {

			ib::error() << "The age of the oldest untracked "
				"record exceeds the log group capacity!";
			ib::error() << "Stopping the log tracking thread at "
				"LSN " << tracked_lsn;
			srv_track_changed_pages = FALSE;
		}
	}

	checkpoint_age = lsn - log->last_checkpoint_lsn;

	if (checkpoint_age >= log->log_group_capacity) {
		DBUG_EXECUTE_IF(
			"print_all_chkp_warnings",
			log_has_printed_chkp_warning = false;);

		if (!log_has_printed_chkp_warning
		    || difftime(time(NULL), log_last_warning_time) > 15) {

			log_has_printed_chkp_warning = true;
			log_last_warning_time = time(NULL);

			ib::error() << "The age of the last checkpoint is "
				<< checkpoint_age << ", which exceeds the log"
				" group capacity " << log->log_group_capacity
				<< ".";
		}
	}

	if (checkpoint_age <= log->max_modified_age_sync) {

		goto function_exit;
	}

	oldest_lsn = buf_pool_get_oldest_modification();

	if (!oldest_lsn
	    || lsn - oldest_lsn > log->max_modified_age_sync
	    || checkpoint_age > log->max_checkpoint_age_async) {

		log->check_flush_or_checkpoint = true;
	}
function_exit:

	return(lsn);
}

/******************************************************//**
Calculates the data capacity of a log group, when the log file headers are not
included.
@return capacity in bytes */
lsn_t
log_group_get_capacity(
/*===================*/
	const log_group_t*	group)	/*!< in: log group */
{
	/* The lsn parameters are updated while holding both the mutexes
	and it is ok to have either of them while reading */
	ut_ad(log_mutex_own() || log_write_mutex_own());

	return((group->file_size - LOG_FILE_HDR_SIZE) * group->n_files);
}

/******************************************************//**
Calculates the offset within a log group, when the log file headers are not
included.
@return size offset (<= offset) */
UNIV_INLINE
lsn_t
log_group_calc_size_offset(
/*=======================*/
	lsn_t			offset,	/*!< in: real offset within the
					log group */
	const log_group_t*	group)	/*!< in: log group */
{
	/* The lsn parameters are updated while holding both the mutexes
	and it is ok to have either of them while reading */
	ut_ad(log_mutex_own() || log_write_mutex_own());

	return(offset - LOG_FILE_HDR_SIZE * (1 + offset / group->file_size));
}

/******************************************************//**
Calculates the offset within a log group, when the log file headers are
included.
@return real offset (>= offset) */
UNIV_INLINE
lsn_t
log_group_calc_real_offset(
/*=======================*/
	lsn_t			offset,	/*!< in: size offset within the
					log group */
	const log_group_t*	group)	/*!< in: log group */
{
	/* The lsn parameters are updated while holding both the mutexes
	and it is ok to have either of them while reading */
	ut_ad(log_mutex_own() || log_write_mutex_own());

	return(offset + LOG_FILE_HDR_SIZE
	       * (1 + offset / (group->file_size - LOG_FILE_HDR_SIZE)));
}

/** Calculate the offset of an lsn within a log group.
@param[in]	lsn	log sequence number
@param[in]	group	log group
@return offset within the log group */
lsn_t
log_group_calc_lsn_offset(
	lsn_t			lsn,
	const log_group_t*	group)
{
	lsn_t	gr_lsn;
	lsn_t	gr_lsn_size_offset;
	lsn_t	difference;
	lsn_t	group_size;
	lsn_t	offset;

	/* The lsn parameters are updated while holding both the mutexes
	and it is ok to have either of them while reading */
	ut_ad(log_mutex_own() || log_write_mutex_own());

	gr_lsn = group->lsn;

	gr_lsn_size_offset = log_group_calc_size_offset(
		group->lsn_offset, group);

	group_size = log_group_get_capacity(group);

	if (lsn >= gr_lsn) {

		difference = lsn - gr_lsn;
	} else {
		difference = gr_lsn - lsn;

		difference = difference % group_size;

		difference = group_size - difference;
	}

	offset = (gr_lsn_size_offset + difference) % group_size;

	/* fprintf(stderr,
	"Offset is " LSN_PF " gr_lsn_offset is " LSN_PF
	" difference is " LSN_PF "\n",
	offset, gr_lsn_size_offset, difference);
	*/

	return(log_group_calc_real_offset(offset, group));
}

/*******************************************************************//**
Calculates where in log files we find a specified lsn.
@return log file number */
ulint
log_calc_where_lsn_is(
/*==================*/
	int64_t*	log_file_offset,	/*!< out: offset in that file
						(including the header) */
	ib_uint64_t	first_header_lsn,	/*!< in: first log file start
						lsn */
	ib_uint64_t	lsn,			/*!< in: lsn whose position to
						determine */
	ulint		n_log_files,		/*!< in: total number of log
						files */
	int64_t		log_file_size)		/*!< in: log file size
						(including the header) */
{
	int64_t		capacity	= log_file_size - LOG_FILE_HDR_SIZE;
	ulint		file_no;
	int64_t		add_this_many;

	if (lsn < first_header_lsn) {
		add_this_many = 1 + (first_header_lsn - lsn)
			/ (capacity * static_cast<int64_t>(n_log_files));
		lsn += add_this_many
			* capacity * static_cast<int64_t>(n_log_files);
	}

	ut_a(lsn >= first_header_lsn);

	file_no = ((ulint)((lsn - first_header_lsn) / capacity))
		% n_log_files;
	*log_file_offset = (lsn - first_header_lsn) % capacity;

	*log_file_offset = *log_file_offset + LOG_FILE_HDR_SIZE;

	return(file_no);
}


/********************************************************//**
Sets the field values in group to correspond to a given lsn. For this function
to work, the values must already be correctly initialized to correspond to
some lsn, for instance, a checkpoint lsn. */
void
log_group_set_fields(
/*=================*/
	log_group_t*	group,	/*!< in/out: group */
	lsn_t		lsn)	/*!< in: lsn for which the values should be
				set */
{
	group->lsn_offset = log_group_calc_lsn_offset(lsn, group);
	group->lsn = lsn;
}
#ifndef UNIV_HOTBACKUP
/*****************************************************************//**
Calculates the recommended highest values for lsn - last_checkpoint_lsn
and lsn - buf_get_oldest_modification().
@retval true on success
@retval false if the smallest log group is too small to
accommodate the number of OS threads in the database server */
static MY_ATTRIBUTE((warn_unused_result))
bool
log_calc_max_ages(void)
/*===================*/
{
	log_group_t*	group;
	lsn_t		margin;
	ulint		free;
	bool		success	= true;
	lsn_t		smallest_capacity;

	log_mutex_enter();

	group = UT_LIST_GET_FIRST(log_sys->log_groups);

	ut_ad(group);

	smallest_capacity = LSN_MAX;

	while (group) {
		if (log_group_get_capacity(group) < smallest_capacity) {

			smallest_capacity = log_group_get_capacity(group);
		}

		group = UT_LIST_GET_NEXT(log_groups, group);
	}

	/* Add extra safety */
	smallest_capacity = smallest_capacity - smallest_capacity / 10;

	/* For each OS thread we must reserve so much free space in the
	smallest log group that it can accommodate the log entries produced
	by single query steps: running out of free log space is a serious
	system error which requires rebooting the database. */

	free = LOG_CHECKPOINT_FREE_PER_THREAD * (10 + srv_thread_concurrency)
		+ LOG_CHECKPOINT_EXTRA_FREE;
	if (free >= smallest_capacity / 2) {
		success = false;

		goto failure;
	} else {
		margin = smallest_capacity - free;
	}

	margin = margin - margin / 10;	/* Add still some extra safety */

	log_sys->log_group_capacity = smallest_capacity;

	log_sys->max_modified_age_async = margin
		- margin / LOG_POOL_PREFLUSH_RATIO_ASYNC;
	log_sys->max_modified_age_sync = margin
		- margin / LOG_POOL_PREFLUSH_RATIO_SYNC;

	log_sys->max_checkpoint_age_async = margin - margin
		/ LOG_POOL_CHECKPOINT_RATIO_ASYNC;
	log_sys->max_checkpoint_age = margin;

failure:
	log_mutex_exit();

	if (!success) {
		ib::error() << "Cannot continue operation. ib_logfiles are too"
			" small for innodb_thread_concurrency "
			<< srv_thread_concurrency << ". The combined size of"
			" ib_logfiles should be bigger than"
			" 200 kB * innodb_thread_concurrency. To get mysqld"
			" to start up, set innodb_thread_concurrency in"
			" my.cnf to a lower value, for example, to 8. After"
			" an ERROR-FREE shutdown of mysqld you can adjust"
			" the size of ib_logfiles. " << INNODB_PARAMETERS_MSG;
	}

	return(success);
}

/******************************************************//**
Initializes the log. */
void
log_init(void)
/*==========*/
{
	log_sys = static_cast<log_t*>(ut_zalloc_nokey(sizeof(log_t)));

	mutex_create(LATCH_ID_LOG_SYS, &log_sys->mutex);
	mutex_create(LATCH_ID_LOG_WRITE, &log_sys->write_mutex);

	mutex_create(LATCH_ID_LOG_FLUSH_ORDER, &log_sys->log_flush_order_mutex);

	/* Start the lsn from one log block from zero: this way every
	log record has a start lsn != zero, a fact which we will use */

	log_sys->lsn = LOG_START_LSN;

	ut_a(LOG_BUFFER_SIZE >= 16 * OS_FILE_LOG_BLOCK_SIZE);
	ut_a(LOG_BUFFER_SIZE >= 4 * UNIV_PAGE_SIZE);

	log_sys->buf_size = LOG_BUFFER_SIZE;

	log_sys->buf_ptr = static_cast<byte*>(
		ut_zalloc_nokey(log_sys->buf_size * 2
				+ MAX_SRV_LOG_WRITE_AHEAD_SIZE));
	log_sys->buf = static_cast<byte*>(
		ut_align(log_sys->buf_ptr, MAX_SRV_LOG_WRITE_AHEAD_SIZE));

	log_sys->first_in_use = true;

	log_sys->max_buf_free = log_sys->buf_size / LOG_BUF_FLUSH_RATIO
		- LOG_BUF_FLUSH_MARGIN;
	log_sys->check_flush_or_checkpoint = true;
	UT_LIST_INIT(log_sys->log_groups, &log_group_t::log_groups);

	log_sys->n_log_ios_old = log_sys->n_log_ios;
	log_sys->last_printout_time = time(NULL);
	/*----------------------------*/

	log_sys->write_lsn = log_sys->lsn;

	log_sys->flush_event = os_event_create(0);

	os_event_set(log_sys->flush_event);

	/*----------------------------*/

	log_sys->last_checkpoint_lsn = log_sys->lsn;
	log_sys->next_checkpoint_lsn = log_sys->lsn;

	rw_lock_create(
		checkpoint_lock_key, &log_sys->checkpoint_lock,
		SYNC_NO_ORDER_CHECK);

	log_sys->checkpoint_buf_ptr = static_cast<byte*>(
		ut_zalloc_nokey(OS_FILE_LOG_BLOCK_SIZE
				+ MAX_SRV_LOG_WRITE_AHEAD_SIZE));

	log_sys->checkpoint_buf = static_cast<byte*>(
		ut_align(log_sys->checkpoint_buf_ptr,
			 MAX_SRV_LOG_WRITE_AHEAD_SIZE));

	/*----------------------------*/

	log_block_init(log_sys->buf, log_sys->lsn);
	log_block_set_first_rec_group(log_sys->buf, LOG_BLOCK_HDR_SIZE);

	log_sys->buf_free = LOG_BLOCK_HDR_SIZE;
	log_sys->lsn = LOG_START_LSN + LOG_BLOCK_HDR_SIZE;

	MONITOR_SET(MONITOR_LSN_CHECKPOINT_AGE,
		    log_sys->lsn - log_sys->last_checkpoint_lsn);

	log_scrub_thread_active= !srv_read_only_mode && srv_scrub_log;
	if (log_scrub_thread_active) {
		log_scrub_event= os_event_create("log_scrub_event");
		os_thread_create(log_scrub_thread, NULL, NULL);
	}
}

/******************************************************************//**
Inits a log group to the log system.
@return true if success, false if not */
MY_ATTRIBUTE((warn_unused_result))
bool
log_group_init(
/*===========*/
	ulint	id,			/*!< in: group id */
	ulint	n_files,		/*!< in: number of log files */
	lsn_t	file_size,		/*!< in: log file size in bytes */
	ulint	space_id)		/*!< in: space id of the file space
					which contains the log files of this
					group */
{
	ulint	i;
	log_group_t*	group;

	group = static_cast<log_group_t*>(ut_malloc_nokey(sizeof(log_group_t)));

	group->id = id;
	group->n_files = n_files;
	group->format = LOG_HEADER_FORMAT_CURRENT;
	group->file_size = file_size;
	group->space_id = space_id;
	group->state = LOG_GROUP_OK;
	group->lsn = LOG_START_LSN;
	group->lsn_offset = LOG_FILE_HDR_SIZE;

	group->file_header_bufs_ptr = static_cast<byte**>(
		ut_zalloc_nokey(sizeof(byte*) * n_files));

	group->file_header_bufs = static_cast<byte**>(
		ut_zalloc_nokey(sizeof(byte**) * n_files));

	for (i = 0; i < n_files; i++) {
		group->file_header_bufs_ptr[i] = static_cast<byte*>(
			ut_zalloc_nokey(LOG_FILE_HDR_SIZE
					+ MAX_SRV_LOG_WRITE_AHEAD_SIZE));

		group->file_header_bufs[i] = static_cast<byte*>(
			ut_align(group->file_header_bufs_ptr[i],
				 MAX_SRV_LOG_WRITE_AHEAD_SIZE));
	}

	group->checkpoint_buf_ptr = static_cast<byte*>(
		ut_zalloc_nokey(OS_FILE_LOG_BLOCK_SIZE +
				MAX_SRV_LOG_WRITE_AHEAD_SIZE));

	group->checkpoint_buf = static_cast<byte*>(
		ut_align(group->checkpoint_buf_ptr,
			 MAX_SRV_LOG_WRITE_AHEAD_SIZE));

	UT_LIST_ADD_LAST(log_sys->log_groups, group);

	return(log_calc_max_ages());
}
#endif /* !UNIV_HOTBACKUP */
/******************************************************//**
Completes an i/o to a log file. */
void
log_io_complete(
/*============*/
	log_group_t*	group)	/*!< in: log group or a dummy pointer */
{
	if ((ulint) group & 0x1UL) {
		/* It was a checkpoint write */
		group = (log_group_t*)((ulint) group - 1);

#ifdef _WIN32
		fil_flush(group->space_id);
#else
		switch (srv_unix_file_flush_method) {
		case SRV_UNIX_O_DSYNC:
		case SRV_UNIX_NOSYNC:
		case SRV_UNIX_ALL_O_DIRECT:
			break;
		case SRV_UNIX_FSYNC:
		case SRV_UNIX_LITTLESYNC:
		case SRV_UNIX_O_DIRECT:
		case SRV_UNIX_O_DIRECT_NO_FSYNC:
			if (thd_flush_log_at_trx_commit(NULL) != 2)
				fil_flush(group->space_id);
		}
#endif /* _WIN32 */

		DBUG_PRINT("ib_log", ("checkpoint info written to group %u",
				      unsigned(group->id)));
		log_io_complete_checkpoint();

		return;
	}

	ut_error;	/*!< We currently use synchronous writing of the
			logs and cannot end up here! */
}

/******************************************************//**
Writes a log file header to a log file space. */
static
void
log_group_file_header_flush(
/*========================*/
	log_group_t*	group,		/*!< in: log group */
	ulint		nth_file,	/*!< in: header to the nth file in the
					log file space */
	lsn_t		start_lsn)	/*!< in: log file data starts at this
					lsn */
{
	byte*	buf;
	lsn_t	dest_offset;

	ut_ad(log_write_mutex_own());
	ut_ad(!recv_no_log_write);
	ut_ad(group->id == 0);
	ut_a(nth_file < group->n_files);

	buf = *(group->file_header_bufs + nth_file);

	memset(buf, 0, OS_FILE_LOG_BLOCK_SIZE);
	mach_write_to_4(buf + LOG_HEADER_FORMAT, LOG_HEADER_FORMAT_CURRENT);
	mach_write_to_8(buf + LOG_HEADER_START_LSN, start_lsn);
	strcpy(reinterpret_cast<char*>(buf) + LOG_HEADER_CREATOR,
	       LOG_HEADER_CREATOR_CURRENT);
	ut_ad(LOG_HEADER_CREATOR_END - LOG_HEADER_CREATOR
	      >= sizeof LOG_HEADER_CREATOR_CURRENT);
	log_block_set_checksum(buf, log_block_calc_checksum_crc32(buf));

	dest_offset = nth_file * group->file_size;

	DBUG_PRINT("ib_log", ("write " LSN_PF
			      " group " ULINTPF
			      " file " ULINTPF " header",
			      start_lsn, group->id, nth_file));

	log_sys->n_log_ios++;

	MONITOR_INC(MONITOR_LOG_IO);

	srv_stats.os_log_pending_writes.inc();

	const ulint	page_no
		= (ulint) (dest_offset / univ_page_size.physical());

	fil_io(IORequestLogWrite, true,
	       page_id_t(group->space_id, page_no),
	       univ_page_size,
	       (ulint) (dest_offset % univ_page_size.physical()),
	       OS_FILE_LOG_BLOCK_SIZE, buf, group);

	srv_stats.os_log_pending_writes.dec();
}

/* Read the first log file header to get the encryption. It's in the
3rd block.
@return true if success */
bool
log_read_encryption()
{
	byte		key[ENCRYPTION_KEY_LEN];
	byte		iv[ENCRYPTION_KEY_LEN];

	byte* log_block_buf_ptr = static_cast<byte*>(ut_malloc_nokey(
		2 * OS_FILE_LOG_BLOCK_SIZE));
	memset(log_block_buf_ptr, 0, 2 * OS_FILE_LOG_BLOCK_SIZE);
	byte* log_block_buf = static_cast<byte*>(
		ut_align(log_block_buf_ptr, OS_FILE_LOG_BLOCK_SIZE));

	ulint		log_space_id = SRV_LOG_SPACE_FIRST_ID;
	const page_id_t	page_id(log_space_id, 0);
	fil_io(IORequestLogRead, true, page_id, univ_page_size,
	       LOG_CHECKPOINT_1 + OS_FILE_LOG_BLOCK_SIZE,
	       OS_FILE_LOG_BLOCK_SIZE, log_block_buf, NULL);

	bool encryption_magic = false;
	bool encrypted_log = false;

	if (memcmp(log_block_buf + LOG_HEADER_CREATOR_END,
		   ENCRYPTION_KEY_MAGIC_RK, ENCRYPTION_MAGIC_SIZE) == 0) {
		encryption_magic = true;
		found_log_encryption_mode = REDO_LOG_ENCRYPT_RK;

		/* Make sure the keyring is loaded. */
		if (!Encryption::check_keyring()) {
			ut_free(log_block_buf_ptr);
			ib::error()
				<< "Redo log was encrypted,"
				<< " but keyring plugin is not loaded.";
			return(false);
		}

		unsigned char* info_ptr = log_block_buf + LOG_HEADER_CREATOR_END + ENCRYPTION_MAGIC_SIZE;
	        uint version = mach_read_from_4(info_ptr);

		memcpy(iv, info_ptr + ENCRYPTION_SERVER_UUID_LEN + 4, ENCRYPTION_KEY_LEN);

#ifdef UNIV_ENCRYPT_DEBUG
			fprintf(stderr, "Using redo log encryption key version: %u\n", version);
#endif
		
		char *key_type = NULL;
		char* rkey = NULL;
		std::ostringstream percona_redo_with_ver_ss;
		percona_redo_with_ver_ss << PERCONA_REDO_KEY_NAME << ':' << version;
		size_t klen;
		if (my_key_fetch(percona_redo_with_ver_ss.str().c_str(), &key_type, NULL,
					reinterpret_cast<void**>(&rkey), &klen) ||
				rkey == NULL)
		{
			ib::error() << "Couldn't fetch redo log encryption key: " << percona_redo_with_ver_ss.str() << ".";
		} else if(key_type == NULL || strncmp(key_type, "AES", 3) != 0) {
			ib::error() << "Unknown redo log encryption type: " << key_type << ".";
		} else {
			encrypted_log = true;
			memcpy(key, rkey, ENCRYPTION_KEY_LEN);
		}
		my_free(key_type);
		my_free(rkey);
	}

	if (memcmp(log_block_buf + LOG_HEADER_CREATOR_END,
		   ENCRYPTION_KEY_MAGIC_V2, ENCRYPTION_MAGIC_SIZE) == 0) {
		encryption_magic = true;
		found_log_encryption_mode = REDO_LOG_ENCRYPT_MK;

		/* Make sure the keyring is loaded. */
		if (!Encryption::check_keyring()) {
			ib::error()
				<< "Redo log was encrypted,"
				<< " but keyring plugin is not loaded.";
		} else if (Encryption::decode_encryption_info(
				key, iv,
				log_block_buf + LOG_HEADER_CREATOR_END)) {
			encrypted_log = true;
		}

	}
	if (encrypted_log) {
		/* If redo log encryption is enabled, set the
		   space flag. Otherwise, we just fill the encryption
		   information to space object for decrypting old
		   redo log blocks. */
		fil_space_t*	space = fil_space_get(log_space_id);
		space->flags |= FSP_FLAGS_MASK_ENCRYPTION;
		dberr_t err = fil_set_encryption(space->id,
				Encryption::AES,
				key, iv);

		if (err == DB_SUCCESS) {
			ut_free(log_block_buf_ptr);
			ib::info() << "Read redo log encryption"
				<< " metadata successful.";
			return(true);
		} else {
			ut_free(log_block_buf_ptr);
			ib::error()
				<< "Can't set redo log tablespace"
				<< " encryption metadata.";
			return(false);
		}
	} else if (encryption_magic) {
		ut_free(log_block_buf_ptr);
		ib::error() << "Cannot read the encryption"
			" information in log file header, please"
			" check if keyring plugin loaded and"
			" the key file exists.";
		return(false);
	}

	ut_free(log_block_buf_ptr);
	return(true);
}


/** Writes encryption information to log header.
@param[in,out]	buf	log file header
@param[in]	key	encryption key
@param[in]	iv	encryption iv */
static
bool
log_file_header_fill_encryption(
	byte*		buf,
	byte*		key,
	byte*		iv)
{
	byte		encryption_info[ENCRYPTION_INFO_SIZE_V2];

	if (!fsp_header_fill_encryption_info(key,
				      iv, encryption_info)) {
		return(false);
	}

	ut_ad(LOG_HEADER_CREATOR_END + ENCRYPTION_INFO_SIZE_V2
	      < OS_FILE_LOG_BLOCK_SIZE);

	memcpy(buf + LOG_HEADER_CREATOR_END,
	       encryption_info,
	       ENCRYPTION_INFO_SIZE_V2);

	return(true);
}

static
bool
log_file_header_fill_encryption(
	byte*		buf,
	ulint           key_version,
	byte*		iv)
{
	byte		encryption_info[ENCRYPTION_INFO_SIZE_V2] = {};

	if (!fsp_header_fill_encryption_info(key_version, iv,
				      encryption_info)) {
		return(false);
	}

	ut_ad(LOG_HEADER_CREATOR_END + ENCRYPTION_INFO_SIZE_V2
	      < OS_FILE_LOG_BLOCK_SIZE);

	memcpy(buf + LOG_HEADER_CREATOR_END,
	       encryption_info,
	       ENCRYPTION_INFO_SIZE_V2);

	return(true);
}

/** Write the encryption info into the log file header(the 3rd block).
It just need to flush the file header block with current master key.
@param[in]	key	encryption key
@param[in]	iv	encryption iv
@return true if success. */
bool
log_write_encryption(
	byte*	key,
	byte*	iv)
{
	const page_id_t	page_id(SRV_LOG_SPACE_FIRST_ID, 0);
	byte*		log_block_buf_ptr;
	byte*		log_block_buf;
	ulint version = 1;

	log_block_buf_ptr = static_cast<byte*>(ut_malloc_nokey(
		2 * OS_FILE_LOG_BLOCK_SIZE));
	memset(log_block_buf_ptr, 0, 2 * OS_FILE_LOG_BLOCK_SIZE);
	log_block_buf = static_cast<byte*>(
		ut_align(log_block_buf_ptr, OS_FILE_LOG_BLOCK_SIZE));

	if (key == NULL && iv == NULL) {
		fil_space_t*	space = fil_space_get(
			SRV_LOG_SPACE_FIRST_ID);

		key = space->encryption_key;
		iv = space->encryption_iv;
		version = space->encryption_key_version;
	}

	log_write_mutex_enter();
	if (srv_redo_log_encrypt == REDO_LOG_ENCRYPT_MK || 
	    found_log_encryption_mode == REDO_LOG_ENCRYPT_MK) {
		if (!log_file_header_fill_encryption(log_block_buf,
						     key,
						     iv)) {
			ut_free(log_block_buf_ptr);
			log_write_mutex_exit();
			return(false);
		}
	} else if (srv_redo_log_encrypt == REDO_LOG_ENCRYPT_RK ||
		   found_log_encryption_mode == REDO_LOG_ENCRYPT_RK) {
		if (!log_file_header_fill_encryption(log_block_buf,
						     version,
						     iv)) {
			ut_free(log_block_buf_ptr);
			log_write_mutex_exit();
			return(false);
		}
	} else {
		ut_ad(0);
	}

	log_sys->n_log_ios++;

	MONITOR_INC(MONITOR_LOG_IO);

	srv_stats.os_log_pending_writes.inc();

	fil_io(IORequestLogWrite, true,
	       page_id,
	       univ_page_size,
	       LOG_CHECKPOINT_1 + OS_FILE_LOG_BLOCK_SIZE,
	       OS_FILE_LOG_BLOCK_SIZE, log_block_buf, NULL);

	srv_stats.os_log_pending_writes.dec();
	log_write_mutex_exit();

	ut_free(log_block_buf_ptr);
	return(true);
}

/** Rotate the redo log encryption
It will re-encrypt the redo log encryption metadata and write it to
redo log file header.
@return true if success. */
bool
log_rotate_encryption() {
	fil_space_t* space = fil_space_get(SRV_LOG_SPACE_FIRST_ID);
 	if (!FSP_FLAGS_GET_ENCRYPTION(space->flags)) {
		return(true);
	}
 	/* Rotate log tablespace */
	return(log_write_encryption(NULL, NULL));
}

/** Check the redo log encryption is enabled or not.
It will try to enable the redo log encryption and write the metadata to
redo log file header. */
void
log_enable_encryption_if_set()
{
	fil_space_t* space = fil_space_get(SRV_LOG_SPACE_FIRST_ID);

	if (srv_shutdown_state != SRV_SHUTDOWN_NONE) {
		return;
	}

	/* Check encryption for redo log is enabled or not. If it's
	enabled, we will start to encrypt the redo log block from now on.
	Note: We need the server_uuid initialized, otherwise, the keyname will
	not contains server uuid. */
	if (srv_redo_log_encrypt != REDO_LOG_ENCRYPT_OFF
	    && !FSP_FLAGS_GET_ENCRYPTION(space->flags)
	    && strlen(server_uuid) > 0) {
		byte	key[ENCRYPTION_KEY_LEN];
		byte	iv[ENCRYPTION_KEY_LEN];

		if (srv_read_only_mode) {
			srv_redo_log_encrypt = REDO_LOG_ENCRYPT_OFF;
			ib::error() << "Can't set redo log tablespace to be"
				<< " encrypted in read-only mode.";
			return;
		}

		bool encryption_enabled = false;
		Encryption::random_value(iv);

		uint version = 1;

		if (srv_redo_log_encrypt == REDO_LOG_ENCRYPT_MK) {
			Encryption::random_value(key);
			encryption_enabled = true;
			if (!log_write_encryption(key, iv)) {
				srv_redo_log_encrypt = REDO_LOG_ENCRYPT_OFF;
				ib::error() << "Can't set redo log"
					<< " tablespace to be"
					<< " encrypted.";
				encryption_enabled = false;
			}
		} else if (srv_redo_log_encrypt == REDO_LOG_ENCRYPT_RK) {
			// load latest key & write version

			char *redo_key_type = NULL;
			byte *rkey = NULL;
			size_t   klen = 0;
			
			encryption_enabled = true;
			
			if (my_key_fetch(PERCONA_REDO_KEY_NAME, &redo_key_type, NULL,
						reinterpret_cast<void**>(&rkey), &klen) || rkey == NULL)
			{
				if (my_key_generate(PERCONA_REDO_KEY_NAME, "AES", NULL, ENCRYPTION_KEY_LEN)) {
					ib::error() << "Redo log key generation failed.";
					encryption_enabled = false;
				} else if (my_key_fetch(PERCONA_REDO_KEY_NAME, &redo_key_type, NULL,
							reinterpret_cast<void**>(&rkey), &klen)) {
					ib::error() << "Couldn't fetch newly generated redo key.";
					encryption_enabled = false;
				} else {
					DBUG_ASSERT(rkey != NULL);
					byte *rkey2 = NULL;
					size_t   klen2 = 0;
					bool err = (parse_system_key(rkey, klen, &version, &rkey2, &klen2) 
						   == reinterpret_cast<uchar*>(NullS));
					ut_ad(klen2 ==	ENCRYPTION_KEY_LEN);
					if (err) {
						encryption_enabled = false;
					} else {
						memcpy(key, rkey2, ENCRYPTION_KEY_LEN);
					}
					if (rkey2 != NULL) {
						my_free(rkey2);
					}
				}
			} else {
				memcpy(key, rkey, ENCRYPTION_KEY_LEN);
			}

			if (encryption_enabled) {
				ut_ad(redo_key_type && strcmp(redo_key_type, "AES") == 0);
			}
			if (redo_key_type != NULL) {
				my_free(redo_key_type);
			}
			if (rkey != NULL) {
				my_free(rkey);
			}

#ifdef UNIV_ENCRYPT_DEBUG
			fprintf(stderr, "Fetched redo key: %s.\n", key);
#endif

			if (encryption_enabled && !log_write_encryption(key, iv)) {
				srv_redo_log_encrypt = REDO_LOG_ENCRYPT_OFF;
				ib::error() << "Can't set redo log"
					<< " tablespace to be"
					<< " encrypted.";
				encryption_enabled = false;
			}
		}

		if (encryption_enabled) {
			space->flags |=
				FSP_FLAGS_MASK_ENCRYPTION;
			dberr_t err = fil_set_encryption(
				space->id, Encryption::AES,
				key, iv);
			space->encryption_key_version = version;
			if (err != DB_SUCCESS) {
				srv_redo_log_encrypt = REDO_LOG_ENCRYPT_OFF;
				ib::warn() << "Can't set redo log"
					<< " tablespace to be"
					<< " encrypted.";
			} else {
				ib::info() << "Redo log encryption is"
					<< " enabled.";
			}
		}
	}

	/* If the redo log space is using default key, rotate it.
	We also need the server_uuid initialized. */
	if (space->encryption_type != Encryption::NONE
	    && Encryption::master_key_id == ENCRYPTION_DEFAULT_MASTER_KEY_ID
	    && !srv_read_only_mode
	    && strlen(server_uuid) > 0
	    && srv_redo_log_encrypt == REDO_LOG_ENCRYPT_MK) {
			ut_ad(FSP_FLAGS_GET_ENCRYPTION(space->flags));

			log_write_encryption(NULL, NULL);
	}

	if (space->encryption_type != Encryption::NONE
	    && space->encryption_key_version ==REDO_LOG_ENCRYPT_NO_VERSION 
	    && !srv_read_only_mode
	    && strlen(server_uuid) > 0
	    && srv_redo_log_encrypt == REDO_LOG_ENCRYPT_RK) {
			/* This only happens when the server uuid was just generated, so we can
			 * save the key to the keyring */
			if (my_key_store(PERCONA_REDO_KEY_NAME, "AES", NULL,
						space->encryption_key, ENCRYPTION_KEY_LEN))
			{
				srv_redo_log_encrypt = REDO_LOG_ENCRYPT_OFF;
				ib::error() << "Can't store redo log encryption key.";
			}
			uint version = 0;
			size_t   klen = 0;
			size_t   klen2 = 0;
			char *redo_key_type = NULL;
			byte *rkey = NULL;
			unsigned char *rkey2 = NULL;
			if (my_key_fetch(PERCONA_REDO_KEY_NAME, &redo_key_type, NULL,
						reinterpret_cast<void**>(&rkey), &klen)) {
				srv_redo_log_encrypt = REDO_LOG_ENCRYPT_OFF;
				ib::error() << "Can't fetch latest redo log encryption key.";
			}
			const bool err = (parse_system_key(rkey, klen, &version, &rkey2, &klen2) == reinterpret_cast<uchar*>(NullS));
			if (err) {
				srv_redo_log_encrypt = REDO_LOG_ENCRYPT_OFF;
				ib::error() << "Can't parse latest redo log encryption key.";
			}
			space->encryption_key_version = version;
			if (!log_write_encryption(NULL, NULL)) {
				ib::error() << "Can't write redo log encryption information.";
			}
			if (rkey != NULL) {
				my_free(rkey);
			}
			if (rkey2 != NULL) {
				my_free(rkey2);
			}
			if (redo_key_type != NULL) {
				my_free(redo_key_type);
			}

		}
}

/******************************************************//**
Stores a 4-byte checksum to the trailer checksum field of a log block
before writing it to a log file. This checksum is used in recovery to
check the consistency of a log block. */
static
void
log_block_store_checksum(
/*=====================*/
	byte*	block)	/*!< in/out: pointer to a log block */
{
	log_block_set_checksum(block, log_block_calc_checksum(block));
}

/******************************************************//**
Writes a buffer to a log file group. */
static
void
log_group_write_buf(
/*================*/
	log_group_t*	group,		/*!< in: log group */
	byte*		buf,		/*!< in: buffer */
	ulint		len,		/*!< in: buffer len; must be divisible
					by OS_FILE_LOG_BLOCK_SIZE */
#ifdef UNIV_DEBUG
	ulint		pad_len,	/*!< in: pad len in the buffer len */
#endif /* UNIV_DEBUG */
	lsn_t		start_lsn,	/*!< in: start lsn of the buffer; must
					be divisible by
					OS_FILE_LOG_BLOCK_SIZE */
	ulint		new_data_offset)/*!< in: start offset of new data in
					buf: this parameter is used to decide
					if we have to write a new log file
					header */
{
	ulint		write_len;
	bool		write_header	= new_data_offset == 0;
	lsn_t		next_offset;
	ulint		i;

	ut_ad(log_write_mutex_own());
	ut_ad(!recv_no_log_write);
	ut_a(len % OS_FILE_LOG_BLOCK_SIZE == 0);
	ut_a(start_lsn % OS_FILE_LOG_BLOCK_SIZE == 0);

loop:
	if (len == 0) {

		return;
	}

	next_offset = log_group_calc_lsn_offset(start_lsn, group);

	if (write_header
	    && next_offset % group->file_size == LOG_FILE_HDR_SIZE) {
		/* We start to write a new log file instance in the group */

		ut_a(next_offset / group->file_size <= ULINT_MAX);

		log_group_file_header_flush(group, (ulint)
					    (next_offset / group->file_size),
					    start_lsn);
		srv_stats.os_log_written.add(OS_FILE_LOG_BLOCK_SIZE);

		srv_stats.log_writes.inc();
	}

	if ((next_offset % group->file_size) + len > group->file_size) {

		/* if the above condition holds, then the below expression
		is < len which is ulint, so the typecast is ok */
		write_len = (ulint)
			(group->file_size - (next_offset % group->file_size));
	} else {
		write_len = len;
	}

	DBUG_PRINT("ib_log",
		   ("write " LSN_PF " to " LSN_PF
		    ": group " ULINTPF " len " ULINTPF
		    " blocks " ULINTPF ".." ULINTPF,
		    start_lsn, next_offset,
		    group->id, write_len,
		    log_block_get_hdr_no(buf),
		    log_block_get_hdr_no(
			    buf + write_len
			    - OS_FILE_LOG_BLOCK_SIZE)));

	ut_ad(pad_len >= len
	      || log_block_get_hdr_no(buf)
		 == log_block_convert_lsn_to_no(start_lsn));

	/* Calculate the checksums for each log block and write them to
	the trailer fields of the log blocks */

	for (i = 0; i < write_len / OS_FILE_LOG_BLOCK_SIZE; i++) {
		ut_ad(pad_len >= len
		      || i * OS_FILE_LOG_BLOCK_SIZE >= len - pad_len
		      || log_block_get_hdr_no(
			      buf + i * OS_FILE_LOG_BLOCK_SIZE)
			 == log_block_get_hdr_no(buf) + i);
		log_block_store_checksum(buf + i * OS_FILE_LOG_BLOCK_SIZE);
	}

	log_sys->n_log_ios++;

	MONITOR_INC(MONITOR_LOG_IO);

	srv_stats.os_log_pending_writes.inc();

	ut_a(next_offset / UNIV_PAGE_SIZE <= ULINT_MAX);

	const ulint	page_no
		= (ulint) (next_offset / univ_page_size.physical());

	fil_io(IORequestLogWrite, true,
	       page_id_t(group->space_id, page_no),
	       univ_page_size,
	       (ulint) (next_offset % UNIV_PAGE_SIZE), write_len, buf,
	       group);

	srv_stats.os_log_pending_writes.dec();

	srv_stats.os_log_written.add(write_len);
	srv_stats.log_writes.inc();

	if (write_len < len) {
		start_lsn += write_len;
		len -= write_len;
		buf += write_len;

		write_header = true;

		goto loop;
	}
}

/** Flush the log has been written to the log file. */
static
void
log_write_flush_to_disk_low()
{
	ut_a(log_sys->n_pending_flushes == 1); /* No other threads here */

#ifndef _WIN32
	bool	do_flush = srv_unix_file_flush_method != SRV_UNIX_O_DSYNC;
#else
	bool	do_flush = true;
#endif
	if (do_flush) {
		log_group_t*	group = UT_LIST_GET_FIRST(log_sys->log_groups);
		fil_flush(group->space_id);
		log_sys->flushed_to_disk_lsn = log_sys->current_flush_lsn;
	}

	log_sys->n_pending_flushes--;
	MONITOR_DEC(MONITOR_PENDING_LOG_FLUSH);

	os_event_set(log_sys->flush_event);
}

/** Switch the log buffer in use, and copy the content of last block
from old log buffer to the head of the to be used one. Thus, buf_free and
buf_next_to_write would be changed accordingly */
static inline
void
log_buffer_switch()
{
	ut_ad(log_mutex_own());
	ut_ad(log_write_mutex_own());

	const byte*	old_buf = log_sys->buf;
	ulint		area_end = ut_calc_align(log_sys->buf_free,
						 OS_FILE_LOG_BLOCK_SIZE);

	if (log_sys->first_in_use) {
		ut_ad((reinterpret_cast<uintptr_t>(log_sys->buf)
		       % srv_log_write_ahead_size) == 0);
		log_sys->buf += log_sys->buf_size;
	} else {
		log_sys->buf -= log_sys->buf_size;
		ut_ad((reinterpret_cast<uintptr_t>(log_sys->buf)
		       % srv_log_write_ahead_size) == 0);
	}

	log_sys->first_in_use = !log_sys->first_in_use;

	/* Copy the last block to new buf */
	ut_memcpy(log_sys->buf,
		  old_buf + area_end - OS_FILE_LOG_BLOCK_SIZE,
		  OS_FILE_LOG_BLOCK_SIZE);

	log_sys->buf_free %= OS_FILE_LOG_BLOCK_SIZE;
	log_sys->buf_next_to_write = log_sys->buf_free;
}

/** Ensure that the log has been written to the log file up to a given
log entry (such as that of a transaction commit). Start a new write, or
wait and check if an already running write is covering the request.
@param[in]	lsn		log sequence number that should be
included in the redo log file write
@param[in]	flush_to_disk	whether the written log should also
be flushed to the file system */
void
log_write_up_to(
	lsn_t	lsn,
	bool	flush_to_disk)
{
#ifdef UNIV_DEBUG
	ulint		loop_count	= 0;
#endif /* UNIV_DEBUG */
	byte*           write_buf;
	lsn_t           write_lsn;

	ut_ad(!srv_read_only_mode);

	if (recv_no_ibuf_operations) {
		/* Recovery is running and no operations on the log files are
		allowed yet (the variable name .._no_ibuf_.. is misleading) */

		return;
	}

loop:
	ut_ad(++loop_count < 128);

#if UNIV_WORD_SIZE > 7
	/* We can do a dirty read of LSN. */
	/* NOTE: Currently doesn't do dirty read for
	(flush_to_disk == true) case, because the log_mutex
	contention also works as the arbitrator for write-IO
	(fsync) bandwidth between log files and data files. */
	os_rmb;
	if (!flush_to_disk && log_sys->write_lsn >= lsn) {
		return;
	}
#endif

	log_write_mutex_enter();
	ut_ad(!recv_no_log_write);

	lsn_t	limit_lsn = flush_to_disk
		? log_sys->flushed_to_disk_lsn
		: log_sys->write_lsn;

	if (limit_lsn >= lsn) {
		log_write_mutex_exit();
		return;
	}

#ifdef _WIN32
# ifndef UNIV_HOTBACKUP
	/* write requests during fil_flush() might not be good for Windows */
	if (log_sys->n_pending_flushes > 0
	    || !os_event_is_set(log_sys->flush_event)) {
		log_write_mutex_exit();
		os_event_wait(log_sys->flush_event);
		goto loop;
	}
# else
	if (log_sys->n_pending_flushes > 0) {
		goto loop;
	}
# endif  /* !UNIV_HOTBACKUP */
#endif /* _WIN32 */

	/* If it is a write call we should just go ahead and do it
	as we checked that write_lsn is not where we'd like it to
	be. If we have to flush as well then we check if there is a
	pending flush and based on that we wait for it to finish
	before proceeding further. */
	if (flush_to_disk
	    && (log_sys->n_pending_flushes > 0
		|| !os_event_is_set(log_sys->flush_event))) {

		/* Figure out if the current flush will do the job
		for us. */
		bool work_done = log_sys->current_flush_lsn >= lsn;

		log_write_mutex_exit();

		os_event_wait(log_sys->flush_event);

		if (work_done) {
			return;
		} else {
			goto loop;
		}
	}

	log_mutex_enter();
	if (!flush_to_disk
	    && log_sys->buf_free == log_sys->buf_next_to_write) {
		/* Nothing to write and no flush to disk requested */
		log_mutex_exit_all();
		return;
	}

	log_group_t*	group;
	ulint		start_offset;
	ulint		end_offset;
	ulint		area_start;
	ulint		area_end;
	ulong		write_ahead_size = srv_log_write_ahead_size;
	ulint		pad_size;

	DBUG_PRINT("ib_log", ("write " LSN_PF " to " LSN_PF,
			      log_sys->write_lsn,
			      log_sys->lsn));

	if (flush_to_disk) {
		log_sys->n_pending_flushes++;
		log_sys->current_flush_lsn = log_sys->lsn;
		MONITOR_INC(MONITOR_PENDING_LOG_FLUSH);
		os_event_reset(log_sys->flush_event);

		if (log_sys->buf_free == log_sys->buf_next_to_write) {
			/* Nothing to write, flush only */
			log_mutex_exit_all();
			log_write_flush_to_disk_low();
			return;
		}
	}

	start_offset = log_sys->buf_next_to_write;
	end_offset = log_sys->buf_free;

	area_start = ut_calc_align_down(start_offset, OS_FILE_LOG_BLOCK_SIZE);
	area_end = ut_calc_align(end_offset, OS_FILE_LOG_BLOCK_SIZE);

	ut_ad(area_end - area_start > 0);

	log_block_set_flush_bit(log_sys->buf + area_start, TRUE);
	log_block_set_checkpoint_no(
		log_sys->buf + area_end - OS_FILE_LOG_BLOCK_SIZE,
		log_sys->next_checkpoint_no);

	write_lsn = log_sys->lsn;
	write_buf = log_sys->buf;

	log_buffer_switch();

	group = UT_LIST_GET_FIRST(log_sys->log_groups);

	log_group_set_fields(group, log_sys->write_lsn);

	log_mutex_exit();

	/* Calculate pad_size if needed. */
	pad_size = 0;
	if (write_ahead_size > OS_FILE_LOG_BLOCK_SIZE) {
		lsn_t	end_offset;
		ulint	end_offset_in_unit;

		end_offset = log_group_calc_lsn_offset(
			ut_uint64_align_up(write_lsn,
					   OS_FILE_LOG_BLOCK_SIZE),
			group);
		end_offset_in_unit = (ulint) (end_offset % write_ahead_size);

		if (end_offset_in_unit > 0
		    && (area_end - area_start) > end_offset_in_unit) {
			/* The first block in the unit was initialized
			after the last writing.
			Needs to be written padded data once. */
			pad_size = write_ahead_size - end_offset_in_unit;

			if (area_end + pad_size > log_sys->buf_size) {
				pad_size = log_sys->buf_size - area_end;
			}

			::memset(write_buf + area_end, 0, pad_size);
		}
	}

	/* Do the write to the log files */
	log_group_write_buf(
		group, write_buf + area_start,
		area_end - area_start + pad_size,
#ifdef UNIV_DEBUG
		pad_size,
#endif /* UNIV_DEBUG */
		ut_uint64_align_down(log_sys->write_lsn,
				     OS_FILE_LOG_BLOCK_SIZE),
		start_offset - area_start);

	srv_stats.log_padded.add(pad_size);

	log_sys->write_lsn = write_lsn;

#ifndef _WIN32
	if (srv_unix_file_flush_method == SRV_UNIX_O_DSYNC
	    || srv_unix_file_flush_method == SRV_UNIX_ALL_O_DIRECT) {
		/* O_SYNC and ALL_O_DIRECT mean the OS did not buffer the log
		file at all: so we have also flushed to disk what we have
		written */
		log_sys->flushed_to_disk_lsn = log_sys->write_lsn;
	}
#endif /* !_WIN32 */

	log_write_mutex_exit();

	if (flush_to_disk) {
		log_write_flush_to_disk_low();
	}
}

/** write to the log file up to the last log entry.
@param[in]	sync	whether we want the written log
also to be flushed to disk. */
void
log_buffer_flush_to_disk(
	bool sync)
{
	ut_ad(!srv_read_only_mode);
	log_write_up_to(log_get_lsn(), sync);
}

/****************************************************************//**
This functions writes the log buffer to the log file and if 'flush'
is set it forces a flush of the log file as well. This is meant to be
called from background master thread only as it does not wait for
the write (+ possible flush) to finish. */
void
log_buffer_sync_in_background(
/*==========================*/
	bool	flush)	/*!< in: flush the logs to disk */
{
	lsn_t	lsn;

	log_mutex_enter();

	lsn = log_sys->lsn;

	if (flush
	    && log_sys->n_pending_flushes > 0
	    && log_sys->current_flush_lsn >= lsn) {
		/* The write + flush will write enough */
		log_mutex_exit();
		return;
	}

	log_mutex_exit();

	log_write_up_to(lsn, flush);
}

/********************************************************************

Tries to establish a big enough margin of free space in the log buffer, such
that a new log entry can be catenated without an immediate need for a flush. */
static
void
log_flush_margin(void)
/*==================*/
{
	log_t*	log	= log_sys;
	lsn_t	lsn	= 0;

	log_mutex_enter();

	if (log->buf_free > log->max_buf_free) {
		/* We can write during flush */
		lsn = log->lsn;
	}

	log_mutex_exit();

	if (lsn) {
		log_write_up_to(lsn, false);
	}
}
#ifndef UNIV_HOTBACKUP
/** Advances the smallest lsn for which there are unflushed dirty blocks in the
buffer pool.
NOTE: this function may only be called if the calling thread owns no
synchronization objects!
@param[in]	new_oldest	try to advance oldest_modified_lsn at least to
this lsn
@return false if there was a flush batch of the same type running,
which means that we could not start this flush batch */
static
bool
log_preflush_pool_modified_pages(
	lsn_t			new_oldest)
{
	if (recv_recovery_on) {
		/* If the recovery is running, we must first apply all
		log records to their respective file pages to get the
		right modify lsn values to these pages: otherwise, there
		might be pages on disk which are not yet recovered to the
		current lsn, and even after calling this function, we could
		not know how up-to-date the disk version of the database is,
		and we could not make a new checkpoint on the basis of the
		info on the buffer pool only. */

		recv_apply_hashed_log_recs(TRUE);
	}

	/* better to wait for flushed by page cleaner */
	ut_ad(buf_page_cleaner_is_active);

	if (srv_flush_sync) {
		/* wake page cleaner for IO burst */
		buf_flush_request_force(new_oldest);
	}

	buf_flush_wait_flushed(new_oldest);

	return(true);
}
#endif /* !UNIV_HOTBACKUP */
/******************************************************//**
Completes a checkpoint. */
static
void
log_complete_checkpoint(void)
/*=========================*/
{
	ut_ad(log_mutex_own());
	ut_ad(log_sys->n_pending_checkpoint_writes == 0);

	log_sys->next_checkpoint_no++;

	ut_ad(log_sys->next_checkpoint_lsn >= log_sys->last_checkpoint_lsn);
	log_sys->last_checkpoint_lsn = log_sys->next_checkpoint_lsn;
	MONITOR_SET(MONITOR_LSN_CHECKPOINT_AGE,
		    log_sys->lsn - log_sys->last_checkpoint_lsn);

	DBUG_PRINT("ib_log", ("checkpoint ended at " LSN_PF
			      ", flushed to " LSN_PF,
			      log_sys->last_checkpoint_lsn,
			      log_sys->flushed_to_disk_lsn));

	rw_lock_x_unlock_gen(&(log_sys->checkpoint_lock), LOG_CHECKPOINT);
}

/******************************************************//**
Completes an asynchronous checkpoint info write i/o to a log file. */
static
void
log_io_complete_checkpoint(void)
/*============================*/
{
	MONITOR_DEC(MONITOR_PENDING_CHECKPOINT_WRITE);

	log_mutex_enter();

	ut_ad(log_sys->n_pending_checkpoint_writes > 0);

	if (--log_sys->n_pending_checkpoint_writes == 0) {
		log_complete_checkpoint();
	}

	log_mutex_exit();

	/* Wake the redo log watching thread to parse the log up to this
	checkpoint. */
	if (srv_track_changed_pages) {
		os_event_reset(srv_redo_log_tracked_event);
		os_event_set(srv_checkpoint_completed_event);
	}
}

/******************************************************//**
Writes the checkpoint info to a log group header. */
static
void
log_group_checkpoint(
/*=================*/
	log_group_t*	group)	/*!< in: log group */
{
	lsn_t		lsn_offset;
	byte*		buf;

	ut_ad(!srv_read_only_mode);
	ut_ad(log_mutex_own());
	ut_ad(srv_shutdown_state != SRV_SHUTDOWN_LAST_PHASE);
#if LOG_CHECKPOINT_SIZE > OS_FILE_LOG_BLOCK_SIZE
# error "LOG_CHECKPOINT_SIZE > OS_FILE_LOG_BLOCK_SIZE"
#endif

	DBUG_PRINT("ib_log", ("checkpoint " UINT64PF " at " LSN_PF
			      " written to group " ULINTPF,
			      log_sys->next_checkpoint_no,
			      log_sys->next_checkpoint_lsn,
			      group->id));

	buf = group->checkpoint_buf;
	memset(buf, 0, OS_FILE_LOG_BLOCK_SIZE);

#ifdef UNIV_DEBUG
	lsn_t		old_next_checkpoint_lsn
		= mach_read_from_8(buf + LOG_CHECKPOINT_LSN);
	ut_ad(old_next_checkpoint_lsn <= log_sys->next_checkpoint_lsn);
#endif /* UNIV_DEBUG */
	mach_write_to_8(buf + LOG_CHECKPOINT_NO, log_sys->next_checkpoint_no);
	mach_write_to_8(buf + LOG_CHECKPOINT_LSN, log_sys->next_checkpoint_lsn);

	lsn_offset = log_group_calc_lsn_offset(log_sys->next_checkpoint_lsn,
					       group);
	mach_write_to_8(buf + LOG_CHECKPOINT_OFFSET, lsn_offset);
	mach_write_to_8(buf + LOG_CHECKPOINT_LOG_BUF_SIZE, log_sys->buf_size);

	log_block_set_checksum(buf, log_block_calc_checksum_crc32(buf));

	MONITOR_INC(MONITOR_PENDING_CHECKPOINT_WRITE);

	log_sys->n_log_ios++;

	MONITOR_INC(MONITOR_LOG_IO);

	ut_ad(LOG_CHECKPOINT_1 < univ_page_size.physical());
	ut_ad(LOG_CHECKPOINT_2 < univ_page_size.physical());

	if (log_sys->n_pending_checkpoint_writes++ == 0) {
		rw_lock_x_lock_gen(&log_sys->checkpoint_lock,
				   LOG_CHECKPOINT);
	}

	/* Note: We alternate the physical place of the checkpoint info.
	See the (next_checkpoint_no & 1) below. */

	/* We send as the last parameter the group machine address
	added with 1, as we want to distinguish between a normal log
	file write and a checkpoint field write */

	fil_io(IORequestLogWrite, false,
	       page_id_t(group->space_id, 0),
	       univ_page_size,
	       (log_sys->next_checkpoint_no & 1)
	       ? LOG_CHECKPOINT_2 : LOG_CHECKPOINT_1,
	       OS_FILE_LOG_BLOCK_SIZE,
	       buf, (byte*) group + 1);

	ut_ad(((ulint) group & 0x1UL) == 0);
}

#ifdef UNIV_HOTBACKUP
/******************************************************//**
Writes info to a buffer of a log group when log files are created in
backup restoration. */
void
log_reset_first_header_and_checkpoint(
/*==================================*/
	byte*		hdr_buf,/*!< in: buffer which will be written to the
				start of the first log file */
	ib_uint64_t	start)	/*!< in: lsn of the start of the first log file;
				we pretend that there is a checkpoint at
				start + LOG_BLOCK_HDR_SIZE */
{
	byte*		buf;
	ib_uint64_t	lsn;

	mach_write_to_4(hdr_buf + LOG_HEADER_FORMAT,
			LOG_HEADER_FORMAT_CURRENT);
	mach_write_to_8(hdr_buf + LOG_HEADER_START_LSN, start);

	lsn = start + LOG_BLOCK_HDR_SIZE;

	/* Write the label of mysqlbackup --restore */
	strcpy((char*)hdr_buf + LOG_HEADER_CREATOR, LOG_HEADER_CREATOR_CURRENT);
	ut_sprintf_timestamp((char*) hdr_buf
			     + (LOG_HEADER_CREATOR
			     + (sizeof LOG_HEADER_CREATOR_CURRENT) - 1));
	buf = hdr_buf + LOG_CHECKPOINT_1;
	memset(buf, 0, OS_FILE_LOG_BLOCK_SIZE);

	/*mach_write_to_8(buf + LOG_CHECKPOINT_NO, 0);*/
	mach_write_to_8(buf + LOG_CHECKPOINT_LSN, lsn);

	mach_write_to_8(buf + LOG_CHECKPOINT_OFFSET,
			LOG_FILE_HDR_SIZE + LOG_BLOCK_HDR_SIZE);
	mach_write_to_8(buf + LOG_CHECKPOINT_LOG_BUF_SIZE, 2 * 1024 * 1024);

	log_block_set_checksum(buf, log_block_calc_checksum_crc32(buf));
}
#endif /* UNIV_HOTBACKUP */

#ifndef UNIV_HOTBACKUP
/** Read a log group header page to log_sys->checkpoint_buf.
@param[in]	group	log group
@param[in]	header	0 or LOG_CHEKCPOINT_1 or LOG_CHECKPOINT2 */
void
log_group_header_read(
	const log_group_t*	group,
	ulint			header)
{
	ut_ad(log_mutex_own());

	log_sys->n_log_ios++;

	MONITOR_INC(MONITOR_LOG_IO);

	fil_io(IORequestLogRead, true,
	       page_id_t(group->space_id, header / univ_page_size.physical()),
	       univ_page_size, header % univ_page_size.physical(),
	       OS_FILE_LOG_BLOCK_SIZE, log_sys->checkpoint_buf, NULL);
}

/** Write checkpoint info to the log header and invoke log_mutex_exit().
@param[in]	sync	whether to wait for the write to complete */
void
log_write_checkpoint_info(
	bool	sync)
{
	log_group_t*	group;

	ut_ad(log_mutex_own());

	if (!srv_read_only_mode) {
		for (group = UT_LIST_GET_FIRST(log_sys->log_groups);
		     group;
		     group = UT_LIST_GET_NEXT(log_groups, group)) {

			log_group_checkpoint(group);
		}
	}

	log_mutex_exit();

	MONITOR_INC(MONITOR_NUM_CHECKPOINT);

	if (sync) {
		/* Wait for the checkpoint write to complete */
		rw_lock_s_lock(&log_sys->checkpoint_lock);
		rw_lock_s_unlock(&log_sys->checkpoint_lock);

		DEBUG_SYNC_C("checkpoint_completed");

		DBUG_EXECUTE_IF(
			"crash_after_checkpoint",
			DBUG_SUICIDE(););
	}
}

/** Set extra data to be written to the redo log during checkpoint.
@param[in]	buf	data to be appended on checkpoint, or NULL
@return pointer to previous data to be appended on checkpoint */
mtr_buf_t*
log_append_on_checkpoint(
	mtr_buf_t*	buf)
{
	log_mutex_enter();
	mtr_buf_t*	old = log_sys->append_on_checkpoint;
	log_sys->append_on_checkpoint = buf;
	log_mutex_exit();
	return(old);
}

/** Make a checkpoint. Note that this function does not flush dirty
blocks from the buffer pool: it only checks what is lsn of the oldest
modification in the pool, and writes information about the lsn in
log files. Use log_make_checkpoint_at() to flush also the pool.
@param[in]	sync		whether to wait for the write to complete
@param[in]	write_always	force a write even if no log
has been generated since the latest checkpoint
@return true if success, false if a checkpoint write was already running */
bool
log_checkpoint(
	bool	sync,
	bool	write_always)
{
	lsn_t	oldest_lsn;

	ut_ad(!srv_read_only_mode);

	if (recv_recovery_is_on()) {
		recv_apply_hashed_log_recs(TRUE);
	}

#ifndef _WIN32
	switch (srv_unix_file_flush_method) {
	case SRV_UNIX_NOSYNC:
	case SRV_UNIX_ALL_O_DIRECT:
		break;
	case SRV_UNIX_O_DSYNC:
	case SRV_UNIX_FSYNC:
	case SRV_UNIX_LITTLESYNC:
	case SRV_UNIX_O_DIRECT:
	case SRV_UNIX_O_DIRECT_NO_FSYNC:
		fil_flush_file_spaces(FIL_TYPE_TABLESPACE);
	}
#endif /* !_WIN32 */

	log_mutex_enter();

	ut_ad(!recv_no_log_write);
	oldest_lsn = log_buf_pool_get_oldest_modification();

	/* Because log also contains headers and dummy log records,
	log_buf_pool_get_oldest_modification() will return log_sys->lsn
	if the buffer pool contains no dirty buffers.
	We must make sure that the log is flushed up to that lsn.
	If there are dirty buffers in the buffer pool, then our
	write-ahead-logging algorithm ensures that the log has been
	flushed up to oldest_lsn. */

	ut_ad(oldest_lsn >= log_sys->last_checkpoint_lsn);
	if (!write_always
	    && oldest_lsn
	    <= log_sys->last_checkpoint_lsn + SIZE_OF_MLOG_CHECKPOINT) {
		/* Do nothing, because nothing was logged (other than
		a MLOG_CHECKPOINT marker) since the previous checkpoint. */
		log_mutex_exit();
		return(true);
	}

	/* Repeat the MLOG_FILE_NAME records after the checkpoint, in
	case some log records between the checkpoint and log_sys->lsn
	need them. Finally, write a MLOG_CHECKPOINT marker. Redo log
	apply expects to see a MLOG_CHECKPOINT after the checkpoint,
	except on clean shutdown, where the log will be empty after
	the checkpoint.

	It is important that we write out the redo log before any
	further dirty pages are flushed to the tablespace files.  At
	this point, because log_mutex_own(), mtr_commit() in other
	threads will be blocked, and no pages can be added to the
	flush lists. */
	lsn_t		flush_lsn	= oldest_lsn;
	const bool	do_write
		= srv_shutdown_state == SRV_SHUTDOWN_NONE
		|| flush_lsn != log_sys->lsn;

	if (fil_names_clear(flush_lsn, do_write)) {
		ut_ad(log_sys->lsn >= flush_lsn + SIZE_OF_MLOG_CHECKPOINT);
		flush_lsn = log_sys->lsn;
	}

	log_mutex_exit();

	log_write_up_to(flush_lsn, true);

	DBUG_EXECUTE_IF(
		"using_wa_checkpoint_middle",
		if (write_always) {
			DEBUG_SYNC_C("wa_checkpoint_middle");

			const my_bool b = TRUE;
			buf_flush_page_cleaner_disabled_debug_update(
				NULL, NULL, NULL, &b);
			dict_stats_disabled_debug_update(
				NULL, NULL, NULL, &b);
			srv_master_thread_disabled_debug_update(
				NULL, NULL, NULL, &b);
		});

	log_mutex_enter();

	ut_ad(log_sys->flushed_to_disk_lsn >= flush_lsn);
	ut_ad(flush_lsn >= oldest_lsn);

	if (log_sys->last_checkpoint_lsn >= oldest_lsn) {
		log_mutex_exit();
		return(true);
	}

	if (log_sys->n_pending_checkpoint_writes > 0) {
		/* A checkpoint write is running */
		log_mutex_exit();

		if (sync) {
			/* Wait for the checkpoint write to complete */
			rw_lock_s_lock(&log_sys->checkpoint_lock);
			rw_lock_s_unlock(&log_sys->checkpoint_lock);
		}

		return(false);
	}

	ut_ad(oldest_lsn >= log_sys->next_checkpoint_lsn);
	log_sys->next_checkpoint_lsn = oldest_lsn;
	log_write_checkpoint_info(sync);
	ut_ad(!log_mutex_own());

	return(true);
}

/** Make a checkpoint at or after a specified LSN.
@param[in]	lsn		the log sequence number, or LSN_MAX
for the latest LSN
@param[in]	write_always	force a write even if no log
has been generated since the latest checkpoint */
void
log_make_checkpoint_at(
	lsn_t			lsn,
	bool			write_always)
{
	/* Preflush pages synchronously */

	if (srv_shutdown_state != SRV_SHUTDOWN_FLUSH_PHASE) {
		while (!log_preflush_pool_modified_pages(lsn)) {
			/* Flush as much as we can */
		}
	}

	while (!log_checkpoint(true, write_always)) {
		/* Force a checkpoint */
	}
}

/****************************************************************//**
Tries to establish a big enough margin of free space in the log groups, such
that a new log entry can be catenated without an immediate need for a
checkpoint. NOTE: this function may only be called if the calling thread
owns no synchronization objects! */
static
void
log_checkpoint_margin(void)
/*=======================*/
{
	log_t*		log		= log_sys;
	lsn_t		age;
	lsn_t		checkpoint_age;
	ib_uint64_t	advance;
	lsn_t		oldest_lsn;
	bool		success;
loop:
	advance = 0;

	log_mutex_enter();
	ut_ad(!recv_no_log_write);

	if (!log->check_flush_or_checkpoint) {
		log_mutex_exit();
		return;
	}

	oldest_lsn = log_buf_pool_get_oldest_modification();

	age = log->lsn - oldest_lsn;

	if (age > log->max_modified_age_sync) {

		/* A flush is urgent: we have to do a synchronous preflush */
		advance = age - log->max_modified_age_sync;
	}

	checkpoint_age = log->lsn - log->last_checkpoint_lsn;

	bool	checkpoint_sync;
	bool	do_checkpoint;

	if (checkpoint_age > log->max_checkpoint_age) {
		/* A checkpoint is urgent: we do it synchronously */
		checkpoint_sync = true;
		do_checkpoint = true;
	} else if (checkpoint_age > log->max_checkpoint_age_async) {
		/* A checkpoint is not urgent: do it asynchronously */
		do_checkpoint = true;
		checkpoint_sync = false;
		log->check_flush_or_checkpoint = false;
	} else {
		do_checkpoint = false;
		checkpoint_sync = false;
		log->check_flush_or_checkpoint = false;
	}

	log_mutex_exit();

	if (advance) {
		lsn_t	new_oldest = oldest_lsn + advance;

		success = log_preflush_pool_modified_pages(new_oldest);

		/* If the flush succeeded, this thread has done its part
		and can proceed. If it did not succeed, there was another
		thread doing a flush at the same time. */
		if (!success) {
			log_mutex_enter();

			log->check_flush_or_checkpoint = true;

			log_mutex_exit();
			goto loop;
		}
	}

	if (do_checkpoint) {
		log_checkpoint(checkpoint_sync, FALSE);

		if (checkpoint_sync) {

			goto loop;
		}
	}
}

/******************************************************//**
Reads a specified log segment to a buffer. Optionally releases the log mutex
before the I/O.*/
void
log_group_read_log_seg(
/*===================*/
	byte*		buf,		/*!< in: buffer where to read */
	log_group_t*	group,		/*!< in: log group */
	lsn_t		start_lsn,	/*!< in: read area start */
	lsn_t		end_lsn,	/*!< in: read area end */
	bool		release_mutex)	/*!< in: whether the log_sys->mutex
					should be released before the read */
{
	ulint	len;
	lsn_t	source_offset;

	ut_ad(log_mutex_own());

loop:
	source_offset = log_group_calc_lsn_offset(start_lsn, group);

	ut_a(end_lsn - start_lsn <= ULINT_MAX);
	len = (ulint) (end_lsn - start_lsn);

	ut_ad(len != 0);

	if ((source_offset % group->file_size) + len > group->file_size) {

		/* If the above condition is true then len (which is ulint)
		is > the expression below, so the typecast is ok */
		len = (ulint) (group->file_size -
			(source_offset % group->file_size));
	}

	log_sys->n_log_ios++;

	MONITOR_INC(MONITOR_LOG_IO);

	ut_a(source_offset / UNIV_PAGE_SIZE <= ULINT_MAX);

	if (release_mutex) {
		log_mutex_exit();
	}

	const ulint	page_no
		= (ulint) (source_offset / univ_page_size.physical());

	fil_io(IORequestLogRead, true,
	       page_id_t(group->space_id, page_no),
	       univ_page_size,
	       (ulint) (source_offset % univ_page_size.physical()),
	       len, buf, NULL);

	start_lsn += len;
	buf += len;

	if (start_lsn != end_lsn) {

		if (release_mutex) {
			log_mutex_enter();
		}
		goto loop;
	}
}

/**
Checks that there is enough free space in the log to start a new query step.
Flushes the log buffer or makes a new checkpoint if necessary. NOTE: this
function may only be called if the calling thread owns no synchronization
objects! */
void
log_check_margins(void)
{
	bool	check	= true;

	do {
		log_flush_margin();
		log_checkpoint_margin();
		log_mutex_enter();
		if (log_check_tracking_margin(0)) {
			log_mutex_exit();
			os_thread_sleep(10000);
			continue;
		}
		ut_ad(!recv_no_log_write);
		check = log_sys->check_flush_or_checkpoint;
		log_mutex_exit();
	} while (check);
}

/****************************************************************//**
Makes a checkpoint at the latest lsn and writes it to first page of each
data file in the database, so that we know that the file spaces contain
all modifications up to that lsn. This can only be called at database
shutdown. This function also writes all log in log files to the log archive. */
void
logs_empty_and_mark_files_at_shutdown(void)
/*=======================================*/
{
	lsn_t			lsn;
	lsn_t			tracked_lsn;
	ulint			count = 0;
	ulint			total_trx;
	ulint			pending_io;
	enum srv_thread_type	active_thd;
	const char*		thread_name;

	ib::info() << "Starting shutdown...";

	while (srv_fast_shutdown == 0 && trx_rollback_or_clean_is_active) {
		/* we should wait until rollback after recovery end
		for slow shutdown */
		os_thread_sleep(100000);
	}

	/* Wait until the master thread and all other operations are idle: our
	algorithm only works if the server is idle at shutdown */

	srv_shutdown_state = SRV_SHUTDOWN_CLEANUP;
loop:
	os_thread_sleep(100000);

	count++;

	/* We need the monitor threads to stop before we proceed with
	a shutdown. */

	thread_name = srv_any_background_threads_are_active();

	if (thread_name != NULL) {
		/* Print a message every 60 seconds if we are waiting
		for the monitor thread to exit. Master and worker
		threads check will be done later. */

		if (srv_print_verbose_log && count > 600) {
			ib::info() << "Waiting for " << thread_name
				<< " to exit";
			count = 0;
		}

		goto loop;
	}

	/* Check that there are no longer transactions, except for
	PREPARED ones. We need this wait even for the 'very fast'
	shutdown, because the InnoDB layer may have committed or
	prepared transactions and we don't want to lose them. */

	total_trx = trx_sys_any_active_transactions();

	if (total_trx > 0) {

		if (srv_print_verbose_log && count > 600) {
			ib::info() << "Waiting for " << total_trx << " active"
				<< " transactions to finish";

			count = 0;
		}

		goto loop;
	}

	/* Check that the background threads are suspended */

	active_thd = srv_get_active_thread_type();

	if (active_thd != SRV_NONE || srv_n_fil_crypt_threads_started) {

		if (active_thd == SRV_PURGE) {
			srv_purge_wakeup();
		}

		/* The srv_lock_timeout_thread, srv_error_monitor_thread
		and srv_monitor_thread should already exit by now. The
		only threads to be suspended are the master threads
		and worker threads (purge threads). Print the thread
		type if any of such threads not in suspended mode */
		if (srv_print_verbose_log && count > 600) {
			const char*	thread_type = "<null>";

			switch (active_thd) {
			case SRV_NONE:
				ut_ad(srv_n_fil_crypt_threads_started);
				thread_type = "encryption thread";
				break;
			case SRV_WORKER:
				thread_type = "worker threads";
				break;
			case SRV_MASTER:
				thread_type = "master thread";
				break;
			case SRV_PURGE:
				thread_type = "purge thread";
				break;
			}

			ib::info() << "Waiting for " << thread_type
				<< " to be suspended";

			count = 0;
		}

		goto loop;
	}

	/* At this point only page_cleaner should be active. We wait
	here to let it complete the flushing of the buffer pools
	before proceeding further. */
	os_rmb;
	ut_ad(buf_lru_manager_running_threads == srv_buf_pool_instances
	      || buf_lru_manager_running_threads == 0);
	srv_shutdown_state = SRV_SHUTDOWN_FLUSH_PHASE;
	count = 0;
	while (buf_page_cleaner_is_active
	       || buf_lru_manager_running_threads > 0) {

		if (srv_print_verbose_log && count == 0) {
			ib::info() << "Waiting for page_cleaner to"
				" finish flushing of buffer pool";
		}
		++count;
		os_thread_sleep(100000);
		if (count > 600) {
			count = 0;
		}

		os_rmb;
	}

	log_mutex_enter();
	const ulint	n_write	= log_sys->n_pending_checkpoint_writes;
	const ulint	n_flush	= log_sys->n_pending_flushes;
	log_mutex_exit();

	if (log_scrub_thread_active || n_write != 0 || n_flush != 0) {
		if (srv_print_verbose_log && count > 600) {
			ib::info() << "Pending checkpoint_writes: " << n_write
				<< ". Pending log flush writes: " << n_flush;
			count = 0;
		}
		goto loop;
	}

	pending_io = buf_pool_check_no_pending_io();

        if (log_scrub_thread_active) {
                ut_ad(!srv_read_only_mode);
                os_event_set(log_scrub_event);
        }

        ut_ad(!log_scrub_thread_active);

	if (pending_io) {
		if (srv_print_verbose_log && count > 600) {
			ib::info() << "Waiting for " << pending_io << " buffer"
				" page I/Os to complete";
			count = 0;
		}

		goto loop;
	}

	if (srv_fast_shutdown == 2) {
		if (!srv_read_only_mode) {
			ib::info() << "MySQL has requested a very fast"
				" shutdown without flushing the InnoDB buffer"
				" pool to data files. At the next mysqld"
				" startup InnoDB will do a crash recovery!";

			/* In this fastest shutdown we do not flush the
			buffer pool:

			it is essentially a 'crash' of the InnoDB server.
			Make sure that the log is all flushed to disk, so
			that we can recover all committed transactions in
			a crash recovery. We must not write the lsn stamps
			to the data files, since at a startup InnoDB deduces
			from the stamps if the previous shutdown was clean. */

			log_buffer_flush_to_disk();

			/* Check that the background threads stay suspended */
			thread_name = srv_any_background_threads_are_active();

			if (thread_name != NULL) {
				ib::warn() << "Background thread "
					<< thread_name << " woke up during"
					" shutdown";
				goto loop;
			}
		}

		srv_shutdown_state = SRV_SHUTDOWN_LAST_PHASE;

		/* Wake the log tracking thread which will then immediatelly
		quit because of srv_shutdown_state value */
		if (srv_redo_log_thread_started) {
			os_event_reset(srv_redo_log_tracked_event);
			os_event_set(srv_checkpoint_completed_event);
		}

		fil_close_all_files();

		thread_name = srv_any_background_threads_are_active();

		ut_a(!thread_name);

		return;
	}

	if (!srv_read_only_mode) {
		log_make_checkpoint_at(LSN_MAX, TRUE);
	}

	log_mutex_enter();

	tracked_lsn = log_get_tracked_lsn();

	lsn = log_sys->lsn;

	/** If innodb_force_recovery is set to 6 then log_sys doesn't
	have recent checkpoint information. So last checkpoint lsn
	will never be equal to current lsn. */
	const bool      is_last =
		((srv_force_recovery == SRV_FORCE_NO_LOG_REDO
		  && lsn == log_sys->last_checkpoint_lsn
		  + LOG_BLOCK_HDR_SIZE)
		 || lsn == log_sys->last_checkpoint_lsn)
		&& (!srv_track_changed_pages
		    || tracked_lsn == log_sys->last_checkpoint_lsn);

	ut_ad(lsn >= log_sys->last_checkpoint_lsn);

	log_mutex_exit();

	if (!is_last) {
		goto loop;
	}

	/* Check that the background threads stay suspended */
	thread_name = srv_any_background_threads_are_active();
	if (thread_name != NULL) {
		ib::warn() << "Background thread " << thread_name << " woke up"
			" during shutdown";

		goto loop;
	}

	if (!srv_read_only_mode) {
		fil_flush_file_spaces(FIL_TYPE_TABLESPACE);
		fil_flush_file_spaces(FIL_TYPE_LOG);
	}

	/* The call fil_write_flushed_lsn() will bypass the buffer
	pool: therefore it is essential that the buffer pool has been
	completely flushed to disk! (We do not call fil_write... if the
	'very fast' shutdown is enabled.) */

	if (!buf_all_freed()) {

		if (srv_print_verbose_log && count > 600) {
			ib::info() << "Waiting for dirty buffer pages to be"
				" flushed";
			count = 0;
		}

		goto loop;
	}

	srv_shutdown_state = SRV_SHUTDOWN_LAST_PHASE;

	/* Signal the log following thread to quit */
	if (srv_redo_log_thread_started) {
		os_event_reset(srv_redo_log_tracked_event);
		os_event_set(srv_checkpoint_completed_event);
	}

	/* Make some checks that the server really is quiet */
	srv_thread_type	type = srv_get_active_thread_type();
	ut_a(type == SRV_NONE);

	bool	freed = buf_all_freed();
	ut_a(freed);

	ut_a(lsn == log_sys->lsn);
	ut_ad(srv_force_recovery >= SRV_FORCE_NO_LOG_REDO
	      || lsn == log_sys->last_checkpoint_lsn);

	if (lsn < srv_start_lsn) {
		ib::error() << "Log sequence number at shutdown " << lsn
			<< " is lower than at startup " << srv_start_lsn
			<< "!";
	}

	srv_shutdown_lsn = lsn;

	if (!srv_read_only_mode) {
		fil_write_flushed_lsn(lsn);
	}

	fil_close_all_files();

	/* Make some checks that the server really is quiet */
	type = srv_get_active_thread_type();
	ut_a(type == SRV_NONE);

	freed = buf_all_freed();
	ut_a(freed);

	ut_a(lsn == log_sys->lsn);
}

/******************************************************//**
Peeks the current lsn.
@return TRUE if success, FALSE if could not get the log system mutex */
ibool
log_peek_lsn(
/*=========*/
	lsn_t*	lsn)	/*!< out: if returns TRUE, current lsn is here */
{
	if (0 == mutex_enter_nowait(&(log_sys->mutex))) {
		*lsn = log_sys->lsn;

		log_mutex_exit();

		return(TRUE);
	}

	return(FALSE);
}

/******************************************************//**
Prints info of the log. */
void
log_print(
/*======*/
	FILE*	file)	/*!< in: file where to print */
{
	double	time_elapsed;
	time_t	current_time;

	log_mutex_enter();

	fprintf(file,
		"Log sequence number " LSN_PF "\n"
		"Log flushed up to   " LSN_PF "\n"
		"Pages flushed up to " LSN_PF "\n"
		"Last checkpoint at  " LSN_PF "\n",
		log_sys->lsn,
		log_sys->flushed_to_disk_lsn,
		log_buf_pool_get_oldest_modification(),
		log_sys->last_checkpoint_lsn);

	fprintf(file,
		"Max checkpoint age    " LSN_PF "\n"
		"Checkpoint age target " LSN_PF "\n"
		"Modified age          " LSN_PF "\n"
		"Checkpoint age        " LSN_PF "\n",
		log_sys->max_checkpoint_age,
		log_sys->max_checkpoint_age_async,
		log_sys->lsn -log_buf_pool_get_oldest_modification(),
		log_sys->lsn - log_sys->last_checkpoint_lsn);

	current_time = time(NULL);

	time_elapsed = difftime(current_time,
				log_sys->last_printout_time);

	if (time_elapsed <= 0) {
		time_elapsed = 1;
	}

	fprintf(file,
		ULINTPF " pending log flushes, "
		ULINTPF " pending chkp writes\n"
		ULINTPF " log i/o's done, %.2f log i/o's/second\n",
		log_sys->n_pending_flushes,
		log_sys->n_pending_checkpoint_writes,
		log_sys->n_log_ios,
		static_cast<double>(
			log_sys->n_log_ios - log_sys->n_log_ios_old)
		/ time_elapsed);

	if (srv_track_changed_pages) {

		/* The maximum tracked LSN age is equal to the maximum
		checkpoint age */
		fprintf(file,
			"Log tracking enabled\n"
			"Log tracked up to   " LSN_PF "\n"
			"Max tracked LSN age " LSN_PF "\n",
			log_get_tracked_lsn(),
			log_sys->max_checkpoint_age);
	}

	log_sys->n_log_ios_old = log_sys->n_log_ios;
	log_sys->last_printout_time = current_time;

	log_mutex_exit();
}

/**********************************************************************//**
Refreshes the statistics used to print per-second averages. */
void
log_refresh_stats(void)
/*===================*/
{
	log_sys->n_log_ios_old = log_sys->n_log_ios;
	log_sys->last_printout_time = time(NULL);
}

/********************************************************//**
Closes a log group. */
static
void
log_group_close(
/*===========*/
	log_group_t*	group)		/* in,own: log group to close */
{
	ulint	i;

	for (i = 0; i < group->n_files; i++) {
		ut_free(group->file_header_bufs_ptr[i]);
	}

	ut_free(group->file_header_bufs_ptr);
	ut_free(group->file_header_bufs);
	ut_free(group->checkpoint_buf_ptr);
	ut_free(group);
}

/********************************************************//**
Closes all log groups. */
void
log_group_close_all(void)
/*=====================*/
{
	log_group_t*	group;

	group = UT_LIST_GET_FIRST(log_sys->log_groups);

	while (UT_LIST_GET_LEN(log_sys->log_groups) > 0) {
		log_group_t*	prev_group = group;

		group = UT_LIST_GET_NEXT(log_groups, group);

		UT_LIST_REMOVE(log_sys->log_groups, prev_group);

		log_group_close(prev_group);
	}
}

/********************************************************//**
Shutdown the log system but do not release all the memory. */
void
log_shutdown(void)
/*==============*/
{
	log_group_close_all();

	ut_free(log_sys->buf_ptr);
	log_sys->buf_ptr = NULL;
	log_sys->buf = NULL;
	ut_free(log_sys->checkpoint_buf_ptr);
	log_sys->checkpoint_buf_ptr = NULL;
	log_sys->checkpoint_buf = NULL;

	os_event_destroy(log_sys->flush_event);

	rw_lock_free(&log_sys->checkpoint_lock);

	mutex_free(&log_sys->mutex);
	mutex_free(&log_sys->write_mutex);
	mutex_free(&log_sys->log_flush_order_mutex);

	if (!srv_read_only_mode && srv_scrub_log) {
		os_event_destroy(log_scrub_event);
	}

	recv_sys_close();
}

/********************************************************//**
Free the log system data structures. */
void
log_mem_free(void)
/*==============*/
{
	if (log_sys != NULL) {
		recv_sys_mem_free();
		ut_free(log_sys);

		log_sys = NULL;
	}
}

static
void
log_pad_current_log_block(void)
/*===========================*/
{

        ut_ad(!recv_no_log_write);
        lsn_t lsn = log_reserve_and_open(OS_FILE_LOG_BLOCK_SIZE);

        ulint pad_length = OS_FILE_LOG_BLOCK_SIZE
                - log_sys->buf_free % OS_FILE_LOG_BLOCK_SIZE
		- LOG_BLOCK_TRL_SIZE;
        if (pad_length == (OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_HDR_SIZE
		- LOG_BLOCK_TRL_SIZE)) {
                pad_length = 0;
        }

        if (pad_length) {
                srv_stats.n_log_scrubs.inc();
        }

        for (ulint i = 0; i < pad_length; i++) {
		byte b = MLOG_DUMMY_RECORD;
                log_write_low(&b, 1);
        }

        lsn = log_sys->lsn;

        log_close();

        ut_a(lsn % OS_FILE_LOG_BLOCK_SIZE == LOG_BLOCK_HDR_SIZE);
}

/*****************************************************************//*
If no log record has been written for a while, fill current log
block with dummy records. */
static
void
log_scrub()
/*=========*/
{
        log_mutex_enter();
        ulint cur_lbn = log_block_convert_lsn_to_no(log_sys->lsn);

        if (next_lbn_to_pad == cur_lbn)
        {
                log_pad_current_log_block();
        }

        next_lbn_to_pad = log_block_convert_lsn_to_no(log_sys->lsn);
        log_mutex_exit();
}

/* log scrubbing speed, in bytes/sec */
ulonglong innodb_scrub_log_speed;

/*****************************************************************//**
This is the main thread for log scrub. It waits for an event and
when waked up fills current log block with dummy records and
sleeps again.
@return this function does not return, it calls os_thread_exit() */
extern "C"
os_thread_ret_t
DECLARE_THREAD(log_scrub_thread)(void*)
{
        ut_ad(!srv_read_only_mode);

        while (srv_shutdown_state < SRV_SHUTDOWN_FLUSH_PHASE) {
                /* log scrubbing interval in µs. */
                ulonglong interval = 1000*1000*512/innodb_scrub_log_speed;

                os_event_wait_time(log_scrub_event, static_cast<ulint>(interval));

                log_scrub();

                os_event_reset(log_scrub_event);
        }

        log_scrub_thread_active = false;

        /* We count the number of threads in os_thread_exit(). A created
        thread should always use that to exit and not use return() to exit. */
        os_thread_exit();

        OS_THREAD_DUMMY_RETURN;
}
#endif /* !UNIV_HOTBACKUP */
