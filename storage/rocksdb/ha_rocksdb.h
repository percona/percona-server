/*
   Copyright (c) 2012,2013 Monty Program Ab

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
#pragma once

#ifdef USE_PRAGMA_INTERFACE
#pragma interface /* gcc class implementation */
#endif

#define ROCKSDB_INCLUDE_RFR 1

/* C++ standard header files */
#include <cinttypes>
#include <deque>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/* MySQL header files */
#include "ib_ut0counter.h"
#include "my_icp.h"
#include "mysql/psi/mysql_rwlock.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/sql_bitmap.h"
#include "sql_string.h"

/* RocksDB header files */
#include "rocksdb/cache.h"
#include "rocksdb/merge_operator.h"
#include "rocksdb/perf_context.h"
#include "rocksdb/sst_file_manager.h"
#include "rocksdb/statistics.h"
#include "rocksdb/utilities/options_util.h"
#include "rocksdb/utilities/transaction_db.h"
#include "rocksdb/utilities/write_batch_with_index.h"

/* MyRocks header files */
#include "./rdb_buff.h"
#include "./rdb_global.h"
#include "./rdb_index_merge.h"
#include "./rdb_perf_context.h"
#include "./rdb_sst_info.h"
#include "./rdb_utils.h"

/**
  @note MyRocks Coding Conventions:
  MyRocks code follows the baseline MySQL coding conventions, available at
  http://dev.mysql.com/doc/internals/en/coding-guidelines.html, with several
  refinements (@see /storage/rocksdb/README file).
*/

namespace myrocks {

class Rdb_converter;
class Rdb_iterator;
class Rdb_iterator_base;
class Rdb_key_def;
class Rdb_tbl_def;
class Rdb_transaction;
class Rdb_transaction_impl;
class Rdb_writebatch_impl;
class Rdb_field_encoder;
class Regex_list_handler;

extern ulong rocksdb_max_row_locks;
#if defined(HAVE_PSI_INTERFACE)
extern PSI_rwlock_key key_rwlock_read_free_rpl_tables;
#endif
extern Regex_list_handler rdb_read_free_regex_handler;
extern bool rocksdb_column_default_value_as_expression;
/**
  @brief
  Rdb_table_handler is a reference-counted structure storing information for
  each open table. All the objects are stored in a global hash map.

  //TODO: join this with Rdb_tbl_def ?
*/
struct Rdb_table_handler {
  char *m_table_name;
  uint m_table_name_length;
  int m_ref_count;

  my_core::THR_LOCK m_thr_lock;  ///< MySQL latch needed by m_db_lock

  /* Stores cumulative table statistics */
  my_io_perf_atomic_t m_io_perf_read;
  Rdb_atomic_perf_counters m_table_perf_context;
};

}  // namespace myrocks

/* Provide hash function for GL_INDEX_ID so we can include it in sets */
namespace std {
template <>
struct hash<myrocks::GL_INDEX_ID> {
  std::size_t operator()(const myrocks::GL_INDEX_ID &gl_index_id) const {
    const uint64_t val =
        ((uint64_t)gl_index_id.cf_id << 32 | (uint64_t)gl_index_id.index_id);
    return std::hash<uint64_t>()(val);
  }
};
}  // namespace std

namespace myrocks {
enum table_cardinality_scan_type {
  SCAN_TYPE_NONE,
  SCAN_TYPE_MEMTABLE_ONLY,
  SCAN_TYPE_FULL_TABLE,
};

enum Rdb_lock_type { RDB_LOCK_NONE, RDB_LOCK_READ, RDB_LOCK_WRITE };

class blob_buffer {
 public:
  ~blob_buffer() { release_blob_buffer(); }
  /*
    Returns the buffer of size(current_size) which will be used
    to store a blob value while unpacking keys from covering index.
  */
  uchar *get_blob_buffer(uint current_size);

  /*
    Resets the m_blob_buffer_current to m_blob_buffer_start.
    If m_blob_buffer_start is nullptr, then the buffer of size total_size
    will be allocated.
  */
  bool reset_blob_buffer(uint total_size);

  /*
    Releases the blob buffer memory
  */
  void release_blob_buffer();

 protected:
  /*
    In case blob indexes are covering, then this buffer will be used
    to store the unpacked blob values temporarily.
    Alocation of m_blob_buffer_start will be done as part of reset_blob_buffer()
    and deallocation will be done in release_blob_buffer()
    Use case of below 3 parameters -
    1) m_blob_buffer_start - stores start pointer of the blob buffer.
    2) m_blob_buffer_current - stores current pointer where we can store blob
    data.
    3) m_total_blob_buffer_allocated - amount of total buffer alocated.
  */
  uchar *m_blob_buffer_start = nullptr;

  uchar *m_blob_buffer_current = nullptr;

  uint m_total_blob_buffer_allocated = 0;
};

/**
  @brief
  Class definition for ROCKSDB storage engine plugin handler
*/

class ha_rocksdb : public my_core::handler, public blob_buffer {
  my_core::THR_LOCK_DATA m_db_lock;  ///< MySQL database lock

  Rdb_table_handler *m_table_handler;  ///< Open table handler

  Rdb_tbl_def *m_tbl_def;

  /* Primary Key encoder from KeyTupleFormat to StorageFormat */
  std::shared_ptr<Rdb_key_def> m_pk_descr;

  /* Array of index descriptors */
  std::shared_ptr<Rdb_key_def> *m_key_descr_arr;

  static bool check_keyread_allowed(bool &pk_can_be_decoded,
                                    const TABLE_SHARE *table_share, uint inx,
                                    uint part, bool all_parts);

  /*
    true <=> Primary Key columns can be decoded from the index. It should be
    enabled by default and may be disabled in init_with_fields() after initial
    keys info is loaded and it turns out the feature isn't supported for
    particular table.
  */
  mutable bool m_pk_can_be_decoded;

  uchar *m_pk_packed_tuple; /* Buffer for storing PK in StorageFormat */
  // ^^ todo: change it to 'char*'? TODO: ^ can we join this with last_rowkey?

  /*
    Temporary buffers for storing the key part of the Key/Value pair
    for secondary indexes.
  */
  uchar *m_sk_packed_tuple;

  /*
    Temporary buffers for storing end key part of the Key/Value pair.
    This is used for range scan only.
  */
  uchar *m_end_key_packed_tuple;

  Rdb_string_writer m_sk_tails;
  Rdb_string_writer m_pk_unpack_info;

  /* Second buffers, used by UPDATE. */
  uchar *m_sk_packed_tuple_old;
  Rdb_string_writer m_sk_tails_old;

  /*
     Temporary buffer being used to store updated values of SK tuple
     to compare with possibly stale SK due to a concurrent update
     (like in the case of a snapshot conflict)
     currently being used in the iterator and point query paths
  */
  uchar *m_sk_packed_tuple_updated;

  /*
    Temporary space for packing VARCHARs (we provide it to
    pack_record()/pack_index_tuple() calls).
  */
  uchar *m_pack_buffer;

  /* class to convert between Mysql format and RocksDB format*/
  std::unique_ptr<Rdb_converter> m_converter;

  std::unique_ptr<Rdb_iterator> m_iterator;
  std::unique_ptr<Rdb_iterator_base> m_pk_iterator;

  /*
    Pointer to the original TTL timestamp value (8 bytes) during UPDATE.
  */
  char *m_ttl_bytes;
  /*
    The TTL timestamp value can change if the explicit TTL column is
    updated. If we detect this when updating the PK, we indicate it here so
    we know we must always update any SK's.
  */
  bool m_ttl_bytes_updated;

  /* rowkey of the last record we've read, in StorageFormat. */
  String m_last_rowkey;

  /*
    Last retrieved record, in table->record[0] data format.

    This is used only when we get the record with rocksdb's Get() call (The
    other option is when we get a rocksdb::Slice from an iterator)
  */
  rocksdb::PinnableSlice m_retrieved_record;

  /*
    For INSERT ON DUPLICATE KEY UPDATE, we store the duplicate record during
    write_row here so that we don't have to re-read in the following
    index_read.

    See also m_insert_with_update.
  */
  rocksdb::PinnableSlice m_dup_key_retrieved_record;

  /* Type of locking to apply to rows */
  Rdb_lock_type m_lock_rows;

  thr_locked_row_action m_locked_row_action;

  /* true means we're doing an index-only read. false means otherwise. */
  bool m_keyread_only;

  /* We only iterate but don't need to decode anything */
  bool m_iteration_only;

  bool m_rnd_scan_started;

  bool m_full_key_lookup = false;

  /*
    TRUE means INSERT ON DUPLICATE KEY UPDATE. In such case we can optimize by
    remember the failed attempt (if there is one that violates uniqueness check)
    in write_row and in the following index_read to skip the lock check and read
    entirely
   */
  bool m_insert_with_update;

  /*
    TRUE if last time the insertion failed due to duplicate key error.
    (m_dupp_errkey holds the key# that we've had error for)
  */
  bool m_dup_key_found;

#ifndef NDEBUG
  /*
    Index tuple (for duplicate PK/unique SK). Used for sanity checking.
  */
  String m_dup_key_tuple;
#endif

  /**
    @brief
    This is a bitmap of indexes (i.e. a set) whose keys (in future, values) may
    be changed by this statement. Indexes that are not in the bitmap do not need
    to be updated.
    @note Valid inside UPDATE statements, IIF(old_pk_slice is set).
  */
  my_core::Bitmap<((MAX_INDEXES + 7) / 8 * 8)> m_update_scope;

  /* SST information used for bulk loading the primary key */
  std::shared_ptr<Rdb_sst_info> m_sst_info;

  /*
    MySQL index number for duplicate key error
  */
  uint m_dupp_errkey;

  [[nodiscard]] int create_key_defs(
      const TABLE &table_arg, Rdb_tbl_def &tbl_def_arg,
      const std::string &actual_user_table_name, bool is_dd_tbl,
      const TABLE *const old_table_arg = nullptr,
      const Rdb_tbl_def *const old_tbl_def_arg = nullptr) const;

  int secondary_index_read(const int keyno, uchar *const buf,
                           const rocksdb::Slice *key,
                           const rocksdb::Slice *value, bool *skip_row)
      MY_ATTRIBUTE((__warn_unused_result__));

  rocksdb::Status get_for_update(Rdb_transaction *const tx,
                                 const Rdb_key_def &kd,
                                 const rocksdb::Slice &key) const;

  int fill_virtual_columns();

  int get_row_by_rowid(uchar *const buf, const char *const rowid,
                       const uint rowid_size, bool *skip_row = nullptr,
                       const bool skip_lookup = false,
                       const bool skip_ttl_check = true)
      MY_ATTRIBUTE((__warn_unused_result__));
  int get_row_by_rowid(uchar *const buf, const uchar *const rowid,
                       const uint rowid_size, bool *skip_row = nullptr,
                       const bool skip_lookup = false,
                       const bool skip_ttl_check = true)
      MY_ATTRIBUTE((__warn_unused_result__)) {
    return get_row_by_rowid(buf, reinterpret_cast<const char *>(rowid),
                            rowid_size, skip_row, skip_lookup, skip_ttl_check);
  }
  int get_row_by_sk(uchar *buf, const Rdb_key_def &kd,
                    const rocksdb::Slice *key);

  void load_auto_incr_value();
  ulonglong load_auto_incr_value_from_index();
  void update_auto_incr_val(ulonglong val);
  void update_auto_incr_val_from_field();
  rocksdb::Status get_datadic_auto_incr(Rdb_transaction *const tx,
                                        const GL_INDEX_ID &gl_index_id,
                                        ulonglong *new_val) const;
  longlong update_hidden_pk_val();
  int load_hidden_pk_value() MY_ATTRIBUTE((__warn_unused_result__));
  int read_hidden_pk_id_from_rowkey(longlong *const hidden_pk_id)
      MY_ATTRIBUTE((__warn_unused_result__));
  bool can_use_single_delete(const uint index) const
      MY_ATTRIBUTE((__warn_unused_result__));
  bool is_blind_delete_enabled();
  bool skip_unique_check() const MY_ATTRIBUTE((__warn_unused_result__));
  bool do_bulk_commit(Rdb_transaction *const tx)
      MY_ATTRIBUTE((__warn_unused_result__));
  [[nodiscard]] static bool has_hidden_pk(const TABLE &t);

  void update_row_stats(const operation_type &type, ulonglong count = 1);

  void set_last_rowkey(const uchar *const old_data);
  void set_last_rowkey(const char *str, size_t len);

  [[nodiscard]] int alloc_key_buffers(const TABLE &table_arg,
                                      const Rdb_tbl_def &tbl_def_arg);
  void free_key_buffers();

  // the buffer size should be at least 2*Rdb_key_def::INDEX_NUMBER_SIZE
  rocksdb::Range get_range(const int i, uchar buf[]) const;

  void records_in_range_internal(uint inx, key_range *const min_key,
                                 key_range *const max_key, int64 disk_size,
                                 int64 rows, ulonglong *total_size,
                                 ulonglong *row_count);

  /*
    Perf timers for data reads
  */
  Rdb_io_perf m_io_perf;

 public:
  static rocksdb::Range get_range(const Rdb_key_def &kd, uchar buf[]);

  /*
    Update stats
  */
  static int update_stats(ha_statistics *ha_stats, Rdb_tbl_def *tbl_def,
                          bool from_handler = false);

  /*
    Controls whether writes include checksums. This is updated from the session
    variable
    at the start of each query.
  */
  bool m_store_row_debug_checksums;

  int m_checksums_pct;

  /* stores the count of index keys with checksum */
  ha_rows m_validated_checksums = 0;

  ha_rocksdb(my_core::handlerton *const hton,
             my_core::TABLE_SHARE *const table_arg);
  virtual ~ha_rocksdb() override;

  /** @brief
    The name that will be used for display purposes.
   */
  const char *table_type() const override {
    DBUG_ENTER_FUNC();

    DBUG_RETURN(rocksdb_hton_name);
  }

  /*
    Returns the name of the table's base name
  */
  const std::string &get_table_basename() const;

  /** @brief
    This is a list of flags that indicate what functionality the storage engine
    implements. The current table flags are documented in handler.h
  */
  ulonglong table_flags() const override {
    DBUG_ENTER_FUNC();

    /*
      HA_BINLOG_STMT_CAPABLE
        We are saying that this engine is just statement capable to have
        an engine that can only handle statement-based logging. This is
        used in testing.
    */
    DBUG_RETURN(HA_BINLOG_ROW_CAPABLE | HA_BINLOG_STMT_CAPABLE |
                HA_CAN_INDEX_BLOBS |
                (m_pk_can_be_decoded ? HA_PRIMARY_KEY_IN_READ_INDEX : 0) |
                HA_PRIMARY_KEY_REQUIRED_FOR_POSITION | HA_NULL_IN_KEY |
                HA_PARTIAL_COLUMN_READ | HA_ONLINE_ANALYZE |
                HA_GENERATED_COLUMNS | HA_CAN_INDEX_VIRTUAL_GENERATED_COLUMN |
                (rocksdb_column_default_value_as_expression
                     ? HA_SUPPORTS_DEFAULT_EXPRESSION
                     : 0) |
                HA_ATTACHABLE_TRX_COMPATIBLE);
  }

  bool init_with_fields() override;

  static bool allow_unsafe_alter() noexcept;

  static ulong index_flags(bool &pk_can_be_decoded,
                           const TABLE_SHARE *table_share, uint inx, uint part,
                           bool all_parts);

  /** @brief
    This is a bitmap of flags that indicates how the storage engine
    implements indexes. The current index flags are documented in
    handler.h. If you do not implement indexes, just return zero here.

    @details
    part is the key part to check. First key part is 0.
    If all_parts is set, MySQL wants to know the flags for the combined
    index, up to and including 'part'.
  */
  ulong index_flags(uint inx, uint part, bool all_parts) const override;

  bool rpl_can_handle_stm_event() const noexcept override;

  bool primary_key_is_clustered() const override {
    DBUG_ENTER_FUNC();

    DBUG_RETURN(true);
  }

  bool should_store_row_debug_checksums() const {
    return m_store_row_debug_checksums && (rand() % 100 < m_checksums_pct);
  }

  int rename_table(const char *const from, const char *const to,
                   const dd::Table *from_table_def MY_ATTRIBUTE((__unused__)),
                   dd::Table *to_table_def MY_ATTRIBUTE((__unused__))) override
      MY_ATTRIBUTE((__warn_unused_result__, __nonnull__(2, 3)));

  int convert_record_from_storage_format(const rocksdb::Slice *const key,
                                         const rocksdb::Slice *const value,
                                         uchar *const buf)
      MY_ATTRIBUTE((__warn_unused_result__));

  int convert_record_from_storage_format(const rocksdb::Slice *const key,
                                         uchar *const buf)
      MY_ATTRIBUTE((__nonnull__, __warn_unused_result__));

  static const std::vector<std::string> parse_into_tokens(const std::string &s,
                                                          const char delim);

  [[nodiscard]] static const std::string generate_cf_name(
      uint index, const TABLE &table_arg, const Rdb_tbl_def &tbl_def_arg,
      bool &per_part_match_found);

  [[nodiscard]] static const char *get_key_name(uint index,
                                                const TABLE &table_arg,
                                                const Rdb_tbl_def &tbl_def_arg);

  [[nodiscard]] static const char *get_key_comment(
      uint index, const TABLE &table_arg, const Rdb_tbl_def &tbl_def_arg);

  static const std::string get_table_comment(const TABLE *const table_arg)
      MY_ATTRIBUTE((__warn_unused_result__));

  [[nodiscard]] static bool is_hidden_pk(uint index, const TABLE &table_arg,
                                         const Rdb_tbl_def &tbl_def_arg);

  [[nodiscard]] static uint pk_index(const TABLE &table_arg,
                                     const Rdb_tbl_def &tbl_def_arg);

  uint active_index_pos() MY_ATTRIBUTE((__warn_unused_result__));

  [[nodiscard]] static bool is_pk(uint index, const TABLE &table_arg,
                                  const Rdb_tbl_def &tbl_def_arg);
  /** @brief
    unireg.cc will call max_supported_record_length(), max_supported_keys(),
    max_supported_key_parts(), uint max_supported_key_length()
    to make sure that the storage engine can handle the data it is about to
    send. Return *real* limits of your storage engine here; MySQL will do
    min(your_limits, MySQL_limits) automatically.
   */
  uint max_supported_record_length() const override {
    DBUG_ENTER_FUNC();

    DBUG_RETURN(HA_MAX_REC_LENGTH);
  }

  uint max_supported_keys() const override {
    DBUG_ENTER_FUNC();

    DBUG_RETURN(MAX_INDEXES);
  }

  uint max_supported_key_parts() const override {
    DBUG_ENTER_FUNC();

    DBUG_RETURN(MAX_REF_PARTS);
  }

  uint max_supported_key_part_length(
      HA_CREATE_INFO *create_info) const override;

  /** @brief
    unireg.cc will call this to make sure that the storage engine can handle
    the data it is about to send. Return *real* limits of your storage engine
    here; MySQL will do min(your_limits, MySQL_limits) automatically.

      @details
    There is no need to implement ..._key_... methods if your engine doesn't
    support indexes.
   */
  uint max_supported_key_length() const override {
    DBUG_ENTER_FUNC();

    DBUG_RETURN(16 * 1024); /* just to return something*/
  }

  /**
    TODO: return actual upper bound of number of records in the table.
    (e.g. save number of records seen on full table scan and/or use file size
    as upper bound)
  */
  ha_rows estimate_rows_upper_bound() override {
    DBUG_ENTER_FUNC();

    DBUG_RETURN(HA_POS_ERROR);
  }

  int index_read_map(uchar *const buf, const uchar *const key,
                     key_part_map keypart_map,
                     enum ha_rkey_function find_flag) override
      MY_ATTRIBUTE((__warn_unused_result__));

  int index_read_last_map(uchar *const buf, const uchar *const key,
                          key_part_map keypart_map) override
      MY_ATTRIBUTE((__warn_unused_result__));

  virtual double scan_time() override {
    DBUG_ENTER_FUNC();

    DBUG_RETURN(
        static_cast<double>((stats.records + stats.deleted) / 20.0 + 10));
  }

  virtual double read_time(uint, uint, ha_rows rows) override;
  virtual void print_error(int error, myf errflag) override;

  int open(const char *const name, int mode, uint test_if_locked,
           const dd::Table *table_def) override
      MY_ATTRIBUTE((__warn_unused_result__));
  int close(void) override MY_ATTRIBUTE((__warn_unused_result__));

  int write_row(uchar *const buf) override
      MY_ATTRIBUTE((__warn_unused_result__));
  int update_row(const uchar *const old_data, uchar *const new_data) override
      MY_ATTRIBUTE((__warn_unused_result__));
  int delete_row(const uchar *const buf) override
      MY_ATTRIBUTE((__warn_unused_result__));
  void update_table_stats_if_needed();
  rocksdb::Status delete_or_singledelete(uint index, Rdb_transaction *const tx,
                                         rocksdb::ColumnFamilyHandle *const cf,
                                         const rocksdb::Slice &key)
      MY_ATTRIBUTE((__warn_unused_result__));

  int index_next(uchar *const buf) override
      MY_ATTRIBUTE((__warn_unused_result__));
  int index_next_same(uchar *const buf, const uchar *key, uint keylen) override
      MY_ATTRIBUTE((__warn_unused_result__));
  int index_prev(uchar *const buf) override
      MY_ATTRIBUTE((__warn_unused_result__));

  int index_first(uchar *const buf) override
      MY_ATTRIBUTE((__warn_unused_result__));
  int index_last(uchar *const buf) override
      MY_ATTRIBUTE((__warn_unused_result__));

  class Item *idx_cond_push(uint keyno, class Item *const idx_cond) override;
  /*
    Default implementation from cancel_pushed_idx_cond() suits us
  */
  static bool check_bloom_and_set_bounds(
      THD *thd, const Rdb_key_def &kd, const rocksdb::Slice &eq_cond,
      size_t bound_len, uchar *const lower_bound, uchar *const upper_bound,
      rocksdb::Slice *lower_bound_slice, rocksdb::Slice *upper_bound_slice,
      bool *check_iterate_bounds);
  static bool can_use_bloom_filter(THD *thd, const Rdb_key_def &kd,
                                   const rocksdb::Slice &eq_cond);

 private:
  friend class Rdb_iterator;
  friend class Rdb_iterator_base;

  struct key_def_cf_info {
    std::shared_ptr<rocksdb::ColumnFamilyHandle> cf_handle;
    bool is_reverse_cf;
    bool is_per_partition_cf;
  };

  struct update_row_info {
    Rdb_transaction *tx;
    const uchar *new_data;
    const uchar *old_data;
    rocksdb::Slice new_pk_slice;
    rocksdb::Slice old_pk_slice;

    // "unpack_info" data for the new PK value
    Rdb_string_writer *new_pk_unpack_info;

    longlong hidden_pk_id;
    bool skip_unique_check;
  };

  /** Flags indicating if current operation can be done instantly */
  enum class Instant_Type : uint16_t {
    /** Impossible to alter instantly */
    INSTANT_IMPOSSIBLE,

    /** Can be instant without any change */
    INSTANT_NO_CHANGE,

    /** Adding or dropping virtual columns only */
    INSTANT_VIRTUAL_ONLY,

    /** ADD COLUMN which can be done instantly, including
    adding stored column only (or along with adding virtual columns) */
    INSTANT_ADD_COLUMN
  };

  [[nodiscard]] int create_table(const std::string &table_name,
                                 const std::string &actual_user_table_name,
                                 const TABLE &table_arg,
                                 ulonglong auto_increment_value,
                                 const dd::Table *table_def);

  [[nodiscard]] bool create_cfs(
      const TABLE &table_arg, const Rdb_tbl_def &tbl_def_arg,
      const std::string &actual_user_table_name,
      std::array<struct key_def_cf_info, MAX_INDEXES + 1> &cfs,
      bool is_dd_tbl) const;

  [[nodiscard]] int create_key_def(const TABLE &table_arg, uint i,
                                   const Rdb_tbl_def &tbl_def_arg,
                                   std::shared_ptr<Rdb_key_def> &new_key_def,
                                   const struct key_def_cf_info &cf_info,
                                   uint64 ttl_duration,
                                   const std::string &ttl_column,
                                   bool is_dd_tbl = false) const;

  [[nodiscard]] bool create_inplace_key_defs(
      const TABLE &table_arg, Rdb_tbl_def &tbl_def_arg,
      const TABLE &old_table_arg, const Rdb_tbl_def &old_tbl_def_arg,
      const std::array<key_def_cf_info, MAX_INDEXES + 1> &cfs,
      uint64 ttl_duration, const std::string &ttl_column) const;

  [[nodiscard]] std::unordered_map<std::string, uint> get_old_key_positions(
      const TABLE &table_arg, const Rdb_tbl_def &tbl_def_arg,
      const TABLE &old_table_arg, const Rdb_tbl_def &old_tbl_def_arg) const;

  int compare_key_parts(const KEY *const old_key,
                        const KEY *const new_key) const
      MY_ATTRIBUTE((__warn_unused_result__));

  int compare_keys(const KEY *const old_key, const KEY *const new_key) const
      MY_ATTRIBUTE((__warn_unused_result__));

  int index_read_intern(uchar *const buf, const uchar *const key,
                        key_part_map keypart_map,
                        enum ha_rkey_function find_flag)
      MY_ATTRIBUTE((__warn_unused_result__));
  int index_read_intern(uchar *buf, bool first)
      MY_ATTRIBUTE((__nonnull__, __warn_unused_result__));
  int index_next_with_direction_intern(uchar *const buf, bool forward,
                                       bool skip_next)
      MY_ATTRIBUTE((__warn_unused_result__));
  Rdb_iterator_base *get_pk_iterator() MY_ATTRIBUTE((__warn_unused_result__));

  enum icp_result check_index_cond() const;

  void calc_updated_indexes();
  int update_write_row(const uchar *const old_data, const uchar *const new_data,
                       const bool skip_unique_check)
      MY_ATTRIBUTE((__warn_unused_result__));
  int get_pk_for_update(struct update_row_info *const row_info);
  int check_and_lock_unique_pk(const struct update_row_info &row_info,
                               bool *const found, const bool skip_unique_check)
      MY_ATTRIBUTE((__warn_unused_result__));
  int acquire_prefix_lock(const Rdb_key_def &kd, Rdb_transaction *tx,
                          const uchar *data)
      MY_ATTRIBUTE((__warn_unused_result__));
  int check_and_lock_sk(const uint key_id,
                        const struct update_row_info &row_info,
                        bool *const found, const bool skip_unique_check)
      MY_ATTRIBUTE((__warn_unused_result__));
  int check_uniqueness_and_lock(const struct update_row_info &row_info,
                                bool pk_changed, const bool skip_unique_check)
      MY_ATTRIBUTE((__warn_unused_result__));
  bool over_bulk_load_threshold(int *err)
      MY_ATTRIBUTE((__warn_unused_result__));
  int check_duplicate_sk(const TABLE *table_arg, const Rdb_key_def &key_def,
                         const rocksdb::Slice *key,
                         struct unique_sk_buf_info *sk_info)
      MY_ATTRIBUTE((__nonnull__, __warn_unused_result__));
  int bulk_load_key(Rdb_transaction *const tx, const Rdb_key_def &kd,
                    const rocksdb::Slice &key, const rocksdb::Slice &value,
                    bool sort)
      MY_ATTRIBUTE((__nonnull__, __warn_unused_result__));
  int update_write_pk(const Rdb_key_def &kd,
                      const struct update_row_info &row_info,
                      const bool pk_changed)
      MY_ATTRIBUTE((__warn_unused_result__));

  int check_partial_index_prefix(const TABLE *table_arg, const Rdb_key_def &kd,
                                 Rdb_transaction *tx, const uchar *data)
      MY_ATTRIBUTE((__warn_unused_result__));
  int update_write_sk(const TABLE *const table_arg, const Rdb_key_def &kd,
                      const struct update_row_info &row_info,
                      const bool bulk_load_sk)
      MY_ATTRIBUTE((__warn_unused_result__));
  int update_write_indexes(const struct update_row_info &row_info,
                           const bool pk_changed)
      MY_ATTRIBUTE((__warn_unused_result__));

  Rdb_tbl_def *get_table_if_exists(const char *const tablename)
      MY_ATTRIBUTE((__warn_unused_result__));
  void read_thd_vars(THD *const thd) MY_ATTRIBUTE((__nonnull__));

  bool contains_foreign_key(THD *const thd)
      MY_ATTRIBUTE((__nonnull__, __warn_unused_result__));

  int inplace_populate_sk(
      TABLE *const table_arg,
      const std::unordered_set<std::shared_ptr<Rdb_key_def>> &indexes)
      MY_ATTRIBUTE((__nonnull__, __warn_unused_result__));

  int finalize_bulk_load(bool print_client_error = true)
      MY_ATTRIBUTE((__warn_unused_result__));

  void inc_table_n_rows();
  void dec_table_n_rows();

  bool should_skip_invalidated_record(const int rc) const;
  bool should_skip_locked_record(const int rc) const;
  bool should_recreate_snapshot(const int rc, const bool is_new_snapshot) const;

  bool can_assume_tracked(THD *thd);
  Instant_Type rocksdb_support_instant(
      my_core::Alter_inplace_info *const ha_alter_info, const TABLE *old_table,
      const TABLE *altered_table) const;

 public:
  void set_pk_can_be_decoded(bool flag) { m_pk_can_be_decoded = flag; }
  int index_init(uint idx, bool sorted) override
      MY_ATTRIBUTE((__warn_unused_result__));
  int index_end() override MY_ATTRIBUTE((__warn_unused_result__));

  void unlock_row() override;

  /** @brief
    Unlike index_init(), rnd_init() can be called two consecutive times
    without rnd_end() in between (it only makes sense if scan=1). In this
    case, the second call should prepare for the new table scan (e.g if
    rnd_init() allocates the cursor, the second call should position the
    cursor to the start of the table; no need to deallocate and allocate
    it again. This is a required method.
  */
  int rnd_init(bool scan) override MY_ATTRIBUTE((__warn_unused_result__));
  int rnd_end() override MY_ATTRIBUTE((__warn_unused_result__));
  int rnd_next(uchar *const buf) override
      MY_ATTRIBUTE((__warn_unused_result__));

  int rnd_pos(uchar *const buf, uchar *const pos) override
      MY_ATTRIBUTE((__warn_unused_result__));
  void position(const uchar *const record) override;
  int info(uint) override;

  /* This function will always return success, therefore no annotation related
   * to checking the return value. Can't change the signature because it's
   * required by the interface. */
  int extra(enum ha_extra_function operation) override;

  int start_stmt(THD *const thd, thr_lock_type lock_type) override
      MY_ATTRIBUTE((__warn_unused_result__));
  int external_lock(THD *const thd, int lock_type) override
      MY_ATTRIBUTE((__warn_unused_result__));
  int truncate(dd::Table *table_def) override
      MY_ATTRIBUTE((__warn_unused_result__));

  int reset() override;

  int check(THD *const thd, HA_CHECK_OPT *const check_opt) override
      MY_ATTRIBUTE((__warn_unused_result__));
  ha_rows records_in_range(uint inx, key_range *const min_key,
                           key_range *const max_key) override
      MY_ATTRIBUTE((__warn_unused_result__));

  int delete_table(Rdb_tbl_def *const tbl);
  int delete_table(const char *const from, const dd::Table *table_def) override
      MY_ATTRIBUTE((__warn_unused_result__));
  int create(const char *const name, TABLE *const form,
             HA_CREATE_INFO *const create_info, dd::Table *table_def) override
      MY_ATTRIBUTE((__warn_unused_result__));
  int truncate_table(Rdb_tbl_def *tbl_def,
                     const std::string &actual_user_table_name,
                     TABLE *table_arg, ulonglong auto_increment_value,
                     dd::Table *table_def);
  bool check_if_incompatible_data(HA_CREATE_INFO *const info,
                                  uint table_changes) override
      MY_ATTRIBUTE((__warn_unused_result__));

  THR_LOCK_DATA **store_lock(THD *const thd, THR_LOCK_DATA **to,
                             enum thr_lock_type lock_type) override
      MY_ATTRIBUTE((__warn_unused_result__));

  bool get_error_message(const int error, String *const buf) override;

  static int rdb_error_to_mysql(const rocksdb::Status &s,
                                const char *msg = nullptr)
      MY_ATTRIBUTE((__warn_unused_result__));

  void get_auto_increment(ulonglong offset, ulonglong increment,
                          ulonglong nb_desired_values,
                          ulonglong *const first_value,
                          ulonglong *const nb_reserved_values) override;
  void update_create_info(HA_CREATE_INFO *const create_info) override;
  int optimize(THD *const thd, HA_CHECK_OPT *const check_opt) override
      MY_ATTRIBUTE((__warn_unused_result__));
  int analyze(THD *const thd, HA_CHECK_OPT *const check_opt) override
      MY_ATTRIBUTE((__warn_unused_result__));

  enum_alter_inplace_result check_if_supported_inplace_alter(
      TABLE *altered_table,
      my_core::Alter_inplace_info *ha_alter_info) override;

  bool prepare_inplace_alter_table(TABLE *altered_table,
                                   my_core::Alter_inplace_info *ha_alter_info,
                                   const dd::Table *old_table_def,
                                   dd::Table *new_table_def) override;

  bool inplace_alter_table(TABLE *altered_table,
                           my_core::Alter_inplace_info *ha_alter_info,
                           const dd::Table *old_table_def,
                           dd::Table *new_table_def) override;

  bool commit_inplace_alter_table(
      TABLE *const altered_table,
      my_core::Alter_inplace_info *const ha_alter_info, bool commit,
      const dd::Table *old_table_def, dd::Table *new_table_def) override;

  /* Determine if this is an instant ALTER TABLE. */
  static bool is_instant(const Alter_inplace_info *ha_alter_info);

  bool is_read_free_rpl_table() const;
  static int adjust_handler_stats_sst_and_memtable(ha_statistics *ha_stats,
                                                   Rdb_tbl_def *tbl_def);
  static int adjust_handler_stats_table_scan(ha_statistics *ha_stats,
                                             Rdb_tbl_def *tbl_def);

  void update_row_read(ulonglong count);
  static void inc_covered_sk_lookup();

  void build_decoder();
  void check_build_decoder();

 protected:
  int records(ha_rows *num_rows) override;
  int records_from_index(ha_rows *num_rows, uint index) override;

#if defined(ROCKSDB_INCLUDE_RFR) && ROCKSDB_INCLUDE_RFR
 public:
  virtual void rpl_before_delete_rows() override;
  virtual void rpl_after_delete_rows() override;
  virtual void rpl_before_update_rows() override;
  virtual void rpl_after_update_rows() override;
  virtual bool rpl_lookup_rows() override;

  virtual bool use_read_free_rpl() const;  // MyRocks only

 private:
  /* Flags tracking if we are inside different replication operation */
  bool m_in_rpl_delete_rows;
  bool m_in_rpl_update_rows;
#endif  // defined(ROCKSDB_INCLUDE_RFR) && ROCKSDB_INCLUDE_RFR

  /* Need to build decoder on next read operation */
  bool m_need_build_decoder;
};

/*
  Helper class for in-place alter, for storing handler context between inplace
  alter calls
*/
struct Rdb_inplace_alter_ctx : public my_core::inplace_alter_handler_ctx {
  /* The new table definition */
  Rdb_tbl_def *const m_new_tdef;

  /* Stores the original key definitions */
  std::shared_ptr<Rdb_key_def> *const m_old_key_descr;

  /* Stores the new key definitions */
  std::shared_ptr<Rdb_key_def> *m_new_key_descr;

  /* Stores the old number of key definitions */
  const uint m_old_n_keys;

  /* Stores the new number of key definitions */
  const uint m_new_n_keys;

  /* Stores the added key glids */
  const std::unordered_set<std::shared_ptr<Rdb_key_def>> m_added_indexes;

  /* Stores the dropped key glids */
  const std::unordered_set<GL_INDEX_ID> m_dropped_index_ids;

  /* Stores number of keys to add */
  const uint m_n_added_keys;

  /* Stores number of keys to drop */
  const uint m_n_dropped_keys;

  /* Stores the largest current auto increment value in the index */
  const ulonglong m_max_auto_incr;

  Rdb_inplace_alter_ctx(
      Rdb_tbl_def *new_tdef, std::shared_ptr<Rdb_key_def> *old_key_descr,
      std::shared_ptr<Rdb_key_def> *new_key_descr, uint old_n_keys,
      uint new_n_keys,
      std::unordered_set<std::shared_ptr<Rdb_key_def>> added_indexes,
      std::unordered_set<GL_INDEX_ID> dropped_index_ids, uint n_added_keys,
      uint n_dropped_keys, ulonglong max_auto_incr)
      : my_core::inplace_alter_handler_ctx(),
        m_new_tdef(new_tdef),
        m_old_key_descr(old_key_descr),
        m_new_key_descr(new_key_descr),
        m_old_n_keys(old_n_keys),
        m_new_n_keys(new_n_keys),
        m_added_indexes(added_indexes),
        m_dropped_index_ids(dropped_index_ids),
        m_n_added_keys(n_added_keys),
        m_n_dropped_keys(n_dropped_keys),
        m_max_auto_incr(max_auto_incr) {}

  ~Rdb_inplace_alter_ctx() {}

 private:
  /* Disable Copying */
  Rdb_inplace_alter_ctx(const Rdb_inplace_alter_ctx &);
  Rdb_inplace_alter_ctx &operator=(const Rdb_inplace_alter_ctx &);
};

/*
  Helper class to control access/init to handlerton instance.
  Contains a flag that is set if the handlerton is in an initialized, usable
  state, plus a reader-writer lock to protect it without serializing reads.
  Since we don't have static initializers for the opaque mysql_rwlock type,
  use constructor and destructor functions to create and destroy
  the lock before and after main(), respectively.
*/
struct Rdb_hton_init_state {
  struct Scoped_lock {
    Scoped_lock(Rdb_hton_init_state &state, bool write) : m_state(state) {
      if (write)
        m_state.lock_write();
      else
        m_state.lock_read();
    }
    ~Scoped_lock() { m_state.unlock(); }

   private:
    Scoped_lock(const Scoped_lock &sl) : m_state(sl.m_state) {}
    void operator=(const Scoped_lock &) {}

    Rdb_hton_init_state &m_state;
  };

  Rdb_hton_init_state() : m_initialized(false) {
    /*
      m_rwlock can not be instrumented as it must be initialized before
      mysql_mutex_register() call to protect some globals from race condition.
    */
    mysql_rwlock_init(0, &m_rwlock);
  }

  ~Rdb_hton_init_state() { mysql_rwlock_destroy(&m_rwlock); }

  void lock_read() { mysql_rwlock_rdlock(&m_rwlock); }

  void lock_write() { mysql_rwlock_wrlock(&m_rwlock); }

  void unlock() { mysql_rwlock_unlock(&m_rwlock); }

  /*
    Must be called with either a read or write lock held, unable to enforce
    behavior as mysql_rwlock has no means of determining if a thread has a lock
  */
  bool initialized() const { return m_initialized; }

  /*
    Must be called with only a write lock held, unable to enforce behavior as
    mysql_rwlock has no means of determining if a thread has a lock
  */
  void set_initialized(bool init) { m_initialized = init; }

 private:
  mysql_rwlock_t m_rwlock;
  bool m_initialized;
};

// file name indicating RocksDB data corruption
std::string rdb_corruption_marker_file_name();

// get rocksdb_db_options
rocksdb::DBOptions *get_rocksdb_db_options();

struct Rdb_compaction_stats_record {
  time_t start_timestamp;
  time_t end_timestamp;
  rocksdb::CompactionJobInfo info;
};

// Holds records of recently run compaction jobs, including ongoing ones. This
// class accesses its internal data structures under a mutex lock.
class Rdb_compaction_stats {
 public:
  Rdb_compaction_stats() {}

  // resize_history() sets the max number of completed compactions for which we
  // store history. Records for all ongoing compactions are stored in addition
  // to at most `max_history_len` historical records.
  void resize_history(size_t max_history_len);

  // get_current_stats() returns the details of pending compactions only.
  std::vector<Rdb_compaction_stats_record> get_current_stats();

  // get_recent_history() returns the details of recently completed compactions.
  std::vector<Rdb_compaction_stats_record> get_recent_history();

  void record_start(rocksdb::CompactionJobInfo info);
  void record_end(rocksdb::CompactionJobInfo info);

 private:
  std::mutex m_mutex;
  // History of completed compactions.
  std::deque<Rdb_compaction_stats_record> m_history;
  size_t m_max_history_len = 0;
  // Hold ongoing compactions in a map keyed on thread ID, as we use that to
  // match `record_start()`s with `record_end()`s.
  std::map<uint64_t, Rdb_compaction_stats_record> m_tid_to_pending_compaction;
};

extern Rdb_compaction_stats compaction_stats;

unsigned long long get_partial_index_sort_max_mem(THD *thd);

Rdb_transaction *get_tx_from_thd(THD *const thd);

const rocksdb::ReadOptions &rdb_tx_acquire_snapshot(Rdb_transaction *tx);

rocksdb::Iterator *rdb_tx_get_iterator(
    THD *thd, rocksdb::ColumnFamilyHandle *const cf, bool skip_bloom_filter,
    const rocksdb::Slice &eq_cond_lower_bound,
    const rocksdb::Slice &eq_cond_upper_bound,
    const rocksdb::Snapshot **snapshot, bool read_current = false,
    bool create_snapshot = true);

rocksdb::Status rdb_tx_get(Rdb_transaction *tx,
                           rocksdb::ColumnFamilyHandle *const column_family,
                           const rocksdb::Slice &key,
                           rocksdb::PinnableSlice *const value);

rocksdb::Status rdb_tx_get_for_update(Rdb_transaction *tx,
                                      const Rdb_key_def &kd,
                                      const rocksdb::Slice &key,
                                      rocksdb::PinnableSlice *const value,
                                      bool exclusive, bool skip_wait);

void rdb_tx_release_lock(Rdb_transaction *tx, const Rdb_key_def &kd,
                         const rocksdb::Slice &key, bool force);

inline void rocksdb_smart_seek(bool seek_backward,
                               rocksdb::Iterator *const iter,
                               const rocksdb::Slice &key_slice) {
  if (seek_backward) {
    iter->SeekForPrev(key_slice);
  } else {
    iter->Seek(key_slice);
  }
}

inline void rocksdb_smart_next(bool seek_backward,
                               rocksdb::Iterator *const iter) {
  if (seek_backward) {
    iter->Prev();
  } else {
    iter->Next();
  }
}

inline void rocksdb_smart_prev(bool seek_backward,
                               rocksdb::Iterator *const iter) {
  if (seek_backward) {
    iter->Next();
  } else {
    iter->Prev();
  }
}

// If the iterator is not valid it might be because of EOF but might be due
// to IOError or corruption. The good practice is always check it.
// https://github.com/facebook/rocksdb/wiki/Iterator#error-handling
bool is_valid_iterator(rocksdb::Iterator *scan_it);

bool rdb_should_hide_ttl_rec(const Rdb_key_def &kd,
                             const rocksdb::Slice &ttl_rec_val,
                             Rdb_transaction *tx);

bool rdb_tx_started(Rdb_transaction *tx);
int rdb_tx_set_status_error(Rdb_transaction *tx, const rocksdb::Status &s,
                            const Rdb_key_def &kd,
                            const Rdb_tbl_def *const tbl_def);

extern std::atomic<uint64_t> rocksdb_partial_index_groups_sorted;
extern std::atomic<uint64_t> rocksdb_partial_index_groups_materialized;
extern std::atomic<uint64_t> rocksdb_partial_index_rows_sorted;
extern std::atomic<uint64_t> rocksdb_partial_index_rows_materialized;
extern bool rocksdb_enable_tmp_table;
extern bool rocksdb_enable_delete_range_for_drop_index;
extern bool rocksdb_disable_instant_ddl;
extern bool rocksdb_partial_index_ignore_killed;

extern unsigned long long rocksdb_converter_record_cached_length;
}  // namespace myrocks
