/*****************************************************************************

Copyright (c) 2011-2012 Percona Inc. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
Street, Fifth Floor, Boston, MA 02110-1301, USA

*****************************************************************************/

/** @file log/log0online.cc
Online database log parsing for changed page tracking */

#include "log0online.h"

#include <dirent.h>
#include <sys/types.h>
#include <algorithm>
#include <array>

#include "my_dbug.h"
#include "my_dir.h"

#include "log0recv.h"
#include "mach0data.h"
#include "mtr0log.h"
#include "srv0srv.h"
#include "srv0start.h"
#include "trx0sys.h"
#include "ut0rbt.h"

#ifdef __WIN__
/* error LNK2001: unresolved external symbol _debug_sync_C_callback_ptr */
#define DEBUG_SYNC_C(dummy) ((void)0)
#else
#include "m_string.h" /* for my_sys.h */
#include "my_sys.h"   /* DEBUG_SYNC_C */
#endif

/** For the given minilog record type determine if the record has (space; page)
associated with it.
@param[in] type the minilog record type
@return true if the record has (space; page) in it */
static constexpr bool log_online_rec_has_page(mlog_id_t type) noexcept {
  static_assert(MLOG_BIGGEST_TYPE == 65,
                "New MTR types must be reviewed for page presence");
  return type != MLOG_MULTI_REC_END && type != MLOG_DUMMY_RECORD &&
         type != MLOG_COMP_PAGE_CREATE_SDI && type != MLOG_PAGE_CREATE_SDI &&
         type != MLOG_TABLE_DYNAMIC_META
#ifdef UNIV_LOG_LSN_DEBUG
         && type != MLOG_LSN
#endif
      ;
}

/* Mutex protecting log_bmp_sys and log buffers there */
static ib_mutex_t log_bmp_sys_mutex;

/** A redo log byte buffer with associated LSN values */
template <std::size_t CAPACITY>
class log_buffer {
 protected:
  using buffer_type = std::array<byte, CAPACITY>;

 public:
  using size_type = typename buffer_type::size_type;
  using const_iterator = typename buffer_type::const_iterator;

 protected:
  buffer_type buffer;
  static const constexpr auto capacity = CAPACITY;
  size_type current_size{0};
  lsn_t start_lsn{0};
  lsn_t current_lsn{0};
  const_iterator current_ptr{buffer.cbegin()};
  lsn_t limit_lsn{0};

  log_buffer() noexcept {}

#ifdef UNIV_DEBUG
  MY_NODISCARD bool invariants() const noexcept {
    ut_ad(mutex_own(&log_bmp_sys_mutex));
    ut_ad(start_lsn <= current_lsn);
    ut_ad(current_lsn <= limit_lsn);
    ut_ad(static_cast<decltype(start_lsn)>(current_ptr - buffer.cbegin()) <=
          current_size);
    return true;
  }
#endif

 public:
  MY_NODISCARD const_iterator ccurrent() const noexcept {
    ut_ad(mutex_own(&log_bmp_sys_mutex));
    return current_ptr;
  }

  void set_limit(lsn_t limit) {
    ut_ad(mutex_own(&log_bmp_sys_mutex));
    ut_ad(limit >= limit_lsn);
    limit_lsn = limit;
  }

  MY_NODISCARD lsn_t get_current_lsn() const noexcept {
    ut_ad(mutex_own(&log_bmp_sys_mutex));
    return current_lsn;
  }
};

static const constexpr auto FOLLOW_SCAN_SIZE = 4 * UNIV_PAGE_SIZE_MAX;

static_assert(FOLLOW_SCAN_SIZE % OS_FILE_LOG_BLOCK_SIZE == 0,
              "FOLLOW_SCAN_SIZE must be a multiple of OS_FILE_LOG_BLOCK_SIZE");

static const constexpr auto LOG_BLOCK_SIZE_NO_TRL =
    OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE;

static const constexpr auto LOG_BLOCK_BOUNDARY_LSN_PAD =
    LOG_BLOCK_HDR_SIZE + LOG_BLOCK_TRL_SIZE;

/** The buffer for reading log data, filled in by recv_read_log_recs and moved
in chunks to the parse buffer while skipping log block headers and trailers. */
class log_read_buffer final : public log_buffer<FOLLOW_SCAN_SIZE> {
 public:
  void read(lsn_t read_start, lsn_t read_end) noexcept;

  MY_NODISCARD bool is_data_available() const noexcept {
    ut_ad(mutex_own(&log_bmp_sys_mutex));
    const auto result = ccurrent() < cend() && get_current_lsn() < limit_lsn;
    if (result) {
      ut_ad(invariants());
    }
    return result;
  }

  /** Check the log block checksum.
  @return true if the log block checksum is OK, false otherwise.  */
  MY_NODISCARD bool is_current_block_valid() const noexcept;

  MY_NODISCARD uint32_t get_current_block_data_len() const noexcept {
    ut_ad(is_current_block_valid());
    return log_block_get_data_len(ccurrent());
  }

  MY_NODISCARD const_iterator cend() const noexcept {
    return &buffer[current_size];
  }

  void advance() noexcept {
    ut_ad(mutex_own(&log_bmp_sys_mutex));
    ut_ad(invariants());
    current_lsn += OS_FILE_LOG_BLOCK_SIZE;
    ut_ad(current_lsn - start_lsn <= current_size);
    current_ptr += OS_FILE_LOG_BLOCK_SIZE;
  }

#ifdef UNIV_DEBUG
 private:
  MY_NODISCARD bool invariants() const noexcept {
    ut_ad(log_buffer::invariants());
    // Until C++17 overaligned allocation support verify the alignment manually
    ut_ad(reinterpret_cast<uintptr_t>(&buffer[0]) %
              INNODB_LOG_WRITE_AHEAD_SIZE_MAX ==
          0);
    ut_ad((ccurrent() - buffer.cbegin()) % OS_FILE_LOG_BLOCK_SIZE == 0);
    ut_ad((buffer.cend() - ccurrent()) % OS_FILE_LOG_BLOCK_SIZE == 0);
    return true;
  }
#endif
};

void log_read_buffer::read(lsn_t read_start, lsn_t read_end) noexcept {
  ut_ad(mutex_own(&log_bmp_sys_mutex));
  ut_ad(read_start < limit_lsn);
  ut_ad(get_current_lsn() >= read_start || get_current_lsn() == 0);
  current_size = read_end - read_start;
  ut_ad(current_size <= capacity);
  recv_read_log_seg(*log_sys, &buffer[0], read_start, read_end, true);
  current_lsn = start_lsn = read_start;
  current_ptr = buffer.cbegin();
}

bool log_read_buffer::is_current_block_valid() const noexcept {
  ut_ad(mutex_own(&log_bmp_sys_mutex));
  ut_ad(invariants());
  ut_ad(cend() - ccurrent() >= OS_FILE_LOG_BLOCK_SIZE);
  const auto checksum_is_ok = log_block_checksum_is_ok(ccurrent());

  if (!checksum_is_ok) {
    // We are reading empty log blocks in some cases (such as
    // tracking log on server startup with log resizing). Such
    // blocks are benign, silently accept them.
    static const byte zero_block[OS_FILE_LOG_BLOCK_SIZE] = {0};
    if (!memcmp(ccurrent(), zero_block, OS_FILE_LOG_BLOCK_SIZE)) {
      return true;
    }

    const auto no = log_block_get_hdr_no(ccurrent());
    const auto expected_no = log_block_convert_lsn_to_no(current_lsn);
    ib::fatal() << "Log block checksum mismatch: LSN " << current_lsn
                << ", expected " << log_block_get_checksum(ccurrent()) << ", "
                << "calculated checksum " << log_block_calc_checksum(ccurrent())
                << ", "
                << "stored log block n:o " << no << ", "
                << "expected log block n:o " << expected_no;
  }

  return checksum_is_ok;
}

/** The redo log parse buffer */
class log_parse_buffer final : public log_buffer<RECV_PARSING_BUF_SIZE> {
 private:
  using iterator = typename buffer_type::iterator;
  enum class parse_result { OK, INCOMPLETE, PAST_END };

  parse_result parse_status{parse_result::OK};
  iterator end_ptr{buffer.begin()};
  lsn_t end_lsn{0};

#ifdef UNIV_DEBUG
  MY_NODISCARD size_type size() const noexcept {
    return end_ptr - buffer.cbegin();
  }

  MY_NODISCARD bool invariants() const noexcept {
    ut_ad(log_buffer::invariants());
    ut_ad(current_ptr <= end_ptr);
    ut_ad(buffer.begin() + current_size == end_ptr);
    ut_ad(current_lsn <= end_lsn);
    if (current_lsn == limit_lsn) ut_ad(parse_status == parse_result::OK);
    return true;
  }
#endif

  MY_NODISCARD bool advance(ulint delta) noexcept;

  MY_NODISCARD const_iterator cend() const noexcept {
    ut_ad(mutex_own(&log_bmp_sys_mutex));
    return end_ptr;
  }

  void move_unprocessed_to_front(parse_result new_parse_status) noexcept;

  MY_NODISCARD size_type unparsed_size() const noexcept {
    return end_ptr - current_ptr;
  }

 public:
  MY_NODISCARD bool parse_next_record(mlog_id_t *type, space_id_t *space,
                                      page_no_t *page_no) noexcept;

  MY_NODISCARD bool can_parse_current_data() const noexcept {
    return parse_status == parse_result::OK && ccurrent() != cend() &&
           current_lsn < limit_lsn;
  }

  MY_NODISCARD bool parsed_past_checkpoint() const noexcept {
    return parse_status == parse_result::PAST_END;
  }

  void reset_parse_status() { parse_status = parse_result::OK; }

  void add(log_read_buffer &from, log_buffer::size_type data_len,
           log_buffer::size_type skip_len) noexcept;

  MY_NODISCARD lsn_t get_end_lsn() const noexcept { return end_lsn; }

#ifdef UNIV_DEBUG
  MY_NODISCARD bool buffer_used_up() const noexcept {
    ut_ad(invariants());
    switch (parse_status) {
      case parse_result::OK:
        if (unparsed_size() == 0) {
          ut_ad(current_lsn == end_lsn);
        } else {
          ut_ad(start_lsn == current_lsn);
          ut_ad(current_lsn < end_lsn);
        }
        break;
      case parse_result::INCOMPLETE:
      case parse_result::PAST_END:
        ut_ad(start_lsn == current_lsn);
        ut_ad(current_lsn < end_lsn);
        ut_ad(unparsed_size() > 0);
        break;
    }
    return true;
  }
#endif
};

bool log_parse_buffer::advance(ulint delta) noexcept {
  ut_ad(mutex_own(&log_bmp_sys_mutex));
  ut_ad(invariants());
  ut_ad(delta > 0);
  ut_ad(!parsed_past_checkpoint());
  ut_ad(parse_status == parse_result::OK);
  ut_ad(unparsed_size() >= delta);
  const auto new_current_lsn = recv_calc_lsn_on_data_add(current_lsn, delta);
  ut_ad(new_current_lsn % OS_FILE_LOG_BLOCK_SIZE >= LOG_BLOCK_HDR_SIZE);
  ut_ad(new_current_lsn % OS_FILE_LOG_BLOCK_SIZE < LOG_BLOCK_SIZE_NO_TRL);
  ut_ad(new_current_lsn - current_lsn >= delta);
  ut_ad(new_current_lsn <= end_lsn);
  if (new_current_lsn > limit_lsn) return false;
  current_lsn = new_current_lsn;
  current_ptr += delta;
  return true;
}

void log_parse_buffer::add(log_read_buffer &from,
                           log_buffer::size_type data_len,
                           log_buffer::size_type skip_len) noexcept {
  ut_ad(mutex_own(&log_bmp_sys_mutex));
  ut_ad(from.is_current_block_valid());
  ut_ad(!parsed_past_checkpoint());
  // Do not skip into middle of the header
  ut_ad(!skip_len || skip_len >= LOG_BLOCK_HDR_SIZE);
  // Do not call this if the whole block must be skipped
  ut_ad(skip_len < LOG_BLOCK_SIZE_NO_TRL);
  ut_ad(data_len > LOG_BLOCK_HDR_SIZE);
  ut_ad(data_len <= OS_FILE_LOG_BLOCK_SIZE);

  const auto start_offset = skip_len ? skip_len : LOG_BLOCK_HDR_SIZE;
  const auto end_offset =
      (data_len == OS_FILE_LOG_BLOCK_SIZE) ? LOG_BLOCK_SIZE_NO_TRL : data_len;
  ut_ad(end_offset > start_offset);
  const auto actual_data_len = end_offset - start_offset;
  const auto copy_start_lsn = from.get_current_lsn() + start_offset;
  const auto copy_end_lsn =
      copy_start_lsn + actual_data_len +
      ((data_len == OS_FILE_LOG_BLOCK_SIZE) ? LOG_BLOCK_BOUNDARY_LSN_PAD : 0);

#ifdef UNIV_DEBUG
  ut_ad(end_lsn < copy_end_lsn);
  ut_ad(current_size + actual_data_len <= capacity);
  if (parse_status == parse_result::OK) {
    ut_ad(copy_start_lsn >= current_lsn + unparsed_size() || current_lsn == 0);
    ut_ad(copy_start_lsn <=
              current_lsn + unparsed_size() + LOG_BLOCK_BOUNDARY_LSN_PAD ||
          current_lsn == 0);
  } else {
    ut_ad(size() == unparsed_size());
    ut_ad(copy_start_lsn >= current_lsn + size() || current_lsn == 0);
  }
  ut_ad(current_lsn != 0 || ccurrent() == buffer.cbegin());
  ut_ad(start_lsn != 0 || ccurrent() == buffer.cbegin());
#endif
  memcpy(end_ptr, from.ccurrent() + start_offset, actual_data_len);
  end_ptr += actual_data_len;
  current_size += actual_data_len;
  end_lsn = copy_end_lsn;
  if (current_lsn == 0) {
    start_lsn = current_lsn = copy_start_lsn;
  }
  ut_ad(invariants());
  ut_ad(current_lsn + actual_data_len <= from.get_current_lsn() + data_len);
  from.advance();
  reset_parse_status();
}

bool log_parse_buffer::parse_next_record(mlog_id_t *type, space_id_t *space,
                                         page_no_t *page_no) noexcept {
  ut_ad(mutex_own(&log_bmp_sys_mutex));
  ut_ad(invariants());
  ut_ad(can_parse_current_data());
  byte *body;
  /* recv_sys is not initialized, so on corrupt log we will SIGSEGV. But the log
  of a live database should not be corrupt. */
  const auto len = recv_parse_log_rec(type, const_cast<byte *>(&*ccurrent()),
                                      const_cast<byte *>(&*cend()), space,
                                      page_no, false, &body);
  if (len > 0) {
    if (advance(len)) {
      ut_ad(len >= 3 || !log_online_rec_has_page(*type));
      if (!can_parse_current_data())
        move_unprocessed_to_front(parse_result::OK);
      return true;
    }
    move_unprocessed_to_front(parse_result::PAST_END);
    return false;
  }
  move_unprocessed_to_front(parse_result::INCOMPLETE);
  return false;
}

void log_parse_buffer::move_unprocessed_to_front(
    parse_result new_parse_status) noexcept {
  ut_ad(mutex_own(&log_bmp_sys_mutex));
  parse_status = new_parse_status;
  const auto new_size = unparsed_size();
  ut_ad(new_size <= current_size);
  ut_ad(new_size <= capacity);
  memmove(&buffer[0], ccurrent(), new_size);
  current_size = new_size;
  start_lsn = current_lsn;
  current_ptr = buffer.cbegin();
  end_ptr = buffer.begin() + current_size;
  ut_ad(invariants());
  ut_ad(buffer_used_up());
}

#ifdef UNIV_PFS_MUTEX
/** Key to register log_bmp_sys_mutex with PFS */
mysql_pfs_key_t log_bmp_sys_mutex_key;
#endif /* UNIV_PFS_MUTEX */

/** On server startup with empty database the first LSN of actual log records
will be this. */
static const constexpr auto MIN_TRACKED_LSN =
    LOG_START_LSN + LOG_BLOCK_HDR_SIZE;

/** Log parsing and bitmap output data structure */
struct log_bitmap_struct {
  log_read_buffer read_buf;
  log_parse_buffer parse_buf;
  char bmp_file_home[FN_REFLEN];
  /*!< directory for bitmap files */
  log_online_bitmap_file_t out;  /*!< The current bitmap file */
  ulint out_seq_num;             /*!< the bitmap file sequence number */
  lsn_t start_lsn{0};            /*!< the LSN of the next unparsed
                                       record and the start of the next LSN
                                       interval to be parsed.  */
  lsn_t end_lsn{0};              /*!< the end of the LSN interval to be
                                         parsed, equal to the next checkpoint
                                         LSN at the time of parse */
  ib_rbt_t *modified_pages;      /*!< the current modified page set,
                                  organized as the RB-tree with the keys
                                  of (space, 4KB-block-start-page-id)
                                  pairs */
  ib_rbt_node_t *page_free_list; /*!< Singly-linked list of freed nodes
                                  of modified_pages tree for later
                                  reuse.  Nodes are linked through
                                  ib_rbt_node_t.left as this field has
                                  both the correct type and the tree does
                                  not mind its overwrite during
                                  rbt_next() tree traversal. */

  MY_NODISCARD lsn_t has_parse_data_to() const noexcept {
    const auto parse_buf_end_lsn = parse_buf.get_end_lsn();
    return parse_buf_end_lsn ? parse_buf_end_lsn : start_lsn;
  }

  void follow_up_to(lsn_t end_lsn_) noexcept {
    ut_ad(mutex_own(&log_bmp_sys_mutex));
    ut_ad(end_lsn_ >= end_lsn);
    end_lsn = end_lsn_;
    read_buf.set_limit(end_lsn);
    parse_buf.set_limit(end_lsn);
  }

  void start_at(lsn_t start_lsn_) noexcept {
    ut_ad(start_lsn == 0);
    ut_ad(end_lsn == 0);
    ut_ad(start_lsn_ >= MIN_TRACKED_LSN);
    start_lsn = start_lsn_;
    log_sys->tracked_lsn.store(start_lsn_);
    end_lsn = start_lsn;
  }
};

static void *log_bmp_sys_unaligned;

/** The log parsing and bitmap output struct instance */
static struct log_bitmap_struct *log_bmp_sys;

/** A read-only MetadataRecover instance to support log record parsing */
MetadataRecover *log_online_metadata_recover = nullptr;

/** File name stem for bitmap files. */
static const constexpr auto bmp_file_name_stem = "ib_modified_log_";

/** File name template for bitmap files.  The 1st format tag is a directory
name, the 2nd tag is the stem, the 3rd tag is a file sequence number, the 4th
tag is the start LSN for the file. */
static const constexpr auto bmp_file_name_template = "%s%s%lu_" LSN_PF ".xdb";

/* Tests if num bit of bitmap is set */
#define IS_BIT_SET(bitmap, num) \
  (*((bitmap) + ((num) >> 3)) & (1UL << ((num)&7UL)))

/** The bitmap file block size in bytes.  All writes will be multiples of this.
 */
static const constexpr auto MODIFIED_PAGE_BLOCK_SIZE = 4096;

/** Offsets in a file bitmap block */
static const constexpr auto MODIFIED_PAGE_IS_LAST_BLOCK =
    0; /* 1 if last block in the current
                                        write, 0 otherwise. */
static const constexpr auto MODIFIED_PAGE_START_LSN =
    4; /* The starting tracked LSN of this and
                                        other blocks in the same write */
static const constexpr auto MODIFIED_PAGE_END_LSN =
    12; /* One past the last tracked LSN of this and
                                        other blocks in the same write */
static const constexpr auto MODIFIED_PAGE_SPACE_ID =
    20; /* The space ID of tracked pages in
                                        this block */
static const constexpr auto MODIFIED_PAGE_1ST_PAGE_ID =
    24; /* The page ID of the first tracked
                                        page in this block */
static const constexpr auto MODIFIED_PAGE_BLOCK_UNUSED_1
    MY_ATTRIBUTE((unused)) = 28; /* Unused in order to align the start
                of bitmap at 8 byte boundary */
static const constexpr auto MODIFIED_PAGE_BLOCK_BITMAP =
    32; /* Start of the bitmap itself */
static const constexpr auto MODIFIED_PAGE_BLOCK_UNUSED_2 =
    MODIFIED_PAGE_BLOCK_SIZE - 8;
/* Unused in order to align the end of
                                        bitmap at 8 byte boundary */
static const constexpr auto MODIFIED_PAGE_BLOCK_CHECKSUM =
    MODIFIED_PAGE_BLOCK_SIZE - 4;
/* The checksum of the current block */

/** Length of the bitmap data in a block in bytes */
static const constexpr auto MODIFIED_PAGE_BLOCK_BITMAP_LEN =
    MODIFIED_PAGE_BLOCK_UNUSED_2 - MODIFIED_PAGE_BLOCK_BITMAP;

/** Length of the bitmap data in a block in page ids */
static const constexpr auto MODIFIED_PAGE_BLOCK_ID_COUNT =
    MODIFIED_PAGE_BLOCK_BITMAP_LEN * 8;

/** Provide a comparisson function for the RB-tree tree (space,
block_start_page) pairs.  Actual implementation does not matter as
long as the ordering is full.
@return -1 if p1 < p2, 0 if p1 == p2, 1 if p1 > p2 */
static int log_online_compare_bmp_keys(
    const void *p1, /*!<in: 1st key to compare */
    const void *p2) /*!<in: 2nd key to compare */
    noexcept {
  const byte *const k1 = (const byte *)p1;
  const byte *const k2 = (const byte *)p2;

  const space_id_t k1_space = mach_read_from_4(k1 + MODIFIED_PAGE_SPACE_ID);
  const space_id_t k2_space = mach_read_from_4(k2 + MODIFIED_PAGE_SPACE_ID);
  if (k1_space == k2_space) {
    const page_no_t k1_start_page =
        mach_read_from_4(k1 + MODIFIED_PAGE_1ST_PAGE_ID);
    const page_no_t k2_start_page =
        mach_read_from_4(k2 + MODIFIED_PAGE_1ST_PAGE_ID);
    return k1_start_page < k2_start_page
               ? -1
               : k1_start_page > k2_start_page ? 1 : 0;
  }
  return k1_space < k2_space ? -1 : 1;
}

/** Set a bit for tracked page in the bitmap. Expand the bitmap tree as
necessary.
@param[in] space   log record space id
@param[in] page_no log record page id */
static void log_online_set_page_bit(space_id_t space, page_no_t page_no) {
  ut_ad(mutex_own(&log_bmp_sys_mutex));

  const ulint block_start_page =
      page_no / MODIFIED_PAGE_BLOCK_ID_COUNT * MODIFIED_PAGE_BLOCK_ID_COUNT;
  const ulint block_pos =
      block_start_page ? (page_no % block_start_page / 8) : (page_no / 8);
  const uint bit_pos = page_no % 8;

  byte search_page[MODIFIED_PAGE_BLOCK_SIZE];
  mach_write_to_4(search_page + MODIFIED_PAGE_SPACE_ID, space);
  mach_write_to_4(search_page + MODIFIED_PAGE_1ST_PAGE_ID, block_start_page);

  byte *page_ptr;
  ib_rbt_bound_t tree_search_pos;
  if (!rbt_search(log_bmp_sys->modified_pages, &tree_search_pos, search_page)) {
    page_ptr = rbt_value(byte, tree_search_pos.last);
  } else {
    ib_rbt_node_t *new_node;

    if (log_bmp_sys->page_free_list) {
      new_node = log_bmp_sys->page_free_list;
      log_bmp_sys->page_free_list = new_node->left;
    } else {
      new_node = static_cast<ib_rbt_node_t *>(
          ut_malloc(SIZEOF_NODE(log_bmp_sys->modified_pages),
                    mem_key_log_online_modified_pages));
    }
    memset(new_node, 0, SIZEOF_NODE(log_bmp_sys->modified_pages));

    page_ptr = rbt_value(byte, new_node);
    mach_write_to_4(page_ptr + MODIFIED_PAGE_SPACE_ID, space);
    mach_write_to_4(page_ptr + MODIFIED_PAGE_1ST_PAGE_ID, block_start_page);

    rbt_add_preallocated_node(log_bmp_sys->modified_pages, &tree_search_pos,
                              new_node);
  }
  page_ptr[MODIFIED_PAGE_BLOCK_BITMAP + block_pos] |= (1U << bit_pos);
}

/** Calculate a bitmap block checksum.  Algorithm borrowed from
log_block_calc_checksum.
@return checksum */
UNIV_INLINE
ulint log_online_calc_checksum(const byte *block) /*!<in: bitmap block */
    noexcept {
  ulint sum = 1;
  ulint sh = 0;

  for (ulint i = 0; i < MODIFIED_PAGE_BLOCK_CHECKSUM; i++) {
    const ulint b = block[i];
    sum &= 0x7FFFFFFFUL;
    sum += b;
    sum += b << sh;
    sh++;
    if (sh > 24) {
      sh = 0;
    }
  }

  return sum;
}

/** Read one bitmap data page and check it for corruption.

@return true if page read OK, false if I/O error */
static bool log_online_read_bitmap_page(
    log_online_bitmap_file_t *bitmap_file, /*!<in/out: bitmap
                                                        file */
    byte *page,                            /*!<out: read page.
                                                          Must be at least
                                                          MODIFIED_PAGE_BLOCK_SIZE
                                                          bytes long */
    bool *checksum_ok)                     /*!<out: true if page
                                checksum OK */
    noexcept {
  ut_a(bitmap_file->size >= MODIFIED_PAGE_BLOCK_SIZE);
  ut_a(bitmap_file->offset <= bitmap_file->size - MODIFIED_PAGE_BLOCK_SIZE);
  ut_a(bitmap_file->offset % MODIFIED_PAGE_BLOCK_SIZE == 0);

  IORequest io_request(IORequest::LOG | IORequest::READ |
                       IORequest::NO_ENCRYPTION);
  const bool success =
      os_file_read(io_request, bitmap_file->file, page, bitmap_file->offset,
                   MODIFIED_PAGE_BLOCK_SIZE);

  if (UNIV_UNLIKELY(!success)) {
    /* The following call prints an error message */
    os_file_get_last_error(true);
    ib::warn() << "Failed reading changed page bitmap file \'"
               << bitmap_file->name << "\'";
    return false;
  }

  bitmap_file->offset += MODIFIED_PAGE_BLOCK_SIZE;
  ut_ad(bitmap_file->offset <= bitmap_file->size);

  const ulint checksum = mach_read_from_4(page + MODIFIED_PAGE_BLOCK_CHECKSUM);
  const ulint actual_checksum = log_online_calc_checksum(page);
  *checksum_ok = (checksum == actual_checksum);

  return true;
}

/** Get the last tracked fully LSN from the bitmap file by reading
backwards untile a correct end page is found.  Detects incomplete
writes and corrupted data.  Sets the start output position for the
written bitmap data.

Multiple bitmap files are handled using the following assumptions:
1) Only the last file might be corrupted.  In case where no good data was found
in the last file, assume that the next to last file is OK.  This assumption
does not limit crash recovery capability in any way.
2) If the whole of the last file was corrupted, assume that the start LSN in
its name is correct and use it for (re-)tracking start.

@return the last fully tracked LSN */
static lsn_t log_online_read_last_tracked_lsn(void) noexcept {
  byte page[MODIFIED_PAGE_BLOCK_SIZE];
  bool is_last_page = false;
  bool checksum_ok = false;
  lsn_t result;
  os_offset_t read_offset = log_bmp_sys->out.offset;

  while ((!checksum_ok || !is_last_page) && read_offset > 0) {
    read_offset -= MODIFIED_PAGE_BLOCK_SIZE;
    log_bmp_sys->out.offset = read_offset;

    if (!log_online_read_bitmap_page(&log_bmp_sys->out, page, &checksum_ok)) {
      checksum_ok = false;
      result = 0;
      break;
    }

    if (checksum_ok) {
      is_last_page = mach_read_from_4(page + MODIFIED_PAGE_IS_LAST_BLOCK);
    } else {
      ib::warn() << "Corruption detected in \'" << log_bmp_sys->out.name
                 << "\' at offset " << read_offset;
    }
  };

  result = (checksum_ok && is_last_page)
               ? mach_read_from_8(page + MODIFIED_PAGE_END_LSN)
               : 0;

  /* Truncate the output file to discard the corrupted bitmap data, if
  any */
  if (!os_file_set_eof_at(log_bmp_sys->out.file, log_bmp_sys->out.offset)) {
    ib::warn() << "Failed truncating changed page bitmap file \'"
               << log_bmp_sys->out.name << "\' to " << log_bmp_sys->out.offset
               << " bytes";
    result = 0;
  }
  return result;
}

/** Check if missing, if any, LSN interval can be read and tracked using the
current LSN value, the LSN value where the tracking stopped, and the log
capacity.

@return true if the missing interval can be tracked or if there's no missing
data.  */
MY_ATTRIBUTE((warn_unused_result))
static bool log_online_can_track_missing(
    lsn_t last_tracked_lsn,   /*!<in: last tracked LSN */
    lsn_t tracking_start_lsn) /*!<in:	current LSN */
    noexcept {
  /* last_tracked_lsn might be < MIN_TRACKED_LSN in the case of empty bitmap
file, handle this too. */
  last_tracked_lsn = std::max(last_tracked_lsn, MIN_TRACKED_LSN);

  if ((last_tracked_lsn > tracking_start_lsn) &&
      (last_tracked_lsn % OS_FILE_LOG_BLOCK_SIZE > LOG_BLOCK_HDR_SIZE)) {
    ib::fatal() << "Last tracked LSN " << last_tracked_lsn
                << " is ahead of tracking start LSN " << tracking_start_lsn
                << ".  This can be caused "
                   "by mismatched bitmap files.";
  }

  return (last_tracked_lsn == tracking_start_lsn) ||
         (log_get_lsn(*log_sys) - last_tracked_lsn <=
          log_sys->lsn_real_capacity);
}

/** Diagnose a gap in tracked LSN range on server startup due to crash or
very fast shutdown and try to close it by tracking the data
immediatelly, if possible. */
static void log_online_track_missing_on_startup(
    lsn_t last_tracked_lsn,   /*!<in: last tracked LSN read from the
                                        bitmap file */
    lsn_t tracking_start_lsn) /*!<in: last checkpoint LSN of the
                                        current server startup */
{
  ut_ad(last_tracked_lsn != tracking_start_lsn);
  ut_ad(srv_track_changed_pages);

  ib::info() << "Last tracked LSN in \'" << log_bmp_sys->out.name << "\' is "
             << last_tracked_lsn << ", with the last checkpoint LSN is "
             << tracking_start_lsn << '.';

  /* See if we can fully recover the missing interval */
  if (log_online_can_track_missing(last_tracked_lsn, tracking_start_lsn)) {
    ib::info() << "Reading the log to advance the last tracked LSN.";

    log_bmp_sys->start_at(std::max(last_tracked_lsn, MIN_TRACKED_LSN));
    if (!log_online_follow_redo_log_one_pass()) {
      exit(1);
    }
    ut_ad(log_bmp_sys->end_lsn == tracking_start_lsn);

    ib::info() << "Continuing tracking changed pages from LSN "
               << log_bmp_sys->end_lsn;
  } else {
    ib::warn() << "The age of last tracked LSN exceeds log "
                  "capacity, tracking-based incremental backups will "
                  "work only from the higher LSN!";

    log_bmp_sys->start_at(tracking_start_lsn);

    ib::info() << "Starting tracking changed pages from LSN "
               << log_bmp_sys->end_lsn;
  }
}

/** Format a bitmap output file name to log_bmp_sys->out.name.  */
static void log_online_make_bitmap_name(
    lsn_t start_lsn) /*!< in: the start LSN name part */
    noexcept {
  snprintf(log_bmp_sys->out.name, sizeof(log_bmp_sys->out.name),
           bmp_file_name_template, log_bmp_sys->bmp_file_home,
           bmp_file_name_stem, log_bmp_sys->out_seq_num, start_lsn);
}

/** Check if an old file that has the name of a new bitmap file we are about to
create should be overwritten.  */
static bool log_online_should_overwrite(
    const char *path) /*!< in: path to file */
    noexcept {
  os_file_stat_t file_info;

  /* Currently, it's OK to overwrite 0-sized files only */
  const dberr_t err =
      os_file_get_status(path, &file_info, false, srv_read_only_mode);
  return err == DB_SUCCESS && file_info.type == OS_FILE_TYPE_FILE &&
         file_info.size == 0LL;
}

/** Create a new empty bitmap output file.

@return true if operation succeeded, false if I/O error */
static bool log_online_start_bitmap_file(void) noexcept {
  bool success = true;

  /* Check for an old file that should be deleted first */
  if (log_online_should_overwrite(log_bmp_sys->out.name)) {
    success = os_file_delete_if_exists(innodb_bmp_file_key,
                                       log_bmp_sys->out.name, nullptr);
  }

  if (UNIV_LIKELY(success)) {
    log_bmp_sys->out.file = os_file_create_simple_no_error_handling(
        innodb_bmp_file_key, log_bmp_sys->out.name, OS_FILE_CREATE,
        OS_FILE_READ_WRITE, srv_read_only_mode, &success);
  }
  if (UNIV_UNLIKELY(!success)) {
    /* The following call prints an error message */
    os_file_get_last_error(true);
    ib::error() << "Cannot create \'" << log_bmp_sys->out.name << "\'";
    return false;
  }

  ut_ad(!log_bmp_sys->out.file.is_closed());
  log_bmp_sys->out.offset = 0;
  return true;
}

/** Close the current bitmap output file and create the next one.

@return true if operation succeeded, false if I/O error */
static bool log_online_rotate_bitmap_file(
    lsn_t next_file_start_lsn) /*!<in: the start LSN name
                                        part */
    noexcept {
  if (!log_bmp_sys->out.file.is_closed()) {
    os_file_close(log_bmp_sys->out.file);
    log_bmp_sys->out.file.set_closed();
  }
  log_bmp_sys->out_seq_num++;
  log_online_make_bitmap_name(next_file_start_lsn);
  return log_online_start_bitmap_file();
}

/** Check the name of a given file if it's a changed page bitmap file and
return file sequence and start LSN name components if it is.  If is not,
the values of output parameters are undefined.

@return true if a given file is a changed page bitmap file.  */
static bool log_online_is_bitmap_file(
    const FILEINFO &file_info,    /*!<in: file to
                                  check */
    ulong *bitmap_file_seq_num,   /*!<out: bitmap file
                                  sequence number */
    lsn_t *bitmap_file_start_lsn) /*!<out: bitmap file
                                  start LSN */
    noexcept {
  ut_ad(strlen(file_info.name) < OS_FILE_MAX_PATH);

  char stem[FN_REFLEN];
  return (MY_S_ISREG(file_info.mystat->st_mode) ||
          MY_S_ISLNK(file_info.mystat->st_mode)) &&
         sscanf(file_info.name, "%[a-z_]%lu_" LSN_PF ".xdb", stem,
                bitmap_file_seq_num, bitmap_file_start_lsn) == 3 &&
         !strcmp(stem, bmp_file_name_stem);
}

/** Initialize the constant part of the log tracking subsystem */

void log_online_init(void) noexcept {
  mutex_create(LATCH_ID_LOG_ONLINE, &log_bmp_sys_mutex);
}

/** Initialize the dynamic part of the log tracking subsystem */

void log_online_read_init(void) {
  static_assert(MODIFIED_PAGE_BLOCK_BITMAP % 8 == 0,
                "Bitmap data start in a bitmap block must be 8-byte aligned");
  static_assert(MODIFIED_PAGE_BLOCK_BITMAP_LEN % 8 == 0,
                "Bitmap data end in a bitmap block must be 8-byte aligned");

  ut_ad(srv_track_changed_pages);

  log_online_metadata_recover =
      UT_NEW(MetadataRecover(true), mem_key_log_online_sys);

  log_bmp_sys_unaligned =
      ut_malloc(sizeof(*log_bmp_sys) + INNODB_LOG_WRITE_AHEAD_SIZE_MAX - 1,
                mem_key_log_online_sys);
  log_bmp_sys = new (
      (reinterpret_cast<uintptr_t>(log_bmp_sys_unaligned) %
       INNODB_LOG_WRITE_AHEAD_SIZE_MAX)
          ? ut_align(log_bmp_sys_unaligned, INNODB_LOG_WRITE_AHEAD_SIZE_MAX)
          : log_bmp_sys_unaligned) log_bitmap_struct;

  /* Initialize bitmap file directory from srv_data_home and add a path
  separator if needed.  */
  const size_t srv_data_home_len = strlen(srv_data_home);
  ut_a(srv_data_home_len < FN_REFLEN);
  strcpy(log_bmp_sys->bmp_file_home, srv_data_home);
  if (srv_data_home_len &&
      log_bmp_sys->bmp_file_home[srv_data_home_len - 1] != SRV_PATH_SEPARATOR) {
    ut_a(srv_data_home_len < FN_REFLEN - 1);
    log_bmp_sys->bmp_file_home[srv_data_home_len] = SRV_PATH_SEPARATOR;
    log_bmp_sys->bmp_file_home[srv_data_home_len + 1] = '\0';
  }

  /* Enumerate existing bitmap files to either open the last one to get
  the last tracked LSN either to find that there are none and start
  tracking from scratch.  */
  log_bmp_sys->out.name[0] = '\0';
  log_bmp_sys->out_seq_num = 0;

  auto *const bitmap_dir =
      my_dir(log_bmp_sys->bmp_file_home, MY_WANT_STAT | MY_DONT_SORT | MY_WME);
  if (UNIV_UNLIKELY(!bitmap_dir)) exit(1);
  lsn_t last_file_start_lsn = MIN_TRACKED_LSN;
  for (uint i = 0; i < bitmap_dir->number_off_files; i++) {
    ulong file_seq_num;
    lsn_t file_start_lsn;

    if (!log_online_is_bitmap_file(bitmap_dir->dir_entry[i], &file_seq_num,
                                   &file_start_lsn))
      continue;

    if (file_seq_num > log_bmp_sys->out_seq_num &&
        bitmap_dir->dir_entry[i].mystat->st_size > 0) {
      log_bmp_sys->out_seq_num = file_seq_num;
      last_file_start_lsn = file_start_lsn;
      /* No dir component (log_bmp_sys->bmp_file_home) here, because
that's the cwd */
      strncpy(log_bmp_sys->out.name, bitmap_dir->dir_entry[i].name,
              FN_REFLEN - 1);
      log_bmp_sys->out.name[FN_REFLEN - 1] = '\0';
    }
  }
  my_dirend(bitmap_dir);

  if (!log_bmp_sys->out_seq_num) {
    log_bmp_sys->out_seq_num = 1;
    log_online_make_bitmap_name(0);
  }

  log_bmp_sys->modified_pages =
      rbt_create(MODIFIED_PAGE_BLOCK_SIZE, log_online_compare_bmp_keys);
  log_bmp_sys->page_free_list = nullptr;

  bool success;
  log_bmp_sys->out.file = os_file_create_simple_no_error_handling(
      innodb_bmp_file_key, log_bmp_sys->out.name, OS_FILE_OPEN,
      OS_FILE_READ_WRITE, srv_read_only_mode, &success);

  const auto tracking_start_lsn = log_sys->last_checkpoint_lsn.load();
  if (!success) {
    /* New file, tracking from scratch */
    if (!log_online_start_bitmap_file()) {
      exit(1);
    }
  } else {
    /* Read the last tracked LSN from the last file */
    lsn_t last_tracked_lsn;
    lsn_t file_start_lsn;

    log_bmp_sys->out.size = os_file_get_size(log_bmp_sys->out.file);
    log_bmp_sys->out.offset = log_bmp_sys->out.size;

    if (log_bmp_sys->out.offset % MODIFIED_PAGE_BLOCK_SIZE != 0) {
      ib::warn() << "Truncated block detected in \'" << log_bmp_sys->out.name
                 << "\' at offset " << log_bmp_sys->out.offset;
      log_bmp_sys->out.offset -=
          log_bmp_sys->out.offset % MODIFIED_PAGE_BLOCK_SIZE;
    }

    last_tracked_lsn = log_online_read_last_tracked_lsn();
    /* Do not rotate if we truncated the file to zero length - we
    can just start writing there */
    const bool need_rotate = (last_tracked_lsn != 0);
    if (!last_tracked_lsn) {
      last_tracked_lsn = last_file_start_lsn;
    }

    /* Start a new file.  Choose the LSN value in its name based on
    if we can retrack any missing data. */
    if (log_online_can_track_missing(last_tracked_lsn, tracking_start_lsn)) {
      file_start_lsn = last_tracked_lsn;
    } else {
      file_start_lsn = tracking_start_lsn;
    }

    if (need_rotate && !log_online_rotate_bitmap_file(file_start_lsn)) {
      exit(1);
    }

    if (last_tracked_lsn < tracking_start_lsn) {
      log_online_track_missing_on_startup(last_tracked_lsn, tracking_start_lsn);
      return;
    }

    if (last_tracked_lsn > tracking_start_lsn) {
      ib::warn() << "Last tracked LSN is " << last_tracked_lsn
                 << ", but the last "
                    "checkpoint LSN is "
                 << tracking_start_lsn
                 << ". The tracking-based incremental "
                    "backups will work only from the latter LSN!";
    }
  }

  ib::info() << "Starting tracking changed pages from LSN "
             << tracking_start_lsn;
  log_bmp_sys->start_at(tracking_start_lsn);
}

/** Shut down the dynamic part of the log tracking subsystem */

void log_online_read_shutdown(void) noexcept {
  mutex_enter(&log_bmp_sys_mutex);

  srv_track_changed_pages = false;

  ib_rbt_node_t *free_list_node = log_bmp_sys->page_free_list;

  if (!log_bmp_sys->out.file.is_closed()) {
    os_file_close(log_bmp_sys->out.file);
    log_bmp_sys->out.file.set_closed();
  }

  rbt_free(log_bmp_sys->modified_pages);

  while (free_list_node) {
    ib_rbt_node_t *next = free_list_node->left;
    ut_free(free_list_node);
    free_list_node = next;
  }

  ut_free(log_bmp_sys_unaligned);
  log_bmp_sys = nullptr;
  log_bmp_sys_unaligned = nullptr;

  srv_redo_log_thread_started = false;

  UT_DELETE(log_online_metadata_recover);
  log_online_metadata_recover = nullptr;

  mutex_exit(&log_bmp_sys_mutex);
}

/** Shut down the constant part of the log tracking subsystem */
void log_online_shutdown(void) noexcept { mutex_free(&log_bmp_sys_mutex); }

/** Parse the log data in the parse buffer for the (space, page) pairs and add
them to the modified page set as necessary.  Removes the fully-parsed records
from the buffer.  If an incomplete record is found, moves it to the end of the
buffer. */
static void log_online_parse_redo_log(void) {
  ut_ad(mutex_own(&log_bmp_sys_mutex));

  while (log_bmp_sys->parse_buf.can_parse_current_data()) {
    mlog_id_t type;
    space_id_t space;
    page_no_t page_no;

    const auto rec_parsed =
        log_bmp_sys->parse_buf.parse_next_record(&type, &space, &page_no);
    ut_ad(log_bmp_sys->parse_buf.get_current_lsn() <=
          log_bmp_sys->read_buf.get_current_lsn() + LOG_BLOCK_BOUNDARY_LSN_PAD);
    if (rec_parsed && log_online_rec_has_page(type)) {
      log_online_set_page_bit(space, page_no);
      if (type == MLOG_INDEX_LOAD) {
        const auto space_size = fil_space_get_size(space);
        for (page_no_t i = 0; i < space_size; i++) {
          log_online_set_page_bit(space, i);
        }
      }
    }
  }
  ut_ad(log_bmp_sys->parse_buf.buffer_used_up());
}

/** Parse the log block: first copies the read log data to the parse buffer
while skipping log block header, trailer and already parsed data.  Then it
actually parses the log to add to the modified page bitmap.
@param[in,out] from log read buffer to take data from
@param[in] skip_already_parsed_len how many bytes of log data should be skipped
as they were parsed before */
static void log_online_parse_redo_log_block(
    log_read_buffer &from, size_t skip_already_parsed_len) noexcept {
  ut_ad(mutex_own(&log_bmp_sys_mutex));
  ut_ad(skip_already_parsed_len <= LOG_BLOCK_SIZE_NO_TRL);

  const auto block_data_len = from.get_current_block_data_len();
  ut_ad(block_data_len == 0 || block_data_len >= LOG_BLOCK_HDR_SIZE);
  ut_ad(block_data_len <= OS_FILE_LOG_BLOCK_SIZE);
  if (skip_already_parsed_len == block_data_len || block_data_len == 0) {
    log_bmp_sys->read_buf.advance();
    return;
  }
  ut_ad(skip_already_parsed_len < block_data_len);
  if (skip_already_parsed_len == LOG_BLOCK_SIZE_NO_TRL &&
      block_data_len == OS_FILE_LOG_BLOCK_SIZE) {
    log_bmp_sys->read_buf.advance();
    return;
  }
  if (block_data_len == LOG_BLOCK_HDR_SIZE) {
    log_bmp_sys->read_buf.advance();
    return;
  }

  log_bmp_sys->parse_buf.add(log_bmp_sys->read_buf, block_data_len,
                             skip_already_parsed_len);
  log_online_parse_redo_log();
}

/** Read and parse one redo log chunk and updates the modified page bitmap. */
MY_ATTRIBUTE((warn_unused_result))
static bool log_online_follow_log_seg(
    lsn_t block_start_lsn, /*!< in: the LSN to read from */
    lsn_t block_end_lsn)   /*!< in: the LSN to read to */
{
  ut_ad(mutex_own(&log_bmp_sys_mutex));

  log_bmp_sys->read_buf.read(block_start_lsn, block_end_lsn);

  // Skip complete blocks already in the parse buffer
  while (log_bmp_sys->read_buf.get_current_lsn() + OS_FILE_LOG_BLOCK_SIZE <=
             log_bmp_sys->has_parse_data_to() &&
         log_bmp_sys->read_buf.is_data_available()) {
    log_bmp_sys->read_buf.advance();
  }

  if (!log_bmp_sys->read_buf.is_data_available()) return true;

  if (log_bmp_sys->has_parse_data_to() >
      log_bmp_sys->read_buf.get_current_lsn()) {
    // Seeking to the new read data to start parsing from, next block must be
    // fully or partially new data to parse
    ut_ad(log_bmp_sys->read_buf.get_current_lsn() <
          log_bmp_sys->has_parse_data_to());
    ut_ad(log_bmp_sys->read_buf.get_current_lsn() + OS_FILE_LOG_BLOCK_SIZE >
          log_bmp_sys->has_parse_data_to());

    /* How many bytes of log data should we skip in the current log block. */
    const auto skip_already_in_parse_buf_len =
        static_cast<ulint>(log_bmp_sys->has_parse_data_to() -
                           log_bmp_sys->read_buf.get_current_lsn());
    log_online_parse_redo_log_block(log_bmp_sys->read_buf,
                                    skip_already_in_parse_buf_len);
  }

  while (log_bmp_sys->read_buf.is_data_available() &&
         !log_bmp_sys->parse_buf.parsed_past_checkpoint()) {
    if (!log_bmp_sys->read_buf.is_current_block_valid()) return false;
    log_online_parse_redo_log_block(log_bmp_sys->read_buf, 0);
  }

  return true;
}

/** Read and parse the redo log in FOLLOW_SCAN_SIZE-sized chunks and update the
modified page bitmap. */
MY_ATTRIBUTE((warn_unused_result))
static bool log_online_follow_log(
    lsn_t contiguous_lsn) /*!< in: the LSN of log block start
                                        containing the log_parse_start_lsn */
{
  ut_ad(mutex_own(&log_bmp_sys_mutex));

  lsn_t block_start_lsn = contiguous_lsn;
  lsn_t block_end_lsn;

  do {
    block_end_lsn = block_start_lsn + FOLLOW_SCAN_SIZE;

    if (!log_online_follow_log_seg(block_start_lsn, block_end_lsn))
      return false;

    /* Next parse LSN can become higher than the last read LSN
    only in the case when the read LSN falls right on the block
    boundary, in which case next parse lsn is bumped to the actual
    data LSN on the next (not yet read) block. */
    ut_a(log_bmp_sys->parse_buf.get_current_lsn() <=
         block_end_lsn + LOG_BLOCK_BOUNDARY_LSN_PAD);

    block_start_lsn = block_end_lsn;
  } while (block_end_lsn < log_bmp_sys->end_lsn);

  return true;
}

/** Write, flush one bitmap block to disk and advance the output position if
successful.

@return true if page written OK, false if I/O error */
static bool log_online_write_bitmap_page(
    const byte *block) /*!< in: block to write */
{
  ut_ad(srv_track_changed_pages);
  ut_ad(mutex_own(&log_bmp_sys_mutex));
  ut_ad(!log_bmp_sys->out.file.is_closed());

  /* Simulate a write error */
  DBUG_EXECUTE_IF("bitmap_page_write_error", {
    if (!srv_is_being_started) {
      ib::error()
          << "simulating bitmap write error in log_online_write_bitmap_page";
      return false;
    }
  });

  /* A crash injection site that ensures last checkpoint LSN > last
  tracked LSN, so that LSN tracking for this interval is tested. */
  DBUG_EXECUTE_IF("crash_before_bitmap_write", {
    const space_id_t space_id =
        mach_read_from_4(block + MODIFIED_PAGE_SPACE_ID);
    if (space_id > 0) DBUG_SUICIDE();
  });

  IORequest io_request(IORequest::WRITE | IORequest::NO_COMPRESSION);
  bool success =
      os_file_write(io_request, log_bmp_sys->out.name, log_bmp_sys->out.file,
                    block, log_bmp_sys->out.offset, MODIFIED_PAGE_BLOCK_SIZE);
  if (UNIV_UNLIKELY(!success)) {
    /* The following call prints an error message */
    os_file_get_last_error(true);
    ib::error() << "Failed writing changed page bitmap file \'"
                << log_bmp_sys->out.name << "\'";
    return false;
  }

  success = os_file_flush(log_bmp_sys->out.file);
  if (UNIV_UNLIKELY(!success)) {
    /* The following call prints an error message */
    os_file_get_last_error(true);
    ib::error() << "Failed flushing changed page bitmap file \'"
                << log_bmp_sys->out.name << "\'";
    return false;
  }

  os_file_advise(log_bmp_sys->out.file, log_bmp_sys->out.offset,
                 MODIFIED_PAGE_BLOCK_SIZE, OS_FILE_ADVISE_DONTNEED);

  log_bmp_sys->out.offset += MODIFIED_PAGE_BLOCK_SIZE;
  return true;
}

/** Append the current changed page bitmap to the bitmap file.  Clears the
bitmap tree and recycles its nodes to the free list.

@return true if bitmap written OK, false if I/O error*/
static bool log_online_write_bitmap(void) {
  ut_ad(mutex_own(&log_bmp_sys_mutex));

  if (log_bmp_sys->out.offset >= srv_max_bitmap_file_size) {
    if (!log_online_rotate_bitmap_file(log_bmp_sys->start_lsn)) {
      return false;
    }
  }
  ut_ad(!log_bmp_sys->out.file.is_closed());

  ib_rbt_node_t *bmp_tree_node =
      (ib_rbt_node_t *)rbt_first(log_bmp_sys->modified_pages);
  const ib_rbt_node_t *const last_bmp_tree_node =
      rbt_last(log_bmp_sys->modified_pages);

  bool success = true;

  while (bmp_tree_node) {
    byte *page = rbt_value(byte, bmp_tree_node);

#ifdef UNIV_DEBUG
    const space_id_t space_id = mach_read_from_4(page + MODIFIED_PAGE_SPACE_ID);
#endif

    /* In case of a bitmap page write error keep on looping over
    the tree to reclaim its memory through the free list instead of
    returning immediatelly. */
    if (UNIV_LIKELY(success)) {
      if (bmp_tree_node == last_bmp_tree_node) {
        mach_write_to_4(page + MODIFIED_PAGE_IS_LAST_BLOCK, 1);
      }

      mach_write_to_8(page + MODIFIED_PAGE_START_LSN, log_bmp_sys->start_lsn);
      mach_write_to_8(page + MODIFIED_PAGE_END_LSN,
                      log_bmp_sys->parse_buf.get_current_lsn());
      mach_write_to_4(page + MODIFIED_PAGE_BLOCK_CHECKSUM,
                      log_online_calc_checksum(page));

      success = log_online_write_bitmap_page(page);
    }

    bmp_tree_node->left = log_bmp_sys->page_free_list;
    log_bmp_sys->page_free_list = bmp_tree_node;

    bmp_tree_node =
        (ib_rbt_node_t *)rbt_next(log_bmp_sys->modified_pages, bmp_tree_node);

    DBUG_EXECUTE_IF("bitmap_page_2_write_error",
                    if (bmp_tree_node && fsp_is_ibd_tablespace(space_id)) {
                      DBUG_SET("+d,bitmap_page_write_error");
                      DBUG_SET("-d,bitmap_page_2_write_error");
                    });
  }

  rbt_reset(log_bmp_sys->modified_pages);
  return success;
}

static void log_online_parse_complete_recs_past_previous_checkpoint() noexcept {
  ut_ad(mutex_own(&log_bmp_sys_mutex));

  if (!log_bmp_sys->parse_buf.parsed_past_checkpoint()) return;
  log_bmp_sys->parse_buf.reset_parse_status();

  log_online_parse_redo_log();
}

/** Read and parse the redo log up to last checkpoint LSN to build the changed
page bitmap which is then written to disk.

@return true if log tracking succeeded, false if bitmap write I/O error */
bool log_online_follow_redo_log_one_pass(void) {
  ut_ad(!srv_read_only_mode);

  if (!srv_track_changed_pages) return true;

  mutex_enter(&log_bmp_sys_mutex);

  if (!srv_track_changed_pages) {
    mutex_exit(&log_bmp_sys_mutex);
    return true;
  }

  ut_ad(!log_bmp_sys->out.file.is_closed());

  /* Grab the LSN of the last checkpoint, we will parse up to it */
  /* Parse up to the LSN of the last checkpoint */
  log_bmp_sys->follow_up_to(log_get_checkpoint_lsn(*log_sys));

  if (log_bmp_sys->end_lsn == log_bmp_sys->start_lsn) {
    mutex_exit(&log_bmp_sys_mutex);
    return true;
  }

  log_online_parse_complete_recs_past_previous_checkpoint();

  bool result = log_online_follow_log(
      ut_uint64_align_down(log_bmp_sys->start_lsn, OS_FILE_LOG_BLOCK_SIZE));

  if (result) {
    result = log_online_write_bitmap();
    log_bmp_sys->start_lsn = log_bmp_sys->end_lsn;
    log_sys->tracked_lsn.store(log_bmp_sys->parse_buf.get_current_lsn());
  }

  mutex_exit(&log_bmp_sys_mutex);
  return result;
}

/** Read and parse redo log for thr FLUSH CHANGED_PAGE_BITMAPS command.
Make sure that the checkpoint LSN measured at the beginning of the command
is tracked.

@return true if log tracking succeeded, false if bitmap write I/O error */
bool log_online_follow_redo_log(void) {
  ut_ad(!srv_read_only_mode);

  const auto last_checkpoint_lsn = log_get_checkpoint_lsn(*log_sys);
  bool result = true;

  DEBUG_SYNC_C("log_online_follow_redo_log");

  while (result && srv_track_changed_pages &&
         last_checkpoint_lsn > log_sys->tracked_lsn.load()) {
    result = log_online_follow_redo_log_one_pass();
  }

  return result;
}

/** Diagnose a bitmap file range setup failure and free the
partially-initialized bitmap file range.  */
UNIV_COLD
static void log_online_diagnose_inconsistent_dir(
    log_online_bitmap_file_range_t *bitmap_files) /*!<in/out: bitmap file
                                                        range */
    noexcept {
  ib::warn() << "Inconsistent bitmap file directory for a "
                "INFORMATION_SCHEMA.INNODB_CHANGED_PAGES query";
  ut_free(bitmap_files->files);
  bitmap_files->files = nullptr;
}

/** List the bitmap files in srv_data_home and setup their range that contains
the specified LSN interval.  This range, if non-empty, will start with a file
that has the greatest LSN equal to or less than the start LSN and will include
all the files up to the one with the greatest LSN less than the end LSN.
Caller must free bitmap_files->files when done if bitmap_files set to non-NULL
and this function returned true.  Field bitmap_files->count might be set to a
larger value than the actual count of the files, and space for the unused array
slots will be allocated but cleared to zeroes.

@return true if succeeded
*/
static bool log_online_setup_bitmap_file_range(
    log_online_bitmap_file_range_t *bitmap_files, /*!<in/out: bitmap file
                                                        range */
    lsn_t range_start,                            /*!<in: start LSN */
    lsn_t range_end)                              /*!<in: end LSN */
{
  ut_ad(range_end >= range_start);

  bitmap_files->count = 0;
  bitmap_files->files = nullptr;

  /* 1st pass: size the info array */

  auto *bitmap_dir = my_dir(srv_data_home, MY_DONT_SORT | MY_WANT_STAT);
  if (UNIV_UNLIKELY(!bitmap_dir)) {
    ib::error() << "Failed to open bitmap directory \'" << srv_data_home
                << "\'";
    return false;
  }

  ulong first_file_seq_num = ULONG_MAX;
  ulong last_file_seq_num = 0;
  lsn_t first_file_start_lsn = LSN_MAX;

  for (uint i = 0; i < bitmap_dir->number_off_files; i++) {
    ulong file_seq_num;
    lsn_t file_start_lsn;

    if (!log_online_is_bitmap_file(bitmap_dir->dir_entry[i], &file_seq_num,
                                   &file_start_lsn) ||
        file_start_lsn >= range_end)
      continue;

    if (file_seq_num > last_file_seq_num) last_file_seq_num = file_seq_num;

    if (file_start_lsn >= range_start) {
      // A file that falls into the range
      if (file_start_lsn < first_file_start_lsn) {
        if (file_seq_num >= first_file_seq_num) {
          log_online_diagnose_inconsistent_dir(bitmap_files);
          my_dirend(bitmap_dir);
          return false;
        }
        first_file_start_lsn = file_start_lsn;
        ut_ad(file_seq_num < first_file_seq_num);
        first_file_seq_num = file_seq_num;
      }
    } else if (first_file_start_lsn > range_start) {
      // A file that does not fully fall into range but we haven't found the
      // file containing the range start yet
      ut_ad(file_start_lsn < range_start);
      if (file_start_lsn > first_file_start_lsn ||
          first_file_start_lsn == LSN_MAX) {
        // A file whose starting LSN is less than range start LSN, but larger
        // than the LSN of a previously considered file to start the range
        first_file_start_lsn = file_start_lsn;
        first_file_seq_num = file_seq_num;
      }
    }
    if ((file_start_lsn == first_file_start_lsn) &&
        (file_seq_num < first_file_seq_num)) {
      // An empty file with lower sequence number
      first_file_seq_num = file_seq_num;
    }
  }
  my_dirend(bitmap_dir);

  if (first_file_seq_num == ULONG_MAX && last_file_seq_num == 0) {
    bitmap_files->count = 0;
    return true;
  }

  bitmap_files->count = last_file_seq_num - first_file_seq_num + 1;

  DEBUG_SYNC_C("setup_bitmap_range_middle");

  /* 2nd pass: get the file names in the file_seq_num order */

  bitmap_dir = my_dir(srv_data_home, MY_DONT_SORT | MY_WANT_STAT);
  if (UNIV_UNLIKELY(!bitmap_dir)) {
    ib::error() << "Failed to open bitmap directory \'" << srv_data_home
                << "\'";
    return false;
  }

  bitmap_files->files =
      static_cast<log_online_bitmap_file_range_struct::files_t *>(
          ut_zalloc(bitmap_files->count * sizeof(bitmap_files->files[0]),
                    mem_key_log_online_iterator_files));

  for (uint i = 0; i < bitmap_dir->number_off_files; i++) {
    ulong file_seq_num;
    lsn_t file_start_lsn;
    if (!log_online_is_bitmap_file(bitmap_dir->dir_entry[i], &file_seq_num,
                                   &file_start_lsn) ||
        file_start_lsn >= range_end || file_start_lsn < first_file_start_lsn)
      continue;

    const size_t array_pos = file_seq_num - first_file_seq_num;
    if (UNIV_UNLIKELY(array_pos >= bitmap_files->count)) {
      log_online_diagnose_inconsistent_dir(bitmap_files);
      my_dirend(bitmap_dir);
      return false;
    }

    if (file_seq_num > bitmap_files->files[array_pos].seq_num) {
      bitmap_files->files[array_pos].seq_num = file_seq_num;
      strncpy(bitmap_files->files[array_pos].name,
              bitmap_dir->dir_entry[i].name, FN_REFLEN);
      bitmap_files->files[array_pos].name[FN_REFLEN - 1] = '\0';
      bitmap_files->files[array_pos].start_lsn = file_start_lsn;
    }
  }
  my_dirend(bitmap_dir);

  if (!bitmap_files->files[0].seq_num ||
      bitmap_files->files[0].seq_num != first_file_seq_num) {
    log_online_diagnose_inconsistent_dir(bitmap_files);
    return false;
  }

  {
    size_t i;
    for (i = 1; i < bitmap_files->count; i++) {
      if (!bitmap_files->files[i].seq_num) {
        break;
      }
      if ((bitmap_files->files[i].seq_num <=
           bitmap_files->files[i - 1].seq_num) ||
          (bitmap_files->files[i].start_lsn <
           bitmap_files->files[i - 1].start_lsn)) {
        log_online_diagnose_inconsistent_dir(bitmap_files);
        return false;
      }
    }
  }

  return true;
}

/** Open a bitmap file for reading.

@return true if opened successfully */
static bool log_online_open_bitmap_file_read_only(
    const char *name,                      /*!<in: bitmap file
                                                           name without directory,
                                                           which is assumed to be
                                                           srv_data_home */
    log_online_bitmap_file_t *bitmap_file) /*!<out: opened bitmap
                                                        file */
{
  ut_ad(name[0] != '\0');

  const size_t srv_data_home_len = strlen(srv_data_home);
  if (srv_data_home_len &&
      srv_data_home[srv_data_home_len - 1] != SRV_PATH_SEPARATOR) {
    snprintf(bitmap_file->name, sizeof(bitmap_file->name), "%s%c%s",
             srv_data_home, SRV_PATH_SEPARATOR, name);
  } else {
    snprintf(bitmap_file->name, sizeof(bitmap_file->name), "%s%s",
             srv_data_home, name);
  }
  bool success = false;
  bitmap_file->file = os_file_create_simple_no_error_handling(
      innodb_bmp_file_key, bitmap_file->name, OS_FILE_OPEN, OS_FILE_READ_ONLY,
      srv_read_only_mode, &success);
  if (UNIV_UNLIKELY(!success)) {
    /* Here and below assume that bitmap file names do not
    contain apostrophes, thus no need for ut_print_filename(). */
    ib::warn() << "Error opening the changed page bitmap \'"
               << bitmap_file->name << "\'";
    return false;
  }

  bitmap_file->size = os_file_get_size(bitmap_file->file);
  bitmap_file->offset = 0;

  os_file_advise(bitmap_file->file, 0, 0, OS_FILE_ADVISE_SEQUENTIAL);
  os_file_advise(bitmap_file->file, 0, 0, OS_FILE_ADVISE_NOREUSE);

  return true;
}

/** Diagnose one or both of the following situations if we read close to
the end of bitmap file:
1) Warn if the remainder of the file is less than one page.
2) Error if we cannot read any more full pages but the last read page
did not have the last-in-run flag set.

@return false for the error */
static bool log_online_diagnose_bitmap_eof(
    const log_online_bitmap_file_t *bitmap_file, /*!< in: bitmap file */
    bool last_page_in_run)                       /*!< in: "last page in
                                run" flag value in the
                                last read page */
    noexcept {
  /* Check if we are too close to EOF to read a full page */
  if ((bitmap_file->size < MODIFIED_PAGE_BLOCK_SIZE) ||
      (bitmap_file->offset > bitmap_file->size - MODIFIED_PAGE_BLOCK_SIZE)) {
    if (UNIV_UNLIKELY(bitmap_file->offset != bitmap_file->size)) {
      /* If we are not at EOF and we have less than one page
      to read, it's junk.  This error is not fatal in
      itself. */

      ib::warn() << "Junk at the end of changed page bitmap "
                    "file \'"
                 << bitmap_file->name << "\'.";
    }

    if (UNIV_UNLIKELY(!last_page_in_run)) {
      /* We are at EOF but the last read page did not finish
      a run */
      /* It's a "Warning" here because it's not a fatal error
      for the whole server */
      ib::warn() << "Changed page bitmap file \'" << bitmap_file->name
                 << "\', size " << bitmap_file->size
                 << " bytes, does not "
                    "contain a complete run at the next read "
                    "offset "
                 << bitmap_file->offset;
      return false;
    }
  }
  return true;
}

/** Initialize the log bitmap iterator for a given range.  The records are
processed at a bitmap block granularity, i.e. all the records in the same block
share the same start and end LSN values, the exact LSN of each record is
unavailable (nor is it defined for blocks that are touched more than once in
the LSN interval contained in the block).  Thus min_lsn and max_lsn should be
set at block boundaries or bigger, otherwise the records at the 1st and the
last blocks will not be returned.  Also note that there might be returned
records with LSN < min_lsn, as min_lsn is used to select the correct starting
file but not block.

@return true if the iterator is initialized OK, false otherwise. */
bool log_online_bitmap_iterator_init(
    log_bitmap_iterator_t *i, /*!<in/out:  iterator */
    lsn_t min_lsn,            /*!< in: start LSN */
    lsn_t max_lsn)            /*!< in: end LSN */
{
  ut_a(i);

  i->max_lsn = max_lsn;

  if (UNIV_UNLIKELY(min_lsn > max_lsn)) {
    /* Empty range */
    i->in_files.count = 0;
    i->in_files.files = nullptr;
    i->in.file.set_closed();
    i->page = nullptr;
    i->failed = false;
    return true;
  }

  if (!log_online_setup_bitmap_file_range(&i->in_files, min_lsn, max_lsn)) {
    i->failed = true;
    return false;
  }

  i->in_i = 0;

  if (i->in_files.count == 0) {
    /* Empty range */
    i->in.file.set_closed();
    i->page = nullptr;
    i->failed = false;
    return true;
  }

  /* Open the 1st bitmap file */
  if (UNIV_UNLIKELY(!log_online_open_bitmap_file_read_only(
          i->in_files.files[i->in_i].name, &i->in))) {
    i->in_i = i->in_files.count;
    ut_free(i->in_files.files);
    i->failed = true;
    return false;
  }

  i->page = static_cast<byte *>(
      ut_malloc(MODIFIED_PAGE_BLOCK_SIZE, mem_key_log_online_iterator_page));
  i->bit_offset = MODIFIED_PAGE_BLOCK_BITMAP_LEN;
  i->start_lsn = i->end_lsn = 0;
  i->space_id = 0;
  i->first_page_id = 0;
  i->last_page_in_run = true;
  i->changed = false;
  i->failed = false;

  return true;
}

/** Releases log bitmap iterator. */
void log_online_bitmap_iterator_release(
    log_bitmap_iterator_t *i) /*!<in/out:  iterator */
    noexcept {
  ut_a(i);

  if (!i->in.file.is_closed()) {
    os_file_close(i->in.file);
    i->in.file.set_closed();
  }
  if (i->in_files.files) {
    ut_free(i->in_files.files);
  }
  if (i->page) {
    ut_free(i->page);
  }
  i->failed = true;
}

/** Iterates through bits of saved bitmap blocks.
Sequentially reads blocks from bitmap file(s) and interates through
their bits. Ignores blocks with wrong checksum.
@return true if iteration is successful, false if all bits are iterated. */

bool log_online_bitmap_iterator_next(
    log_bitmap_iterator_t *i) /*!<in/out: iterator */
{
  ut_a(i);

  if (UNIV_UNLIKELY(i->in_files.count == 0)) {
    return false;
  }

  if (UNIV_LIKELY(i->bit_offset < MODIFIED_PAGE_BLOCK_BITMAP_LEN)) {
    ++i->bit_offset;
    i->changed =
        IS_BIT_SET(i->page + MODIFIED_PAGE_BLOCK_BITMAP, i->bit_offset);
    return true;
  }

  if (i->end_lsn >= i->max_lsn && i->last_page_in_run) return false;

  bool checksum_ok = false;
  while (!checksum_ok) {
    bool success;

    while (i->in.size < MODIFIED_PAGE_BLOCK_SIZE ||
           (i->in.offset > i->in.size - MODIFIED_PAGE_BLOCK_SIZE)) {
      /* Advance file */
      i->in_i++;
      success = os_file_close_no_error_handling(i->in.file);
      i->in.file.set_closed();
      if (UNIV_UNLIKELY(!success)) {
        os_file_get_last_error(true);
        i->failed = true;
        return false;
      }

      success = log_online_diagnose_bitmap_eof(&i->in, i->last_page_in_run);
      if (UNIV_UNLIKELY(!success)) {
        i->failed = true;
        return false;
      }

      if (i->in_i == i->in_files.count) {
        return false;
      }

      if (UNIV_UNLIKELY(i->in_files.files[i->in_i].seq_num == 0)) {
        i->failed = true;
        return false;
      }

      success = log_online_open_bitmap_file_read_only(
          i->in_files.files[i->in_i].name, &i->in);
      if (UNIV_UNLIKELY(!success)) {
        i->failed = true;
        return false;
      }
    }

    success = log_online_read_bitmap_page(&i->in, i->page, &checksum_ok);
    if (UNIV_UNLIKELY(!success)) {
      os_file_get_last_error(true);
      ib::warn() << "Failed reading changed page bitmap "
                    "file \'"
                 << i->in_files.files[i->in_i].name << "\'";
      i->failed = true;
      return false;
    }
  }

  i->start_lsn = mach_read_from_8(i->page + MODIFIED_PAGE_START_LSN);
  i->end_lsn = mach_read_from_8(i->page + MODIFIED_PAGE_END_LSN);
  i->space_id = mach_read_from_4(i->page + MODIFIED_PAGE_SPACE_ID);
  i->first_page_id = mach_read_from_4(i->page + MODIFIED_PAGE_1ST_PAGE_ID);
  i->last_page_in_run = mach_read_from_4(i->page + MODIFIED_PAGE_IS_LAST_BLOCK);
  i->bit_offset = 0;
  i->changed = IS_BIT_SET(i->page + MODIFIED_PAGE_BLOCK_BITMAP, i->bit_offset);

  return true;
}

/** Delete all the bitmap files for data less than the specified LSN.
If called with lsn == 0 (i.e. set by RESET request) or LSN_MAX,
restart the bitmap file sequence, otherwise continue it.

@return false to indicate success, true for failure. */
bool log_online_purge_changed_page_bitmaps(
    lsn_t lsn) /*!< in: LSN to purge files up to */
{
  if (lsn == 0) {
    lsn = LSN_MAX;
  }

  bool log_bmp_sys_inited = false;
  if (srv_redo_log_thread_started) {
    /* User requests might happen with both enabled and disabled
    tracking */
    log_bmp_sys_inited = true;
    mutex_enter(&log_bmp_sys_mutex);
    if (!srv_redo_log_thread_started) {
      log_bmp_sys_inited = false;
      mutex_exit(&log_bmp_sys_mutex);
    }
  }

  log_online_bitmap_file_range_t bitmap_files;
  if (!log_online_setup_bitmap_file_range(&bitmap_files, 0, LSN_MAX)) {
    if (log_bmp_sys_inited) {
      mutex_exit(&log_bmp_sys_mutex);
    }
    return true;
  }

  if (srv_redo_log_thread_started && lsn > log_bmp_sys->end_lsn) {
    ut_ad(log_bmp_sys->end_lsn > 0);
    /* If we have to delete the current output file, close it
    first. */
    os_file_close(log_bmp_sys->out.file);
    log_bmp_sys->out.file.set_closed();
  }

  bool result = false;

  for (size_t i = 0; i < bitmap_files.count; i++) {
    /* We consider the end LSN of the current bitmap, derived from
    the start LSN of the subsequent bitmap file, to determine
    whether to remove the current bitmap.  Note that bitmap_files
    does not contain an entry for the bitmap past the given LSN so
    we must check the boundary conditions as well.  For example,
    consider 1_0.xdb and 2_10.xdb and querying LSN 5.  bitmap_files
    will only contain 1_0.xdb and we must not delete it since it
    represents LSNs 0-9. */
    if ((i + 1 == bitmap_files.count ||
         bitmap_files.files[i + 1].seq_num == 0 ||
         bitmap_files.files[i + 1].start_lsn > lsn) &&
        (lsn != LSN_MAX)) {
      break;
    }

    /* In some non-trivial cases the sequence of .xdb files may
       have gaps. For instance:
       ib_modified_log_1_0.xdb
       ib_modified_log_2_<mmm>.xdb
       ib_modified_log_4_<nnn>.xdb
       Adding this check as a safety precaution. */
    if (bitmap_files.files[i].name[0] == '\0') continue;

    /* If redo log tracking is enabled, reuse 'bmp_file_home' from
    'log_bmp_sys'. Otherwise, compose the full '.xdb' file path from
    'srv_data_home', adding a path separator if necessary. */
    char full_bmp_file_name[2 * FN_REFLEN + 2];

    if (log_bmp_sys != nullptr)
      snprintf(full_bmp_file_name, sizeof(full_bmp_file_name), "%s%s",
               log_bmp_sys->bmp_file_home, bitmap_files.files[i].name);
    else {
      char separator[2] = {0, 0};
      const auto srv_data_home_len = strlen(srv_data_home);

      ut_a(srv_data_home_len < FN_REFLEN);
      if (srv_data_home_len != 0 &&
          srv_data_home[srv_data_home_len - 1] != SRV_PATH_SEPARATOR) {
        separator[0] = SRV_PATH_SEPARATOR;
      }
      snprintf(full_bmp_file_name, sizeof(full_bmp_file_name), "%s%s%s",
               srv_data_home, separator, bitmap_files.files[i].name);
    }

    if (!os_file_delete_if_exists(innodb_bmp_file_key, full_bmp_file_name,
                                  nullptr)) {
      os_file_get_last_error(true);
      result = true;
      break;
    }
  }

  if (log_bmp_sys_inited) {
    if (lsn > log_bmp_sys->end_lsn) {
      lsn_t new_file_lsn;
      if (lsn == LSN_MAX) {
        /* RESET restarts the sequence */
        log_bmp_sys->out_seq_num = 0;
        new_file_lsn = 0;
      } else {
        new_file_lsn = log_bmp_sys->end_lsn;
      }
      if (!log_online_rotate_bitmap_file(new_file_lsn)) {
        /* If file create failed, stop log tracking */
        srv_track_changed_pages = false;
      }
    }

    ut_ad(!log_bmp_sys->out.file.is_closed() || !srv_track_changed_pages);
    mutex_exit(&log_bmp_sys_mutex);
  }

  ut_free(bitmap_files.files);
  return result;
}
