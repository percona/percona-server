/*****************************************************************************

Copyright (c) 2015 Percona Inc. All Rights Reserved.
Copyright (c) 1995, 2013, Oracle and/or its affiliates. All rights reserved.
Copyright (c) 2009, Google Inc.

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
@file include/log0archive.h
Redo log archiving
*******************************************************/

#ifndef log0archive_h
#define log0archive_h

#include "univ.i"
#include "log0types.h" /* lsn_t */
#include "log0log.h" /* log_group_t */

/* Pointer to this variable is used as the i/o-message when we do i/o to an
archive */
extern byte	log_archive_io;

#define LOG_ARCHIVE	11122331
#define LOG_RECOVER	98887331

#define LOG_ARCH_ON		71
#define LOG_ARCH_STOPPING	72
#define LOG_ARCH_STOPPING2	73
#define LOG_ARCH_STOPPED	74
#define LOG_ARCH_OFF		75

#define LOG_ARCHIVE_BUF_SIZE	(srv_log_buffer_size * UNIV_PAGE_SIZE / 4)

/******************************************************//**
Completes an archiving i/o. */

void
log_io_complete_archive(void);
/*=========================*/

/****************************************************************//**
Tries to establish a big enough margin of free space in the log groups, such
that a new log entry can be catenated without an immediate need for
archiving. */

void
log_archive_margin(void);
/*====================*/

/********************************************************************//**
Starts an archiving operation.
@return	true if succeed, false if an archiving operation was already running */

bool
log_archive_do(
/*===========*/
	bool	sync,	/*!< in: true if synchronous operation is desired */
	ulint*	n_bytes);/*!< out: archive log buffer size, 0 if nothing to
			 archive */

/****************************************************************//**
Writes the log contents to the archive at least up to the lsn when this
function was called. */

void
log_archive_all(void);


/*****************************************************//**
Closes the possible open archive log file (for each group) the first group,
and if it was open, increments the group file count by 2, if desired. */

void
log_archive_close_groups(
/*=====================*/
	bool	increment_file_count);	/*!< in: true if we want to increment
					the file count */

/****************************************************************//**
Stop archiving the log so that a gap may occur in the archived log files.
@return	DB_SUCCESS or DB_ERROR */

ulint
log_archive_noarchivelog(void);
/*==========================*/

/****************************************************************//**
Start archiving the log so that a gap may occur in the archived log files.
@return	DB_SUCCESS or DB_ERROR */

ulint
log_archive_archivelog(void);
/*========================*/

/******************************************************//**
Generates an archived log file name. */

void
log_archived_file_name_gen(
/*=======================*/
	char*	buf,	/*!< in: buffer where to write */
	ulint	buf_len,/*!< in: buffer length */
	ulint	id,	/*!< in: group id */
	lsn_t	file_no);/*!< in: file number */

/******************************************************//**
Get offset within archived log file to continue to write
with. */

void
log_archived_get_offset(
/*====================*/
	log_group_t*	group,		/*!< in: log group */
	lsn_t		file_no,	/*!< in: archive log file number */
	lsn_t		archived_lsn,	/*!< in: last archived LSN */
	lsn_t*		offset);	/*!< out: offset within archived file */

#endif
