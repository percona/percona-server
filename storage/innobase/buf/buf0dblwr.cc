/*****************************************************************************

Copyright (c) 1995, 2018, Oracle and/or its affiliates. All Rights Reserved.
Copyright (c) 2016, Percona Inc. All Rights Reserved.

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

/** @file buf/buf0dblwr.cc
 Doublwrite buffer module

 Created 2011/12/19
 *******************************************************/

#include <sys/types.h>

#include "buf0buf.h"
#include "buf0checksum.h"
#include "buf0dblwr.h"
#include "ha_prototypes.h"
#include "my_compiler.h"
#include "my_inttypes.h"
#include "os0file.h"
#include "page0zip.h"
#include "srv0srv.h"
#include "srv0start.h"
#include "trx0purge.h"

/** The doublewrite buffer */
buf_dblwr_t *buf_dblwr = NULL;

/** Set to TRUE when the doublewrite buffer is being created */
ibool buf_dblwr_being_created = FALSE;

/** Determines if a page number is located inside the doublewrite buffer.
 @return true if the location is inside the two blocks of the
 doublewrite buffer */
ibool buf_dblwr_page_inside(page_no_t page_no) /*!< in: page number */
{
  if (buf_dblwr == NULL) {
    return (FALSE);
  }

  if (page_no >= buf_dblwr->block1 &&
      page_no < buf_dblwr->block1 + TRX_SYS_DOUBLEWRITE_BLOCK_SIZE) {
    return (TRUE);
  }

  if (page_no >= buf_dblwr->block2 &&
      page_no < buf_dblwr->block2 + TRX_SYS_DOUBLEWRITE_BLOCK_SIZE) {
    return (TRUE);
  }

  return (FALSE);
}

/** Calls buf_page_get() on the TRX_SYS_PAGE and returns a pointer to the
 doublewrite buffer within it.
 @return pointer to the doublewrite buffer within the filespace header
 page. */
UNIV_INLINE
byte *buf_dblwr_get(mtr_t *mtr) /*!< in/out: MTR to hold the page latch */
{
  buf_block_t *block;

  block = buf_page_get(page_id_t(TRX_SYS_SPACE, TRX_SYS_PAGE_NO),
                       univ_page_size, RW_X_LATCH, mtr);

  buf_block_dbg_add_level(block, SYNC_NO_ORDER_CHECK);

  return (buf_block_get_frame(block) + TRX_SYS_DOUBLEWRITE);
}

/** Flush a batch of writes to the datafiles that have already been
 written to the dblwr buffer on disk. */
void buf_dblwr_sync_datafiles() {
  /* Wake possible simulated aio thread to actually post the
  writes to the operating system */
  os_aio_simulated_wake_handler_threads();

  /* Wait that all async writes to tablespaces have been posted to
  the OS */
  os_aio_wait_until_no_pending_writes();

  /* Now we flush the data to disk (for example, with fsync) */
  fil_flush_file_spaces(to_int(FIL_TYPE_TABLESPACE));
}

/** Creates or initialializes the doublewrite buffer at a database start. */
static void buf_dblwr_init(
    byte *doublewrite) /*!< in: pointer to the doublewrite buf
                       header on trx sys page */
{
  ulint buf_size;

  buf_dblwr = static_cast<buf_dblwr_t *>(ut_zalloc_nokey(sizeof(buf_dblwr_t)));

  /* There are two blocks of same size in the doublewrite
  buffer. */
  buf_size = 2 * TRX_SYS_DOUBLEWRITE_BLOCK_SIZE;

  /* There must be atleast one buffer for single page writes
  and one buffer for batch writes. */
  ut_a(srv_doublewrite_batch_size > 0 && srv_doublewrite_batch_size < buf_size);

  mutex_create(LATCH_ID_BUF_DBLWR, &buf_dblwr->mutex);

  buf_dblwr->s_event = os_event_create("dblwr_single_event");
  buf_dblwr->s_reserved = 0;

  buf_dblwr->block1 =
      mach_read_from_4(doublewrite + TRX_SYS_DOUBLEWRITE_BLOCK1);
  buf_dblwr->block2 =
      mach_read_from_4(doublewrite + TRX_SYS_DOUBLEWRITE_BLOCK2);

  buf_dblwr->in_use =
      static_cast<bool *>(ut_zalloc_nokey(buf_size * sizeof(bool)));

  buf_dblwr->write_buf_unaligned =
      static_cast<byte *>(ut_malloc_nokey((1 + buf_size) * UNIV_PAGE_SIZE));

  buf_dblwr->write_buf = static_cast<byte *>(
      ut_align(buf_dblwr->write_buf_unaligned, UNIV_PAGE_SIZE));

  buf_dblwr->buf_block_arr =
      static_cast<buf_page_t **>(ut_zalloc_nokey(buf_size * sizeof(void *)));
}

/** Creates the doublewrite buffer to a new InnoDB installation. The header of
 the doublewrite buffer is placed on the trx system header page.
 @return true if successful, false if not. */
MY_ATTRIBUTE((warn_unused_result))
bool buf_dblwr_create(void) {
  buf_block_t *block2;
  buf_block_t *new_block;
  byte *doublewrite;
  byte *fseg_header;
  page_no_t page_no;
  page_no_t prev_page_no;
  ulint i;
  mtr_t mtr;

  static const char *cannot_continue =
      "Cannot create doublewrite buffer: you must increase"
      " your buffer pool size. Cannot continue operation.";

  if (buf_dblwr) {
    /* Already inited */

    return (true);
  }

start_again:
  mtr_start(&mtr);
  buf_dblwr_being_created = TRUE;

  doublewrite = buf_dblwr_get(&mtr);

  if (mach_read_from_4(doublewrite + TRX_SYS_DOUBLEWRITE_MAGIC) ==
      TRX_SYS_DOUBLEWRITE_MAGIC_N) {
    /* The doublewrite buffer has already been created:
    just read in some numbers */

    buf_dblwr_init(doublewrite);

    mtr_commit(&mtr);
    buf_dblwr_being_created = FALSE;
    return (true);
  }

  ib::info(ER_IB_MSG_95) << "Doublewrite buffer not found: creating new";

  ulint min_doublewrite_size =
      ((2 * TRX_SYS_DOUBLEWRITE_BLOCK_SIZE + FSP_EXTENT_SIZE / 2 + 100) *
       UNIV_PAGE_SIZE);
  if (buf_pool_get_curr_size() < min_doublewrite_size) {
    ib::error(ER_IB_MSG_96) << cannot_continue;

    mtr_commit(&mtr);
    buf_dblwr_being_created = FALSE;
    return (false);
  }

  block2 = fseg_create(TRX_SYS_SPACE, TRX_SYS_PAGE_NO,
                       TRX_SYS_DOUBLEWRITE + TRX_SYS_DOUBLEWRITE_FSEG, &mtr);

  /* fseg_create acquires a second latch on the page,
  therefore we must declare it: */

  buf_block_dbg_add_level(block2, SYNC_NO_ORDER_CHECK);

  if (block2 == NULL) {
    ib::error(ER_IB_MSG_97) << cannot_continue;

    /* We exit without committing the mtr to prevent
    its modifications to the database getting to disk */

    mtr_commit(&mtr);
    buf_dblwr_being_created = FALSE;
    return (false);
  }

  fseg_header = doublewrite + TRX_SYS_DOUBLEWRITE_FSEG;
  prev_page_no = 0;

  for (i = 0; i < 2 * TRX_SYS_DOUBLEWRITE_BLOCK_SIZE + FSP_EXTENT_SIZE / 2;
       i++) {
    new_block =
        fseg_alloc_free_page(fseg_header, prev_page_no + 1, FSP_UP, &mtr);
    if (new_block == NULL) {
      ib::error(ER_IB_MSG_98) << cannot_continue;

      mtr_commit(&mtr);
      buf_dblwr_being_created = FALSE;
      return (false);
    }

    /* We read the allocated pages to the buffer pool;
    when they are written to disk in a flush, the space
    id and page number fields are also written to the
    pages. When we at database startup read pages
    from the doublewrite buffer, we know that if the
    space id and page number in them are the same as
    the page position in the tablespace, then the page
    has not been written to in doublewrite. */

    ut_ad(rw_lock_get_x_lock_count(&new_block->lock) == 1);
    page_no = new_block->page.id.page_no();

    if (i == FSP_EXTENT_SIZE / 2) {
      ut_a(page_no == FSP_EXTENT_SIZE);
      mlog_write_ulint(doublewrite + TRX_SYS_DOUBLEWRITE_BLOCK1, page_no,
                       MLOG_4BYTES, &mtr);
      mlog_write_ulint(
          doublewrite + TRX_SYS_DOUBLEWRITE_REPEAT + TRX_SYS_DOUBLEWRITE_BLOCK1,
          page_no, MLOG_4BYTES, &mtr);

    } else if (i == FSP_EXTENT_SIZE / 2 + TRX_SYS_DOUBLEWRITE_BLOCK_SIZE) {
      ut_a(page_no == 2 * FSP_EXTENT_SIZE);
      mlog_write_ulint(doublewrite + TRX_SYS_DOUBLEWRITE_BLOCK2, page_no,
                       MLOG_4BYTES, &mtr);
      mlog_write_ulint(
          doublewrite + TRX_SYS_DOUBLEWRITE_REPEAT + TRX_SYS_DOUBLEWRITE_BLOCK2,
          page_no, MLOG_4BYTES, &mtr);

    } else if (i > FSP_EXTENT_SIZE / 2) {
      ut_a(page_no == prev_page_no + 1);
    }

    if (((i + 1) & 15) == 0) {
      /* rw_locks can only be recursively x-locked
      2048 times. (on 32 bit platforms,
      (lint) 0 - (X_LOCK_DECR * 2049)
      is no longer a negative number, and thus
      lock_word becomes like a shared lock).
      For 4k page size this loop will
      lock the fseg header too many times. Since
      this code is not done while any other threads
      are active, restart the MTR occasionally. */
      mtr_commit(&mtr);
      mtr_start(&mtr);
      doublewrite = buf_dblwr_get(&mtr);
      fseg_header = doublewrite + TRX_SYS_DOUBLEWRITE_FSEG;
    }

    prev_page_no = page_no;
  }

  mlog_write_ulint(doublewrite + TRX_SYS_DOUBLEWRITE_MAGIC,
                   TRX_SYS_DOUBLEWRITE_MAGIC_N, MLOG_4BYTES, &mtr);
  mlog_write_ulint(
      doublewrite + TRX_SYS_DOUBLEWRITE_MAGIC + TRX_SYS_DOUBLEWRITE_REPEAT,
      TRX_SYS_DOUBLEWRITE_MAGIC_N, MLOG_4BYTES, &mtr);

  mlog_write_ulint(doublewrite + TRX_SYS_DOUBLEWRITE_SPACE_ID_STORED,
                   TRX_SYS_DOUBLEWRITE_SPACE_ID_STORED_N, MLOG_4BYTES, &mtr);
  mtr_commit(&mtr);

  /* Flush the modified pages to disk and make a checkpoint */
  log_make_latest_checkpoint();

  /* Remove doublewrite pages from LRU */
  buf_pool_invalidate();

  ib::info(ER_IB_MSG_99) << "Doublewrite buffer created";

  goto start_again;
}

/** Compute the path to the parallel doublewrite buffer, if not already done */
MY_ATTRIBUTE((warn_unused_result))
static dberr_t buf_parallel_dblwr_make_path(void) noexcept {
  if (parallel_dblwr_buf.path) return (DB_SUCCESS);

  char path[FN_REFLEN + 1 /* OS_PATH_SEPARATOR */];
  const char *dir = nullptr;

  ut_ad(srv_parallel_doublewrite_path);

  if (Fil_path::is_absolute_path(srv_parallel_doublewrite_path)) {
    strncpy(path, srv_parallel_doublewrite_path, sizeof(path));
  } else {
    /* A relative path to the parallel doublewrite file is based either on
    srv_data_home, either mysql data directory if the former is empty. */
    dir = srv_data_home[0] ? srv_data_home : MySQL_datadir_path();
    if (dir[strlen(dir) - 1] == OS_PATH_SEPARATOR)
      snprintf(path, sizeof(path), "%s%s", dir, srv_parallel_doublewrite_path);
    else
      snprintf(path, sizeof(path), "%s%c%s", dir, OS_PATH_SEPARATOR,
               srv_parallel_doublewrite_path);
  }

  os_file_type_t type;
  bool exists = false;

  bool ret = os_file_status(path, &exists, &type);

  /* For realpath() to succeed the file must exist. */

  if (ret && exists) {
    if (my_realpath(path, path, MY_WME) != 0) return (DB_ERROR);
    if (type != OS_FILE_TYPE_FILE) {
      ib::error() << "Parallel doublewrite path " << path
                  << " must point to "
                     "a regular file";
      return (DB_WRONG_FILE_NAME);
    }
  } else if (!Fil_path::is_absolute_path(srv_parallel_doublewrite_path)) {
    /* If it does not exist, and is not an absolute path, then resolve only the
    directory part and append srv_parallel_doublewrite_path to it. */
    char dir_full[FN_REFLEN];

    if (my_realpath(dir_full, dir, MY_WME) != 0) return (DB_ERROR);

    if (dir_full[strlen(dir_full) - 1] == OS_PATH_SEPARATOR)
      snprintf(path, sizeof(path), "%s%s", dir_full,
               srv_parallel_doublewrite_path);
    else
      snprintf(path, sizeof(path), "%s%c%s", dir_full, OS_PATH_SEPARATOR,
               srv_parallel_doublewrite_path);
  }

  parallel_dblwr_buf.path = mem_strdup(path);
  return (parallel_dblwr_buf.path ? DB_SUCCESS : DB_OUT_OF_MEMORY);
}

/** Close the parallel doublewrite buffer file */
static void buf_parallel_dblwr_close(void) noexcept {
  if (!parallel_dblwr_buf.file.is_closed()) {
    os_file_close(parallel_dblwr_buf.file);
    parallel_dblwr_buf.file.set_closed();
  }
}

/** Maximum possible parallel doublewrite buffer file size in bytes */
#define MAX_DOUBLEWRITE_FILE_SIZE \
  ((MAX_DOUBLEWRITE_BATCH_SIZE) * (MAX_DBLWR_SHARDS) * (UNIV_PAGE_SIZE))

/**
At database startup initializes the doublewrite buffer memory structure if
we already have a doublewrite buffer created in the data files. If we are
upgrading to an InnoDB version which supports multiple tablespaces, then this
function performs the necessary update operations. If we are in a crash
recovery, this function loads the pages from double write buffer into memory.
@param[in]	file		File handle
@param[in]	path		Path name of file
@return DB_SUCCESS or error code */
dberr_t buf_dblwr_init_or_load_pages(pfs_os_file_t file, const char *path) {
  byte *buf;
  byte *page;
  page_no_t block1;
  page_no_t block2;
  space_id_t space_id;
  byte *read_buf;
  byte *doublewrite;
  byte *unaligned_read_buf;
  ibool reset_space_ids = FALSE;
  recv_dblwr_t &recv_dblwr = recv_sys->dblwr;

  if (srv_read_only_mode) {
    ib::info() << "Skipping doublewrite buffer processing due to "
                  "InnoDB running in read only mode";
    return (DB_SUCCESS);
  }

  /* We do the file i/o past the buffer pool */

  unaligned_read_buf = static_cast<byte *>(ut_malloc_nokey(2 * UNIV_PAGE_SIZE));

  read_buf = static_cast<byte *>(ut_align(unaligned_read_buf, UNIV_PAGE_SIZE));

  /* Read the trx sys header to check if we are using the doublewrite
  buffer */
  dberr_t err;

  IORequest read_request(IORequest::READ);

  read_request.disable_compression();

  err = os_file_read(read_request, file, read_buf,
                     TRX_SYS_PAGE_NO * UNIV_PAGE_SIZE, UNIV_PAGE_SIZE);

  if (err != DB_SUCCESS) {
    ib::error(ER_IB_MSG_100)
        << "Failed to read the system tablespace header page";

    ut_free(unaligned_read_buf);

    return (err);
  }

  doublewrite = read_buf + TRX_SYS_DOUBLEWRITE;

  if (mach_read_from_4(doublewrite + TRX_SYS_DOUBLEWRITE_MAGIC) ==
      TRX_SYS_DOUBLEWRITE_MAGIC_N) {
    /* The doublewrite buffer has been created */

    buf_dblwr_init(doublewrite);

    block1 = buf_dblwr->block1;
    block2 = buf_dblwr->block2;

    buf = buf_dblwr->write_buf;
  } else {
    ut_free(unaligned_read_buf);
    return (DB_SUCCESS);
  }

  if (mach_read_from_4(doublewrite + TRX_SYS_DOUBLEWRITE_SPACE_ID_STORED) !=
      TRX_SYS_DOUBLEWRITE_SPACE_ID_STORED_N) {
    /* We are upgrading from a version < 4.1.x to a version where
    multiple tablespaces are supported. We must reset the space id
    field in the pages in the doublewrite buffer because starting
    from this version the space id is stored to
    FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID. */

    reset_space_ids = TRUE;

    ib::info(ER_IB_MSG_1266)
        << "Resetting space id's in the doublewrite buffer";
  }

  /* Read the pages from the doublewrite buffer to memory */
  err = os_file_read(read_request, file, buf, block1 * UNIV_PAGE_SIZE,
                     TRX_SYS_DOUBLEWRITE_BLOCK_SIZE * UNIV_PAGE_SIZE);

  if (err != DB_SUCCESS) {
    ib::error(ER_IB_MSG_101) << "Failed to read the first double write buffer "
                                "extent";

    ut_free(unaligned_read_buf);

    return (err);
  }

  err = os_file_read(
      read_request, file, buf + TRX_SYS_DOUBLEWRITE_BLOCK_SIZE * UNIV_PAGE_SIZE,
      block2 * UNIV_PAGE_SIZE, TRX_SYS_DOUBLEWRITE_BLOCK_SIZE * UNIV_PAGE_SIZE);

  if (err != DB_SUCCESS) {
    ib::error(ER_IB_MSG_102) << "Failed to read the second double write buffer "
                                "extent";

    ut_free(unaligned_read_buf);

    return (err);
  }

  /* Check if any of these pages is half-written in data files, in the
  intended position */

  page = buf;

  for (page_no_t i = 0; i < TRX_SYS_DOUBLEWRITE_BLOCK_SIZE * 2; i++) {
    if (reset_space_ids) {
      page_no_t source_page_no;

      space_id = 0;
      mach_write_to_4(page + FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID, space_id);
      /* We do not need to calculate new checksums for the
      pages because the field .._SPACE_ID does not affect
      them. Write the page back to where we read it from. */

      if (i < TRX_SYS_DOUBLEWRITE_BLOCK_SIZE) {
        source_page_no = block1 + i;
      } else {
        source_page_no = block2 + i - TRX_SYS_DOUBLEWRITE_BLOCK_SIZE;
      }

      IORequest write_request(IORequest::WRITE);

      /* Recovered data file pages are written out
      as uncompressed. */

      write_request.disable_compression();

      err = os_file_write(write_request, path, file, page,
                          source_page_no * UNIV_PAGE_SIZE, UNIV_PAGE_SIZE);

      if (err != DB_SUCCESS) {
        ib::error(ER_IB_MSG_103) << "Failed to write to the double write"
                                    " buffer";

        ut_free(unaligned_read_buf);

        return (err);
      }

    } else {
      recv_dblwr.add_to_sys(page);
    }

    page += univ_page_size.physical();
  }

  err = buf_parallel_dblwr_make_path();
  if (err != DB_SUCCESS) {
    ut_free(unaligned_read_buf);
    return (err);
  }

  ut_ad(parallel_dblwr_buf.file.is_closed());
  bool success;
  parallel_dblwr_buf.file = os_file_create_simple_no_error_handling(
      innodb_parallel_dblwrite_file_key, parallel_dblwr_buf.path, OS_FILE_OPEN,
      OS_FILE_READ_ONLY, true, &success);
  if (!success) {
    /* We are not supposed to check errno != ENOENT directly, but
    os_file_get_last_error will spam error log if it's handled
    there. */
    if (errno != ENOENT) {
      os_file_get_last_error(true);
      ib::error() << "Failed to open the parallel doublewrite "
                     "buffer at "
                  << parallel_dblwr_buf.path;
      ut_free(unaligned_read_buf);
      return (DB_CANNOT_OPEN_FILE);
    }
    /* Failed to open because the file did not exist: OK */
    ib::info() << "Crash recovery did not find the parallel "
                  "doublewrite buffer at "
               << parallel_dblwr_buf.path;
  } else {
    /* Cannot possibly be upgrading from 4.1 */
    ut_ad(!reset_space_ids);

    os_file_set_nocache(parallel_dblwr_buf.file, parallel_dblwr_buf.path,
                        "open", false);

    os_offset_t size = os_file_get_size(parallel_dblwr_buf.file);

    if (size > MAX_DOUBLEWRITE_FILE_SIZE) {
      ib::error() << "Parallel doublewrite buffer size " << size
                  << " bytes is larger than the maximum "
                     "size "
                  << MAX_DOUBLEWRITE_FILE_SIZE
                  << " bytes supported by this server "
                     "version";
      buf_parallel_dblwr_close();
      ut_free(unaligned_read_buf);
      return (DB_CORRUPTION);
    }

    if (size % UNIV_PAGE_SIZE) {
      ib::error() << "Parallel doublewrite buffer size " << size
                  << " bytes is not a multiple of "
                     "a page size "
                  << UNIV_PAGE_SIZE << " bytes";
      buf_parallel_dblwr_close();
      ut_free(unaligned_read_buf);
      return (DB_CORRUPTION);
    }

    if (size == 0) {
      ib::info() << "Parallel doublewrite buffer is zero-sized";
      buf_parallel_dblwr_close();
      ut_free(unaligned_read_buf);
      return (DB_SUCCESS);
    }

    ib::info() << "Recovering partial pages from the parallel "
                  "doublewrite buffer at "
               << parallel_dblwr_buf.path;

    parallel_dblwr_buf.recovery_buf_unaligned = static_cast<byte *>(
        ut_malloc(size + UNIV_PAGE_SIZE, mem_key_parallel_doublewrite));
    if (!parallel_dblwr_buf.recovery_buf_unaligned) {
      buf_parallel_dblwr_close();
      ut_free(unaligned_read_buf);
      return (DB_OUT_OF_MEMORY);
    }
    byte *recovery_buf = static_cast<byte *>(
        ut_align(parallel_dblwr_buf.recovery_buf_unaligned, UNIV_PAGE_SIZE));

    err = os_file_read(read_request, parallel_dblwr_buf.file, recovery_buf, 0,
                       size);
    if (err != DB_SUCCESS) {
      ib::error() << "Failed to read the parallel "
                     "doublewrite buffer";
      buf_parallel_dblwr_close();
      ut_free(unaligned_read_buf);
      ut_free(parallel_dblwr_buf.recovery_buf_unaligned);
      return (DB_ERROR);
    }

    byte zero_page[UNIV_PAGE_SIZE_MAX] = {0};
    for (page = recovery_buf; page < recovery_buf + size;
         page += UNIV_PAGE_SIZE) {
      /* Skip all zero pages */
      const ulint checksum = mach_read_from_4(page + FIL_PAGE_SPACE_OR_CHKSUM);

      if (checksum != 0 || memcmp(page, zero_page, UNIV_PAGE_SIZE) != 0)
        recv_dblwr.add(page);
    }
    buf_parallel_dblwr_close();
  }

  if (reset_space_ids) {
    os_file_flush(file);
  }

  ut_free(unaligned_read_buf);

  return (DB_SUCCESS);
}

/** Delete the parallel doublewrite file, if its path already has been
computed. It is up to the caller to ensure that this called at safe point */
void buf_parallel_dblwr_delete() noexcept {
  if (parallel_dblwr_buf.path)
    os_file_delete_if_exists(innodb_parallel_dblwrite_file_key,
                             parallel_dblwr_buf.path, nullptr);
}

/** Release any unused parallel doublewrite pages and free their underlying
buffer at the end of crash recovery */
void buf_parallel_dblwr_finish_recovery() noexcept {
  recv_sys->dblwr.pages.clear();
  ut_free(parallel_dblwr_buf.recovery_buf_unaligned);
  parallel_dblwr_buf.recovery_buf_unaligned = nullptr;
}

/** Recover a single page
@param[in]	page_no_dblwr	Page number in the doublewrite buffer
@param[in,out]	space		Tablespace instance to write to
@param[in]	page_no		Page number in the tablespace
@param[in]	page		Page data to write */
static void buf_dblwr_recover_page(page_no_t page_no_dblwr, fil_space_t *space,
                                   page_no_t page_no, page_t *page) {
  byte *ptr;
  byte *read_buf;

  ptr = static_cast<byte *>(ut_malloc_nokey(2 * UNIV_PAGE_SIZE));

  read_buf = static_cast<byte *>(ut_align(ptr, UNIV_PAGE_SIZE));

  fil_space_open_if_needed(space);

  if (page_no >= space->size) {
    /* Do not report the warning if the tablespace is
    going to be truncated. */
    if (undo::is_active(space->id)) {
      ib::warn(ER_IB_MSG_104) << "Page " << page_no_dblwr
                              << " in the doublewrite buffer is"
                                 " not within space bounds: page "
                              << page_id_t(space->id, page_no);
    }
  } else {
    const page_size_t page_size(space->flags);
    const page_id_t page_id(space->id, page_no);

    /* We want to ensure that for partial reads the
    unread portion of the page is NUL. */
    memset(read_buf, 0x0, page_size.physical());

    IORequest request;

    request.dblwr_recover();

    /* Read in the actual page from the file */
    dberr_t err = fil_io(request, true, page_id, page_size, 0,
                         page_size.physical(), read_buf, NULL);

    if (err != DB_SUCCESS) {
      ib::warn(ER_IB_MSG_105)
          << "Double write buffer recovery: " << page_id << " read failed with "
          << "error: " << ut_strerr(err);
    }

    /* Check if the page is corrupt */
    BlockReporter block(true, read_buf, page_size,
                        fsp_is_checksum_disabled(space->id));

    if (block.is_corrupted()) {
      ib::info(ER_IB_MSG_106) << "Database page corruption or"
                              << " a failed file read of page " << page_id
                              << ". Trying to recover it from the"
                              << " doublewrite buffer.";

      BlockReporter dblwr_buf_page(true, page, page_size,
                                   fsp_is_checksum_disabled(space->id));

      dberr_t err = DB_SUCCESS;

      if (space->crypt_data ==
          NULL)  // if it was crypt_data encrypted it was already decrypted
        err = os_dblwr_decrypt_page(space, page);

      if (err != DB_SUCCESS || dblwr_buf_page.is_corrupted()) {
        ib::error(ER_IB_MSG_107) << "Dump of the page:";
        buf_page_print(read_buf, page_size, BUF_PAGE_PRINT_NO_CRASH);

        ib::error(ER_IB_MSG_108) << "Dump of corresponding"
                                    " page in doublewrite buffer:";

        buf_page_print(page, page_size, BUF_PAGE_PRINT_NO_CRASH);

        ib::fatal(ER_IB_MSG_109) << "The page in the"
                                    " doublewrite buffer is"
                                    " corrupt. Cannot continue"
                                    " operation. You can try to"
                                    " recover the database with"
                                    " innodb_force_recovery=6";
      }
    } else {
      bool t1 = buf_page_is_zeroes(read_buf, page_size);

      bool t2 = buf_page_is_zeroes(page, page_size);

      BlockReporter reporter = BlockReporter(
          true, page, page_size, fsp_is_checksum_disabled(space->id));

      bool t3 = reporter.is_corrupted();

      if (t1 && !(t2 || t3)) {
        /* Database page contained only
        zeroes, while a valid copy is
        available in dblwr buffer. */

      } else {
        ut_free(ptr);
        return;
      }
    }

    /* Recovered data file pages are written out
    as uncompressed. */

    IORequest write_request(IORequest::WRITE);

    write_request.disable_compression();

    /* Write the good page from the doublewrite
    buffer to the intended position. */

    err = fil_io(write_request, true, page_id, page_size, 0,
                 page_size.physical(), const_cast<byte *>(page), NULL);

    ut_a(err == DB_SUCCESS);

    ib::info(ER_IB_MSG_110)
        << "Recovered page " << page_id << " from the doublewrite buffer.";
  }

  ut_free(ptr);
}

/** Process and remove the double write buffer pages for all tablespaces. */
void buf_dblwr_process() {
  page_no_t page_no_dblwr = 0;
  recv_dblwr_t &dblwr = recv_sys->dblwr;

  /* For cloned database double write pages should be ignored. */
  if (recv_sys->is_cloned_db) {
    dblwr.pages.clear();
  }

  for (auto i = dblwr.pages.begin(); i != dblwr.pages.end();
       ++i, ++page_no_dblwr) {
    byte *page = *i;
    page_no_t page_no = page_get_page_no(page);
    space_id_t space_id = page_get_space_id(page);

    fil_space_t *space = fil_space_get(space_id);

    if (space == nullptr) {
      /* We will have to lazily apply this page
      when we see a MLOG_FILE_OPEN redo record
      during recovery. */

      using Page = recv_dblwr_t::Page;

      dblwr.deferred.push_back(Page(page_no_dblwr, page));
    } else {
      buf_dblwr_recover_page(page_no_dblwr, space, page_no, page);
    }
  }

  fil_flush_file_spaces(to_int(FIL_TYPE_TABLESPACE));

  if (srv_buf_pool_instances == 0) return;  // Support log unit tests

  buf_parallel_dblwr_finish_recovery();

  /* If parallel doublewrite buffer was used, now it's safe to delete and
  re-create it. */
  buf_parallel_dblwr_delete();
  if (buf_parallel_dblwr_create() != DB_SUCCESS)
    ib::fatal() << "Creating the parallel doublewrite buffer failed";
}

/** Recover pages from the double write buffer for a specific tablespace.
The pages that were read from the doublewrite buffer are written to the
tablespace they belong to.
@param[in]	space		Tablespace instance */
void buf_dblwr_recover_pages(fil_space_t *space) {
  recv_dblwr_t &dblwr = recv_sys->dblwr;

  for (auto it = dblwr.deferred.begin(); it != dblwr.deferred.end();
       /* No op */) {
    using Page = recv_dblwr_t::Page;

    Page &page = *it;
    space_id_t space_id = page_get_space_id(page.m_page);

    if (space_id == space->id) {
      page_no_t page_no;

      page_no = page_get_page_no(page.m_page);

      buf_dblwr_recover_page(0, space, page_no, page.m_page);

      page.close();

      it = dblwr.deferred.erase(it);
    } else {
      ++it;
    }
  }

  fil_flush_file_spaces(to_int(FIL_TYPE_TABLESPACE));
}

/** Frees doublewrite buffer. */
void buf_dblwr_free(void) {
  /* Free the double write data structures. */
  ut_ad(buf_dblwr->s_reserved == 0);

  os_event_destroy(buf_dblwr->s_event);
  ut_free(buf_dblwr->write_buf_unaligned);
  buf_dblwr->write_buf_unaligned = NULL;

  ut_free(buf_dblwr->buf_block_arr);
  buf_dblwr->buf_block_arr = NULL;

  ut_free(buf_dblwr->in_use);
  buf_dblwr->in_use = NULL;

  mutex_free(&buf_dblwr->mutex);
  ut_free(buf_dblwr);
  buf_dblwr = NULL;
}

/** Updates the doublewrite buffer when an IO request is completed. */
void buf_dblwr_update(
    const buf_page_t *bpage, /*!< in: buffer block descriptor */
    buf_flush_t flush_type)  /*!< in: flush type */
{
  if (!srv_use_doublewrite_buf || buf_dblwr == NULL ||
      fsp_is_system_temporary(bpage->id.space())) {
    return;
  }

  ut_ad(!srv_read_only_mode);

  switch (flush_type) {
    case BUF_FLUSH_LIST:
    case BUF_FLUSH_LRU: {
      const ulint i = buf_parallel_dblwr_partition(bpage, flush_type);
      struct parallel_dblwr_shard_t *const dblwr_shard =
          &parallel_dblwr_buf.shard[i];

      ut_ad(!os_event_is_set(dblwr_shard->batch_completed));

      if (dblwr_shard->batch_size.fetch_sub(1, std::memory_order_relaxed) == 1)
        /* The last page from the doublewrite batch. */
        os_event_set(dblwr_shard->batch_completed);
    } break;
    case BUF_FLUSH_SINGLE_PAGE: {
      const ulint size = 2 * TRX_SYS_DOUBLEWRITE_BLOCK_SIZE;
      ulint i;
      mutex_enter(&buf_dblwr->mutex);
      for (i = 0; i < size; ++i) {
        if (buf_dblwr->buf_block_arr[i] == bpage) {
          buf_dblwr->s_reserved--;
          buf_dblwr->buf_block_arr[i] = NULL;
          buf_dblwr->in_use[i] = false;
          break;
        }
      }

      /* The block we are looking for must exist as a
      reserved block. */
      ut_a(i < size);
    }
      os_event_set(buf_dblwr->s_event);
      mutex_exit(&buf_dblwr->mutex);
      break;
    case BUF_FLUSH_N_TYPES:
      ut_error;
  }
}

/** Check the LSN values on the page. */
static void buf_dblwr_check_page_lsn(
    const page_t *page) /*!< in: page to check */
{
  if (memcmp(page + (FIL_PAGE_LSN + 4),
             page + (UNIV_PAGE_SIZE - FIL_PAGE_END_LSN_OLD_CHKSUM + 4), 4)) {
    const ulint lsn1 = mach_read_from_4(page + FIL_PAGE_LSN + 4);
    const ulint lsn2 = mach_read_from_4(page + UNIV_PAGE_SIZE -
                                        FIL_PAGE_END_LSN_OLD_CHKSUM + 4);

    ib::error(ER_IB_MSG_111) << "The page to be written seems corrupt!"
                                " The low 4 bytes of LSN fields do not match"
                                " ("
                             << lsn1 << " != " << lsn2
                             << ")!"
                                " Noticed in the buffer pool.";
  }
}

/** Asserts when a corrupt block is find during writing out data to the
 disk. */
static void buf_dblwr_assert_on_corrupt_block(
    const buf_block_t *block) /*!< in: block to check */
{
  buf_page_print(block->frame, univ_page_size, BUF_PAGE_PRINT_NO_CRASH);

  ib::fatal(ER_IB_MSG_112)
      << "Apparent corruption of an index page " << block->page.id
      << " to be written to data file. We intentionally crash"
         " the server to prevent corrupt data from ending up in"
         " data files.";
}

/** Check the LSN values on the page with which this block is associated.
 Also validate the page if the option is set. */
static void buf_dblwr_check_block(
    const buf_block_t *block) /*!< in: block to check */
{
  ut_ad(buf_block_get_state(block) == BUF_BLOCK_FILE_PAGE);

  switch (fil_page_get_type(block->frame)) {
    case FIL_PAGE_INDEX:
    case FIL_PAGE_RTREE:
    case FIL_PAGE_SDI:
      if (page_is_comp(block->frame)) {
        if (page_simple_validate_new(block->frame)) {
          return;
        }
      } else if (page_simple_validate_old(block->frame)) {
        return;
      }
      /* While it is possible that this is not an index page
      but just happens to have wrongly set FIL_PAGE_TYPE,
      such pages should never be modified to without also
      adjusting the page type during page allocation or
      buf_flush_init_for_writing() or fil_page_reset_type(). */
      break;
    case FIL_PAGE_TYPE_FSP_HDR:
    case FIL_PAGE_IBUF_BITMAP:
    case FIL_PAGE_TYPE_UNKNOWN:
      /* Do not complain again, we already reset this field. */
    case FIL_PAGE_UNDO_LOG:
    case FIL_PAGE_INODE:
    case FIL_PAGE_IBUF_FREE_LIST:
    case FIL_PAGE_TYPE_SYS:
    case FIL_PAGE_TYPE_TRX_SYS:
    case FIL_PAGE_TYPE_XDES:
    case FIL_PAGE_TYPE_BLOB:
    case FIL_PAGE_TYPE_ZBLOB:
    case FIL_PAGE_TYPE_ZBLOB2:
    case FIL_PAGE_SDI_BLOB:
    case FIL_PAGE_SDI_ZBLOB:
    case FIL_PAGE_TYPE_LOB_INDEX:
    case FIL_PAGE_TYPE_LOB_DATA:
    case FIL_PAGE_TYPE_LOB_FIRST:
    case FIL_PAGE_TYPE_ZLOB_FIRST:
    case FIL_PAGE_TYPE_ZLOB_DATA:
    case FIL_PAGE_TYPE_ZLOB_INDEX:
    case FIL_PAGE_TYPE_ZLOB_FRAG:
    case FIL_PAGE_TYPE_ZLOB_FRAG_ENTRY:
    case FIL_PAGE_TYPE_RSEG_ARRAY:
      /* TODO: validate also non-index pages */
      return;
    case FIL_PAGE_TYPE_ALLOCATED:
      /* empty pages could be flushed by encryption threads
         and scrubbing */
      return;
  }

  buf_dblwr_assert_on_corrupt_block(block);
}

/** Writes a page that has already been written to the doublewrite buffer
 to the datafile. It is the job of the caller to sync the datafile. */
static void buf_dblwr_write_block_to_datafile(
    const buf_page_t *bpage, /*!< in: page to write */
    bool sync)               /*!< in: true if sync IO
                             is requested */
{
  ut_a(buf_page_in_file(bpage));

  ulint type = IORequest::WRITE;

  if (sync) {
    type |= IORequest::DO_NOT_WAKE;
  }

  dberr_t err;
  IORequest request(type);

  if (bpage->zip.data != NULL) {
    ut_ad(bpage->size.is_compressed());

    err =
        fil_io(request, sync, bpage->id, bpage->size, 0, bpage->size.physical(),
               (void *)bpage->zip.data, (void *)bpage);

    ut_a(err == DB_SUCCESS);

  } else {
    ut_ad(!bpage->size.is_compressed());

    /* Our IO API is common for both reads and writes and is
    therefore geared towards a non-const parameter. */

    buf_block_t *block =
        reinterpret_cast<buf_block_t *>(const_cast<buf_page_t *>(bpage));

    ut_a(buf_block_get_state(block) == BUF_BLOCK_FILE_PAGE);
    buf_dblwr_check_page_lsn(block->frame);

    err = fil_io(request, sync, bpage->id, bpage->size, 0,
                 bpage->size.physical(), block->frame, block);

    ut_a(err == DB_SUCCESS);
  }
}

/** Encrypt a page in doublewerite buffer shard. The page is
encrypted using its tablespace key.
@param[in]	block		the buffer pool block for the page
@param[in,out]	dblwr_page	in: unencrypted page
out: encrypted page (if tablespace is
encrypted */
static void buf_dblwr_encrypt_page(const buf_block_t *block,
                                   page_t *dblwr_page) {
  const auto space_id = block->page.id.space();
  fil_space_t *space = fil_space_acquire_silent(space_id);

  if (space == nullptr) {
    /* Tablespace dropped */
    return;
  }

  byte *encrypted_buf = static_cast<byte *>(ut_zalloc_nokey(UNIV_PAGE_SIZE));
  ut_a(encrypted_buf != nullptr);

  const page_size_t page_size(space->flags);
  const bool success =
      os_dblwr_encrypt_page(space, dblwr_page, encrypted_buf, UNIV_PAGE_SIZE);

  if (success) {
    memcpy(dblwr_page, encrypted_buf, page_size.physical());
  }

  ut_free(encrypted_buf);

  fil_space_release(space);
}

/* Disable encryption of Page 0 of any tablespace or if it is system
tablespace, do not encrypt pages upto TRX_SYS_PAGE_NO (including).
TRX_SYS_PAGE should be not encrypted because dblwr buffer is found
from this page
@param[in]	block	buffer block
@return true if encryption should be disabled for the block, else false */
static bool buf_dblwr_disable_encryption(const buf_block_t &block) noexcept {
  return (block.page.id.page_no() == 0 ||
          (block.page.id.space() == TRX_SYS_SPACE &&
           block.page.id.page_no() <= TRX_SYS_PAGE_NO));
}

/** Flushes possible buffered writes from the specified partition of
the doublewrite memory buffer to disk, and also wakes up the aio
thread if simulated aio is used. It is very important to call this
function after a batch of writes has been posted, and also when we may
have to wait for a page latch! Otherwise a deadlock of threads can
occur.
@param[in]	dblwr_partition	doublewrite partition */
void buf_dblwr_flush_buffered_writes(ulint dblwr_partition) noexcept {
  ulint len;
  byte *write_buf;

  ut_ad(parallel_dblwr_buf.recovery_buf_unaligned == nullptr);

  if (!srv_use_doublewrite_buf || buf_dblwr == nullptr) {
    /* Sync the writes to the disk. */
    buf_dblwr_sync_datafiles();
    return;
  }

  ut_ad(!srv_read_only_mode);

  struct parallel_dblwr_shard_t *const dblwr_shard =
      &parallel_dblwr_buf.shard[dblwr_partition];

  /* Write first to doublewrite buffer blocks. We use synchronous
  aio and thus know that file write has been completed when the
  control returns. */

  if (dblwr_shard->first_free == 0) {
    /* Wake possible simulated aio thread as there could be
    system temporary tablespace pages active for flushing.
    Note: system temporary tablespace pages are not scheduled
    for doublewrite. */
    os_aio_simulated_wake_handler_threads();

    return;
  }

  write_buf = dblwr_shard->write_buf;

  const bool encrypt_parallel_dblwr = srv_parallel_dblwr_encrypt;

  for (ulint len2 = 0, i = 0; i < dblwr_shard->first_free;
       len2 += UNIV_PAGE_SIZE, i++) {
    const buf_block_t *block;

    block = reinterpret_cast<buf_block_t *>(dblwr_shard->buf_block_arr[i]);

    page_t *dblwr_page = write_buf + len2;

    if (buf_block_get_state(block) != BUF_BLOCK_FILE_PAGE ||
        block->page.zip.data) {
      /* No simple validate for compressed
      pages exists. */
      continue;
    }

    /* Check that the actual page in the buffer pool is
    not corrupt and the LSN values are sane. */
    buf_dblwr_check_block(block);

    /* Check that the page as written to the doublewrite
    buffer has sane LSN values. */
    buf_dblwr_check_page_lsn(dblwr_page);

    // it can be already encrypted by encryption threads
    FilSpace space(TRX_SYS_SPACE);
    if (encrypt_parallel_dblwr && space()->crypt_data == nullptr &&
        !buf_dblwr_disable_encryption(*block)) {
      buf_dblwr_encrypt_page(block, dblwr_page);
    }
  }

  len = dblwr_shard->first_free * UNIV_PAGE_SIZE;

  /* Find our part of the doublewrite buffer */
  const os_offset_t file_pos =
      dblwr_partition * srv_doublewrite_batch_size * UNIV_PAGE_SIZE;
  IORequest io_req(IORequest::WRITE | IORequest::NO_COMPRESSION);

#ifdef UNIV_DEBUG
  /* The file size must not increase */
  const os_offset_t desired_size = srv_doublewrite_batch_size * UNIV_PAGE_SIZE *
                                   buf_parallel_dblwr_shard_num();
  const os_offset_t actual_size = os_file_get_size(parallel_dblwr_buf.file);
  ut_ad(desired_size == actual_size);
  ut_ad(file_pos + len <= actual_size);
  /* We must not touch neighboring buffers */
  ut_ad(file_pos + len <=
        (dblwr_partition + 1) * srv_doublewrite_batch_size * UNIV_PAGE_SIZE);
#endif

  const auto err =
      os_file_write(io_req, parallel_dblwr_buf.path, parallel_dblwr_buf.file,
                    write_buf, file_pos, len);
  if (UNIV_UNLIKELY(err != DB_SUCCESS))
    ib::fatal(ER_PARALLEL_DOUBLEWRITE_WRITE_ERROR);

  ut_ad(dblwr_shard->first_free <= srv_doublewrite_batch_size);

  /* increment the doublewrite flushed pages counter */
  srv_stats.dblwr_pages_written.add(dblwr_shard->first_free);
  srv_stats.dblwr_writes.inc();

  if (parallel_dblwr_buf.needs_flush) os_file_flush(parallel_dblwr_buf.file);

  /* We know that the writes have been flushed to disk now
  and in recovery we will find them in the doublewrite buffer
  blocks. Next do the writes to the intended positions. */

  dblwr_shard->batch_size.store(dblwr_shard->first_free,
                                std::memory_order_release);

  for (ulint i = 0; i < dblwr_shard->first_free; i++)
    buf_dblwr_write_block_to_datafile(dblwr_shard->buf_block_arr[i], false);

  /* Wake possible simulated aio thread to actually post the
  writes to the operating system. We don't flush the files
  at this point. We leave it to the IO helper thread to flush
  datafiles when the whole batch has been processed. */
  os_aio_simulated_wake_handler_threads();

  os_event_wait(dblwr_shard->batch_completed);
  os_event_reset(dblwr_shard->batch_completed);

#ifdef UNIV_DEBUG
  ut_ad(dblwr_shard->batch_size.load(std::memory_order_acquire) == 0);
#endif
  dblwr_shard->first_free = 0;

  /* This will finish the batch. Sync data files to the disk. */
  fil_flush_file_spaces(FIL_TYPE_TABLESPACE);
}

/** Posts a buffer page for writing. If the doublewrite memory buffer
is full, calls buf_dblwr_flush_buffered_writes and waits for for free
space to appear.
@param[in]	bpage	buffer block to write
@param[in]	flush_type	BUF_FLUSH_LRU or BUF_FLUSH_LIST */
void buf_dblwr_add_to_batch(buf_page_t *bpage,
                            buf_flush_t flush_type) noexcept {
  ut_ad(flush_type == BUF_FLUSH_LRU || flush_type == BUF_FLUSH_LIST);
  ut_a(buf_page_in_file(bpage));
  ut_ad(!mutex_own(&buf_pool_from_bpage(bpage)->LRU_list_mutex));

  const auto dblwr_partition = buf_parallel_dblwr_partition(bpage, flush_type);
  struct parallel_dblwr_shard_t *dblwr_shard =
      &parallel_dblwr_buf.shard[dblwr_partition];

try_again:
  ut_a(dblwr_shard->first_free <= srv_doublewrite_batch_size);
  ut_ad(!os_event_is_set(dblwr_shard->batch_completed));

  if (dblwr_shard->first_free == srv_doublewrite_batch_size) {
    buf_dblwr_flush_buffered_writes(dblwr_partition);
    goto try_again;
  }

  byte *p = dblwr_shard->write_buf +
            univ_page_size.physical() * dblwr_shard->first_free;

  if (bpage->size.is_compressed()) {
    UNIV_MEM_ASSERT_RW(bpage->zip.data, bpage->size.physical());
    /* Copy the compressed page and clear the rest. */

    memcpy(p, bpage->zip.data, bpage->size.physical());

    memset(p + bpage->size.physical(), 0x0,
           univ_page_size.physical() - bpage->size.physical());
  } else {
    ut_a(buf_page_get_state(bpage) == BUF_BLOCK_FILE_PAGE);

    UNIV_MEM_ASSERT_RW(((buf_block_t *)bpage)->frame, bpage->size.logical());

    memcpy(p, ((buf_block_t *)bpage)->frame, bpage->size.logical());
  }

  dblwr_shard->buf_block_arr[dblwr_shard->first_free++] = bpage;

  ut_ad(!os_event_is_set(dblwr_shard->batch_completed));
  ut_ad(dblwr_shard->first_free <= srv_doublewrite_batch_size);
}

/** Writes a page to the doublewrite buffer on disk, sync it, then write
 the page to the datafile and sync the datafile. This function is used
 for single page flushes. If all the buffers allocated for single page
 flushes in the doublewrite buffer are in use we wait here for one to
 become free. We are guaranteed that a slot will become free because any
 thread that is using a slot must also release the slot before leaving
 this function. */
void buf_dblwr_write_single_page(
    buf_page_t *bpage, /*!< in: buffer block to write */
    bool sync)         /*!< in: true if sync IO requested */
{
  page_no_t i;
  dberr_t err;
  page_no_t size;
  page_no_t offset;

  ut_a(buf_page_in_file(bpage));
  ut_a(srv_use_doublewrite_buf);
  ut_a(buf_dblwr != NULL);

  size = 2 * TRX_SYS_DOUBLEWRITE_BLOCK_SIZE;

  if (buf_page_get_state(bpage) == BUF_BLOCK_FILE_PAGE) {
    /* Check that the actual page in the buffer pool is
    not corrupt and the LSN values are sane. */
    buf_dblwr_check_block((buf_block_t *)bpage);

    /* Check that the page as written to the doublewrite
    buffer has sane LSN values. */
    if (!bpage->zip.data) {
      buf_dblwr_check_page_lsn(((buf_block_t *)bpage)->frame);
    }
  }

retry:
  mutex_enter(&buf_dblwr->mutex);
  if (buf_dblwr->s_reserved == size) {
    /* All slots are reserved. */
    int64_t sig_count = os_event_reset(buf_dblwr->s_event);
    mutex_exit(&buf_dblwr->mutex);
    os_event_wait_low(buf_dblwr->s_event, sig_count);

    goto retry;
  }

  for (i = 0; i < size; ++i) {
    if (!buf_dblwr->in_use[i]) {
      break;
    }
  }

  /* We are guaranteed to find a slot. */
  ut_a(i < size);
  buf_dblwr->in_use[i] = true;
  buf_dblwr->s_reserved++;
  buf_dblwr->buf_block_arr[i] = bpage;

  /* increment the doublewrite flushed pages counter */
  srv_stats.dblwr_pages_written.inc();
  srv_stats.dblwr_writes.inc();

  mutex_exit(&buf_dblwr->mutex);

  /* Lets see if we are going to write in the first or second
  block of the doublewrite buffer. */
  if (i < TRX_SYS_DOUBLEWRITE_BLOCK_SIZE) {
    offset = buf_dblwr->block1 + i;
  } else {
    offset = buf_dblwr->block2 + i - TRX_SYS_DOUBLEWRITE_BLOCK_SIZE;
  }

  /* We deal with compressed and uncompressed pages a little
  differently here. In case of uncompressed pages we can
  directly write the block to the allocated slot in the
  doublewrite buffer in the system tablespace and then after
  syncing the system table space we can proceed to write the page
  in the datafile.
  In case of compressed page we first do a memcpy of the block
  to the in-memory buffer of doublewrite before proceeding to
  write it. This is so because we want to pad the remaining
  bytes in the doublewrite page with zeros. */

  IORequest write_request(IORequest::WRITE);

  if (buf_dblwr_disable_encryption(*(buf_block_t *)bpage)) {
    write_request.disable_encryption();
  }

  if (bpage->size.is_compressed()) {
    memcpy(buf_dblwr->write_buf + univ_page_size.physical() * i,
           bpage->zip.data, bpage->size.physical());

    memset(buf_dblwr->write_buf + univ_page_size.physical() * i +
               bpage->size.physical(),
           0x0, univ_page_size.physical() - bpage->size.physical());

    err = fil_io(write_request, true, page_id_t(TRX_SYS_SPACE, offset),
                 univ_page_size, 0, univ_page_size.physical(),
                 (void *)(buf_dblwr->write_buf + univ_page_size.physical() * i),
                 NULL);
  } else {
    /* It is a regular page. Write it directly to the
    doublewrite buffer */

    err = fil_io(IORequestWrite, true, page_id_t(TRX_SYS_SPACE, offset),
                 univ_page_size, 0, univ_page_size.physical(),
                 (void *)((buf_block_t *)bpage)->frame, NULL);
  }

  ut_a(err == DB_SUCCESS);

  /* Now flush the doublewrite buffer data to disk */
  fil_flush(TRX_SYS_SPACE);

  /* We know that the write has been flushed to disk now
  and during recovery we will find it in the doublewrite buffer
  blocks. Next do the write to the intended position. */
  buf_dblwr_write_block_to_datafile(bpage, sync);
}

/** Compute the size and path of the parallel doublewrite buffer, create it,
and disable OS caching for it
@return DB_SUCCESS or error code */
static MY_ATTRIBUTE((warn_unused_result)) dberr_t
    buf_parallel_dblwr_file_create(void) noexcept {
  ut_ad(!srv_read_only_mode);
  /* The buffer size is two doublewrite batches (one for LRU, one for
  flush list flusher) per buffer pool instance. */
  const os_offset_t size = srv_doublewrite_batch_size * UNIV_PAGE_SIZE *
                           buf_parallel_dblwr_shard_num();
  ut_a(size <= MAX_DOUBLEWRITE_FILE_SIZE);
  ut_a(size > 0);
  ut_a(size % UNIV_PAGE_SIZE == 0);

  dberr_t err = buf_parallel_dblwr_make_path();
  if (err != DB_SUCCESS) return (err);

  ut_ad(parallel_dblwr_buf.file.is_closed());
  ut_ad(parallel_dblwr_buf.recovery_buf_unaligned == NULL);

  /* Set O_SYNC if innodb_flush_method == O_DSYNC. */
  const ulint o_sync =
      (srv_unix_file_flush_method == SRV_UNIX_O_DSYNC) ? OS_FILE_O_SYNC : 0;

  bool success;
  parallel_dblwr_buf.file = os_file_create_simple(
      innodb_parallel_dblwrite_file_key, parallel_dblwr_buf.path,
      OS_FILE_CREATE | o_sync, OS_FILE_READ_WRITE, false, &success);
  if (!success) {
    if (os_file_get_last_error(false) == OS_FILE_ALREADY_EXISTS) {
      ib::error() << "A parallel doublewrite file " << parallel_dblwr_buf.path
                  << " found on startup.";
    }
    return (DB_ERROR);
  }

  const bool o_direct_set = os_file_set_nocache(
      parallel_dblwr_buf.file, parallel_dblwr_buf.path, "create", false);
  switch (srv_unix_file_flush_method) {
    case SRV_UNIX_NOSYNC:
    case SRV_UNIX_O_DSYNC:
    case SRV_UNIX_O_DIRECT_NO_FSYNC:
      parallel_dblwr_buf.needs_flush = !o_direct_set;
      break;
    case SRV_UNIX_FSYNC:
    case SRV_UNIX_LITTLESYNC:
    case SRV_UNIX_O_DIRECT:
      parallel_dblwr_buf.needs_flush = true;
      break;
  }

  success = os_file_set_size(parallel_dblwr_buf.path, parallel_dblwr_buf.file,
                             0, size, srv_read_only_mode, true);
  if (!success) {
    buf_parallel_dblwr_free(true);
    return (DB_ERROR);
  }
  ut_ad(os_file_get_size(parallel_dblwr_buf.file) == size);

  ib::info() << "Created parallel doublewrite buffer at "
             << parallel_dblwr_buf.path << ", size "
             << os_file_get_size(parallel_dblwr_buf.file) << " bytes";

  return (DB_SUCCESS);
}

/** Initialize parallel doublewrite subsystem: create its data structure and
the disk file.
@return DB_SUCCESS or error code */
dberr_t buf_parallel_dblwr_create(void) noexcept {
  if (!parallel_dblwr_buf.file.is_closed() || srv_read_only_mode) {
    ut_ad(parallel_dblwr_buf.recovery_buf_unaligned == nullptr);
    return (DB_SUCCESS);
  }

  memset(parallel_dblwr_buf.shard, 0, sizeof(parallel_dblwr_buf.shard));

  dberr_t err = buf_parallel_dblwr_file_create();
  if (err != DB_SUCCESS) {
    return (err);
  }

  for (ulint i = 0; i < buf_parallel_dblwr_shard_num(); i++) {
    struct parallel_dblwr_shard_t *dblwr_shard = &parallel_dblwr_buf.shard[i];

    dblwr_shard->write_buf_unaligned = static_cast<byte *>(
        ut_malloc((1 + srv_doublewrite_batch_size) * UNIV_PAGE_SIZE,
                  mem_key_parallel_doublewrite));
    if (!dblwr_shard->write_buf_unaligned) {
      buf_parallel_dblwr_free(true);
      return (DB_OUT_OF_MEMORY);
    }
    dblwr_shard->write_buf = static_cast<byte *>(
        ut_align(dblwr_shard->write_buf_unaligned, UNIV_PAGE_SIZE));
    dblwr_shard->buf_block_arr = static_cast<buf_page_t **>(
        ut_zalloc(srv_doublewrite_batch_size * sizeof(void *),
                  mem_key_parallel_doublewrite));
    if (!dblwr_shard->buf_block_arr) {
      buf_parallel_dblwr_free(true);
      return (DB_OUT_OF_MEMORY);
    }

    dblwr_shard->batch_completed =
        os_event_create("parallel_dblwr_batch_completed");
    os_event_reset(dblwr_shard->batch_completed);
  }

  return (DB_SUCCESS);
}

/** Cleanup parallel doublewrite memory structures and optionally close and
delete the doublewrite buffer file too.
@param	delete_file	whether to close and delete the buffer file too  */
void buf_parallel_dblwr_free(bool delete_file) noexcept {
  for (ulint i = 0; i < buf_parallel_dblwr_shard_num(); i++) {
    struct parallel_dblwr_shard_t *dblwr_shard = &parallel_dblwr_buf.shard[i];

    if (dblwr_shard->write_buf_unaligned && dblwr_shard->buf_block_arr) {
      os_event_destroy(dblwr_shard->batch_completed);
    }

    ut_free(dblwr_shard->write_buf_unaligned);
    ut_free(dblwr_shard->buf_block_arr);
  }

  if (delete_file) {
    buf_parallel_dblwr_close();
    buf_parallel_dblwr_delete();
  }

  ut_free(parallel_dblwr_buf.path);
  parallel_dblwr_buf.path = nullptr;
}

/** The parallel doublewrite buffer */
parallel_dblwr_t parallel_dblwr_buf;

/** Constructor
@param[in]	no	Doublewrite page number
@param[in]	page	Page read from no */
recv_dblwr_t::Page::Page(page_no_t no, const byte *page) : m_no(no) {
  m_ptr = static_cast<byte *>(ut_malloc_nokey(UNIV_PAGE_SIZE * 2));
  m_page = static_cast<byte *>(ut_align(m_ptr, UNIV_PAGE_SIZE));

  ut_a(m_ptr != nullptr);
  ut_a(m_page != nullptr);

  memcpy(m_page, page, UNIV_PAGE_SIZE);
}
