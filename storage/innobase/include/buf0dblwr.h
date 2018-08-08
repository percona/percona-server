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

/** @file include/buf0dblwr.h
 Doublewrite buffer module

 Created 2011/12/19 Inaam Rana
 *******************************************************/

#ifndef buf0dblwr_h
#define buf0dblwr_h

#include <atomic>

#include "buf0types.h"
#include "log0log.h"
#include "log0recv.h"
#include "univ.i"
#include "ut0byte.h"

/** Maximum doublewrite batch size. This cannot be set higher than 127, the
legacy innodb_doublewrite_batch_size maximum value, without decoupling the
legacy buffer sizing in the system tablespace from it. */
static const constexpr auto MAX_DOUBLEWRITE_BATCH_SIZE = 127;

/** Doublewrite system */
extern buf_dblwr_t *buf_dblwr;
/** Set to TRUE when the doublewrite buffer is being created */
extern ibool buf_dblwr_being_created;

/** Creates the doublewrite buffer to a new InnoDB installation. The header of
 the doublewrite buffer is placed on the trx system header page.
 @return true if successful, false if not. */
MY_ATTRIBUTE((warn_unused_result))
bool buf_dblwr_create(void);

/** At a database startup initializes the doublewrite buffer memory structure if
 we already have a doublewrite buffer created in the data files. If we are
 upgrading to an InnoDB version which supports multiple tablespaces, then this
 function performs the necessary update operations. If we are in a crash
 recovery, this function loads the pages from double write buffer into memory.
 @return DB_SUCCESS or error code */
MY_ATTRIBUTE((warn_unused_result))
dberr_t buf_dblwr_init_or_load_pages(pfs_os_file_t file, const char *path);

/** Process and remove the double write buffer pages for all tablespaces. */
void buf_dblwr_process(void);

/** frees doublewrite buffer. */
void buf_dblwr_free(void);
/** Updates the doublewrite buffer when an IO request is completed. */
void buf_dblwr_update(
    const buf_page_t *bpage, /*!< in: buffer block descriptor */
    buf_flush_t flush_type); /*!< in: flush type */
/** Determines if a page number is located inside the doublewrite buffer.
 @return true if the location is inside the two blocks of the
 doublewrite buffer */
ibool buf_dblwr_page_inside(page_no_t page_no); /*!< in: page number */

/** Posts a buffer page for writing. If the doublewrite memory buffer
is full, calls buf_dblwr_flush_buffered_writes and waits for for free
space to appear.
@param[in]	bpage	buffer block to write
@param[in]      flush_type BUF_FLUSH_LRU or BUF_FLUSH_LIST */
void buf_dblwr_add_to_batch(buf_page_t *bpage, buf_flush_t flush_type) noexcept;

/** Flush a batch of writes to the datafiles that have already been
 written to the dblwr buffer on disk. */
void buf_dblwr_sync_datafiles();

/** Flushes possible buffered writes from the specified partition of
the doublewrite memory buffer to disk, and also wakes up the aio
thread if simulated aio is used. It is very important to call this
function after a batch of writes has been posted, and also when we may
have to wait for a page latch!  Otherwise a deadlock of threads can
occur.
@param [in] dlbwr_partition doublewrite partition */
void buf_dblwr_flush_buffered_writes(ulint doublewrite_partition) noexcept;
/** Writes a page to the doublewrite buffer on disk, sync it, then write
 the page to the datafile and sync the datafile. This function is used
 for single page flushes. If all the buffers allocated for single page
 flushes in the doublewrite buffer are in use we wait here for one to
 become free. We are guaranteed that a slot will become free because any
 thread that is using a slot must also release the slot before leaving
 this function. */
void buf_dblwr_write_single_page(
    buf_page_t *bpage, /*!< in: buffer block to write */
    bool sync);        /*!< in: true if sync IO requested */

/** Recover pages from the double write buffer for a specific tablespace.
The pages that were read from the doublewrite buffer are written to the
tablespace they belong to.
@param[in]	space		Tablespace instance */
void buf_dblwr_recover_pages(fil_space_t *space);

/** Return the number of partitions in the parallel doublewrite buffer
@return number of parallel doublewrite partitions */
MY_NODISCARD
inline ulint buf_parallel_dblwr_shard_num(void) noexcept {
  static_assert(BUF_FLUSH_LIST == 0 || BUF_FLUSH_LIST == 1,
                "BUF_FLUSH_LIST must be 0 or 1 for shard indexing");
  static_assert(BUF_FLUSH_LRU == 0 || BUF_FLUSH_LRU == 1,
                "BUF_FLUSH_LRU must be 0 or 1 for shard indexing");
  return 2 * srv_buf_pool_instances;
}

/** Return the doublewrite partition number for a given buffer pool and flush
type.
@return the doublewrite partition number */
MY_NODISCARD
inline ulint buf_parallel_dblwr_partition(ulint instance_no,
                                          buf_flush_t flush_type) noexcept {
  ut_ad(flush_type == BUF_FLUSH_LIST || flush_type == BUF_FLUSH_LRU);
  ut_ad(instance_no < srv_buf_pool_instances);
  ulint result = instance_no * 2 + flush_type;
  ut_ad(result < buf_parallel_dblwr_shard_num());
  return (result);
}

/** Return the doublewrite partition number for a given buffer page and flush
type.
@return the doublewrite partition number */
MY_NODISCARD
inline ulint buf_parallel_dblwr_partition(const buf_page_t *bpage,
                                          buf_flush_t flush_type) noexcept {
  return (buf_parallel_dblwr_partition(bpage->buf_pool_index, flush_type));
}

/** Return the doublewrite partition number for a given buffer pool and flush
type.
@return the doublewrite partition number */
MY_NODISCARD
inline ulint buf_parallel_dblwr_partition(const buf_pool_t *buf_pool,
                                          buf_flush_t flush_type) noexcept {
  return (buf_parallel_dblwr_partition(buf_pool->instance_no, flush_type));
}

/** Initialize parallel doublewrite subsystem: create its data structure and
the disk file.
@return DB_SUCCESS or error code */
MY_NODISCARD
dberr_t buf_parallel_dblwr_create(void) noexcept;

/** Delete the parallel doublewrite file, if its path already has been
computed. It is up to the caller to ensure that this called at safe point */
void buf_parallel_dblwr_delete(void) noexcept;

/** Cleanup parallel doublewrite memory structures and optionally close and
delete the doublewrite buffer file too.
@param	delete_file	whether to close and delete the buffer file too  */
void buf_parallel_dblwr_free(bool delete_file) noexcept;

/** Release any unused parallel doublewrite pages and free their underlying
buffer at the end of crash recovery */
void buf_parallel_dblwr_finish_recovery(void) noexcept;

/** A single parallel doublewrite partition data structure */
struct parallel_dblwr_shard_t {
  /** First free position in write_buf measured in units of
  UNIV_PAGE_SIZE */
  ulint first_free;
  /** Number of pages posted to I/O in this doublewrite batch */
  std::atomic<ulint> batch_size;
  /** Raw heap pointer for write_buf */
  byte *write_buf_unaligned;
  /** Write buffer used in writing to the doublewrite buffer, aligned
  on UNIV_PAGE_SIZE */
  byte *write_buf;
  /** Array to store pointers to the buffer blocks which have been cached
  to write_buf */
  buf_page_t **buf_block_arr;
  /** I/O for a doublewrite batch completion event */
  os_event_t batch_completed;
};

/** Maximum possible number of doublewrite partitions */
static const constexpr auto MAX_DBLWR_SHARDS = MAX_BUFFER_POOLS * 2;

/** Parallel doublewrite buffer data structure */
class parallel_dblwr_t {
 public:
  /** Parallel doublewrite buffer file handle */
  pfs_os_file_t file;
  /** Whether the doublewrite buffer file needs flushing after each
  write */
  bool needs_flush;
  /** Path to the parallel doublewrite buffer */
  char *path;
  /** Individual parallel doublewrite partitions */
  parallel_dblwr_shard_t shard[MAX_DBLWR_SHARDS];
  /** Buffer for reading in parallel doublewrite buffer pages
  during crash recovery */
  byte *recovery_buf_unaligned;

  /** Default constructor for the parallel doublewrite instance */
  parallel_dblwr_t(void) : path(nullptr), recovery_buf_unaligned(nullptr) {
    file.set_closed();
  }
};

/** The parallel doublewrite buffer */
extern parallel_dblwr_t parallel_dblwr_buf;

/** Doublewrite control struct */
struct buf_dblwr_t {
  ib_mutex_t mutex;           /*!< mutex protecting write_buf */
  page_no_t block1;           /*!< the page number of the first
                              doublewrite block (64 pages) */
  page_no_t block2;           /*!< page number of the second block */
  ulint s_reserved;           /*!< number of slots currently
                           reserved for single page flushes. */
  os_event_t s_event;         /*!< event where threads wait for a
                              single page flush slot. */
  bool *in_use;               /*!< flag used to indicate if a slot is
                              in use. Only used for single page
                              flushes. */
  byte *write_buf;            /*!< write buffer used in writing to the
                            doublewrite buffer, aligned to an
                            address divisible by UNIV_PAGE_SIZE
                            (which is required by Windows aio) */
  byte *write_buf_unaligned;  /*!< pointer to write_buf,
                  but unaligned */
  buf_page_t **buf_block_arr; /*!< array to store pointers to
                        the buffer blocks which have been
                        cached to write_buf */
};

#endif
