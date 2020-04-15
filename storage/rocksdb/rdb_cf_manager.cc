/*
   Copyright (c) 2014, SkySQL Ab

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

#ifdef USE_PRAGMA_IMPLEMENTATION
#pragma implementation  // gcc: Class implementation
#endif

/* MySQL header files */
#include "sql/debug_sync.h"

/* This C++ files header file */
#include "./rdb_cf_manager.h"
#include "./rdb_threads.h"

/* MyRocks header files */
#include <string>
#include <vector>
#include "./ha_rocksdb.h"
#include "./ha_rocksdb_proto.h"
#include "./rdb_datadic.h"
#include "./rdb_psi.h"

namespace myrocks {

/* Check if ColumnFamily name says it's a reverse-ordered CF */
bool Rdb_cf_manager::is_cf_name_reverse(const char *const name) {
  /* nullptr means the default CF is used.. (TODO: can the default CF be
   * reverse?) */
  return (name && !strncmp(name, "rev:", 4));
}

void Rdb_cf_manager::init(
    std::unique_ptr<Rdb_cf_options> &&cf_options,
    std::vector<rocksdb::ColumnFamilyHandle *> *const handles) {
#ifdef HAVE_PSI_INTERFACE
  mysql_mutex_init(rdb_cfm_mutex_key, &m_mutex, MY_MUTEX_INIT_FAST);
#else
  mysql_mutex_init(&m_mutex, MY_MUTEX_INIT_FAST);
#endif

  DBUG_ASSERT(cf_options != nullptr);
  DBUG_ASSERT(handles != nullptr);
  DBUG_ASSERT(handles->size() > 0);

  m_cf_options = std::move(cf_options);

  for (auto cfh_ptr : *handles) {
    DBUG_ASSERT(cfh_ptr != nullptr);

    std::shared_ptr<rocksdb::ColumnFamilyHandle> cfh(cfh_ptr);
    m_cf_name_map[cfh_ptr->GetName()] = cfh;
    m_cf_id_map[cfh_ptr->GetID()] = cfh;
  }
}

void Rdb_cf_manager::cleanup() {
  m_cf_name_map.clear();
  m_cf_id_map.clear();
  mysql_mutex_destroy(&m_mutex);
  m_cf_options = nullptr;
}

/*
  @brief
  Find column family by name. If it doesn't exist, maybe create it

  @detail
    See Rdb_cf_manager::get_cf
*/
std::shared_ptr<rocksdb::ColumnFamilyHandle> Rdb_cf_manager::get_or_create_cf(
    rocksdb::DB *const rdb, const std::string &cf_name_arg, bool create) {
  DBUG_ASSERT(rdb != nullptr);

  std::shared_ptr<rocksdb::ColumnFamilyHandle> cf_handle;

  if (cf_name_arg == PER_INDEX_CF_NAME) {
    // per-index column families is no longer supported.
    my_error(ER_PER_INDEX_CF_DEPRECATED, MYF(0));
    return cf_handle;
  }

  const std::string &cf_name =
      cf_name_arg.empty() ? DEFAULT_CF_NAME : cf_name_arg;

  RDB_MUTEX_LOCK_CHECK(m_mutex);

  const auto it = m_cf_name_map.find(cf_name);

  if (it != m_cf_name_map.end()) {
    cf_handle = it->second;
  } else {
    /* Create a Column Family. */
    rocksdb::ColumnFamilyOptions opts;

    bool cf_name_found = m_cf_options->get_cf_options(cf_name, &opts);

    if (create || cf_name_found) {
      LogPluginErrMsg(INFORMATION_LEVEL, 0, "Creating a column family %s",
                      cf_name.c_str());
      LogPluginErrMsg(INFORMATION_LEVEL, 0, "    write_buffer_size=%ld",
                      opts.write_buffer_size);
      LogPluginErrMsg(INFORMATION_LEVEL, 0,
                      "    target_file_size_base=%" PRIu64,
                      opts.target_file_size_base);

      rocksdb::ColumnFamilyHandle *cf_handle_ptr = nullptr;
      const rocksdb::Status s =
          rdb->CreateColumnFamily(opts, cf_name, &cf_handle_ptr);

      if (s.ok()) {
        DBUG_ASSERT(cf_handle_ptr != nullptr);
        cf_handle.reset(cf_handle_ptr);
        m_cf_name_map[cf_handle_ptr->GetName()] = cf_handle;
        m_cf_id_map[cf_handle_ptr->GetID()] = cf_handle;
      }
    } else {
      my_error(ER_WRONG_ARGUMENTS, MYF(0),
               "CREATE | ALTER - can not find "
               "column family for storing index data and creation is not "
               "allowed.");
      cf_handle = nullptr;
    }
  }

  RDB_MUTEX_UNLOCK_CHECK(m_mutex);

  return cf_handle;
}

/*
  Find column family by its cf_name.
*/
std::shared_ptr<rocksdb::ColumnFamilyHandle> Rdb_cf_manager::get_cf(
    const std::string &cf_name_arg) const {
  return get_cf(cf_name_arg, false /*lock_held_by_caller*/);
}

std::shared_ptr<rocksdb::ColumnFamilyHandle> Rdb_cf_manager::get_cf(
    const std::string &cf_name_arg, const bool lock_held_by_caller) const {
  std::shared_ptr<rocksdb::ColumnFamilyHandle> cf_handle;

  if (!lock_held_by_caller) {
    RDB_MUTEX_LOCK_CHECK(m_mutex);
  }
  std::string cf_name = cf_name_arg.empty() ? DEFAULT_CF_NAME : cf_name_arg;

  const auto it = m_cf_name_map.find(cf_name);

  if (it != m_cf_name_map.end()) {
    cf_handle = it->second;
  }

  if (!cf_handle) {
    LogPluginErrMsg(WARNING_LEVEL, 0, "Column family '%s' not found.",
                    cf_name.c_str());
  }

  if (!lock_held_by_caller) {
    RDB_MUTEX_UNLOCK_CHECK(m_mutex);
  }

  return cf_handle;
}

std::shared_ptr<rocksdb::ColumnFamilyHandle> Rdb_cf_manager::get_cf(
    const uint32_t id) const {
  std::shared_ptr<rocksdb::ColumnFamilyHandle> cf_handle;

  RDB_MUTEX_LOCK_CHECK(m_mutex);
  const auto it = m_cf_id_map.find(id);
  if (it != m_cf_id_map.end()) cf_handle = it->second;
  RDB_MUTEX_UNLOCK_CHECK(m_mutex);

  return cf_handle;
}

std::vector<std::string> Rdb_cf_manager::get_cf_names(void) const {
  std::vector<std::string> names;

  RDB_MUTEX_LOCK_CHECK(m_mutex);
  for (const auto &it : m_cf_name_map) {
    names.push_back(it.first);
  }
  RDB_MUTEX_UNLOCK_CHECK(m_mutex);

  return names;
}

std::vector<std::shared_ptr<rocksdb::ColumnFamilyHandle>>
Rdb_cf_manager::get_all_cf(void) const {
  std::vector<std::shared_ptr<rocksdb::ColumnFamilyHandle>> list;

  RDB_MUTEX_LOCK_CHECK(m_mutex);

  for (auto it : m_cf_id_map) {
    DBUG_ASSERT(it.second != nullptr);
    list.push_back(it.second);
  }

  RDB_MUTEX_UNLOCK_CHECK(m_mutex);

  return list;
}

int Rdb_cf_manager::remove_dropped_cf(Rdb_dict_manager *const dict_manager,
                                      rocksdb::TransactionDB *const rdb,
                                      const uint32 &cf_id) {
  dict_manager->assert_lock_held();
  RDB_MUTEX_LOCK_CHECK(m_mutex);
  const std::unique_ptr<rocksdb::WriteBatch> wb = dict_manager->begin();
  rocksdb::WriteBatch *const batch = wb.get();

  const auto it = m_cf_id_map.find(cf_id);
  if (it == m_cf_id_map.end()) {
    dict_manager->delete_dropped_cf_and_flags(batch, cf_id);
    dict_manager->commit(batch);
    RDB_MUTEX_UNLOCK_CHECK(m_mutex);

    LogPluginErrMsg(WARNING_LEVEL, 0,
                    "Column family with id %u is marked as dropped, "
                    "but doesn't exist in cf manager",
                    cf_id);

    return HA_EXIT_FAILURE;
  }

  auto cf_handle = it->second.get();
  const std::string cf_name = cf_handle->GetName();

  if (!dict_manager->get_dropped_cf(cf_id)) {
    RDB_MUTEX_UNLOCK_CHECK(m_mutex);
    LogPluginErrMsg(WARNING_LEVEL, 0,
                    "Column family %s with id %u is not in "
                    "the list of cf ids to be dropped",
                    cf_name.c_str(), cf_id);
    return HA_EXIT_FAILURE;
  }

  auto status = rdb->DropColumnFamily(cf_handle);

  if (!status.ok()) {
    dict_manager->delete_dropped_cf(batch, cf_id);
    dict_manager->commit(batch);
    RDB_MUTEX_UNLOCK_CHECK(m_mutex);

    LogPluginErrMsg(ERROR_LEVEL, 0,
                    "Dropping column family %s with id %u on RocksDB failed",
                    cf_name.c_str(), cf_id);

    return ha_rocksdb::rdb_error_to_mysql(status);
  }

  DBUG_EXECUTE_IF("rocksdb_remove_dropped_cf", {
    THD *thd = new THD();
    thd->thread_stack = reinterpret_cast<char *>(&(thd));
    thd->store_globals();
    static constexpr char act[] = "now signal ready_to_restart_during_drop_cf";
    DBUG_ASSERT(!debug_sync_set_action(thd, STRING_WITH_LEN(act)));
    thd->restore_globals();
    delete thd;
  });

  auto id_iter = m_cf_id_map.find(cf_id);
  DBUG_ASSERT(id_iter != m_cf_id_map.end());
  m_cf_id_map.erase(id_iter);

  auto name_iter = m_cf_name_map.find(cf_name);
  DBUG_ASSERT(name_iter != m_cf_name_map.end());
  m_cf_name_map.erase(name_iter);

  dict_manager->delete_dropped_cf_and_flags(batch, cf_id);

  dict_manager->commit(batch);
  RDB_MUTEX_UNLOCK_CHECK(m_mutex);

  LogPluginErrMsg(INFORMATION_LEVEL, 0,
                  "Column family %s with id %u has been dropped successfully",
                  cf_name.c_str(), cf_id);

  return HA_EXIT_SUCCESS;
}

namespace {
struct Rdb_cf_scanner : public Rdb_tables_scanner {
  uint32_t m_cf_id;

  explicit Rdb_cf_scanner(uint32_t cf_id) : m_cf_id(cf_id) {}

  int add_table(Rdb_tbl_def *tdef) override {
    DBUG_ASSERT(tdef != nullptr);

    for (uint i = 0; i < tdef->m_key_count; i++) {
      const Rdb_key_def &kd = *tdef->m_key_descr_arr[i];

      if (kd.get_cf()->GetID() == m_cf_id) {
        return HA_EXIT_FAILURE;
      }
    }
    return HA_EXIT_SUCCESS;
  }
};

}  // namespace

int Rdb_cf_manager::drop_cf(Rdb_ddl_manager *const ddl_manager,
                            Rdb_dict_manager *const dict_manager,
                            const std::string &cf_name) {
  dict_manager->assert_lock_held();
  uint32_t cf_id = 0;

  if (cf_name == DEFAULT_SYSTEM_CF_NAME || cf_name == DEFAULT_CF_NAME ||
      cf_name.empty()) {
    return HA_EXIT_FAILURE;
  }

  // We should have already acquired dict manager lock. The order
  // of lock acquisition here is:
  //  dict_manager -> cf_manager -> ddl_manager

  RDB_MUTEX_LOCK_CHECK(m_mutex);
  auto cf_handle = get_cf(cf_name, true /* lock_held_by_caller */).get();
  if (cf_handle == nullptr) {
    RDB_MUTEX_UNLOCK_CHECK(m_mutex);
    LogPluginErrMsg(WARNING_LEVEL, 0,
                    "Cannot mark Column family %s to be dropped, "
                    "because it doesn't exist in cf manager",
                    cf_name.c_str());

    return HA_EXIT_FAILURE;
  }

  cf_id = cf_handle->GetID();
  Rdb_cf_scanner scanner(cf_id);

  auto ret = ddl_manager->scan_for_tables(&scanner);
  if (ret) {
    RDB_MUTEX_UNLOCK_CHECK(m_mutex);
    LogPluginErrMsg(WARNING_LEVEL, 0,
                    "Cannot mark Column family %s with id %u to be dropped, "
                    "because it is in use",
                    cf_name.c_str(), cf_id);
    return ret;
  }

  ret = ddl_manager->find_in_uncommitted_keydef(cf_id);
  if (ret) {
    RDB_MUTEX_UNLOCK_CHECK(m_mutex);
    LogPluginErrMsg(INFORMATION_LEVEL, 0,
                    "Cannot mark Column family %s with id %u to be dropped, "
                    "because it is used by an ongoing add index command",
                    cf_name.c_str(), cf_id);
    return ret;
  }

  // We don't call DropColumnFamily on RocksDB here and
  // we don't delete handle object. Here we mark the column family
  // as dropped.

  const std::unique_ptr<rocksdb::WriteBatch> wb = dict_manager->begin();
  rocksdb::WriteBatch *const batch = wb.get();

  dict_manager->add_dropped_cf(batch, cf_id);
  dict_manager->commit(batch);

  RDB_MUTEX_UNLOCK_CHECK(m_mutex);

  LogPluginErrMsg(INFORMATION_LEVEL, 0,
                  "Column family %s with id %u has been marked to be dropped",
                  cf_name.c_str(), cf_id);

  return HA_EXIT_SUCCESS;
}

int Rdb_cf_manager::create_cf_flags_if_needed(
    const Rdb_dict_manager *const dict_manager, const uint32 &cf_id,
    const std::string &cf_name, const bool is_per_partition_cf) {
  uchar flags =
      (is_cf_name_reverse(cf_name.c_str()) ? Rdb_key_def::REVERSE_CF_FLAG : 0) |
      (is_per_partition_cf ? Rdb_key_def::PER_PARTITION_CF_FLAG : 0);

  uint existing_cf_flags;
  if (dict_manager->get_cf_flags(cf_id, &existing_cf_flags)) {
    // For the purposes of comparison we'll clear the partitioning bit. The
    // intent here is to make sure that both partitioned and non-partitioned
    // tables can refer to the same CF.
    existing_cf_flags &= ~Rdb_key_def::CF_FLAGS_TO_IGNORE;
    flags &= ~Rdb_key_def::CF_FLAGS_TO_IGNORE;

    if (existing_cf_flags != flags) {
      my_error(ER_CF_DIFFERENT, MYF(0), cf_name.c_str(), flags,
               existing_cf_flags);

      return HA_EXIT_FAILURE;
    }
  } else {
    const std::unique_ptr<rocksdb::WriteBatch> wb = dict_manager->begin();
    rocksdb::WriteBatch *const batch = wb.get();

    dict_manager->add_cf_flags(batch, cf_id, flags);
    dict_manager->commit(batch);
  }

  return HA_EXIT_SUCCESS;
}

}  // namespace myrocks
