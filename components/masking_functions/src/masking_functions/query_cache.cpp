/* Copyright (c) 2024 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA */

#include "masking_functions/query_cache.hpp"

#include <chrono>
#include <string_view>

#include <mysql/components/services/log_builtins.h>
#include <mysql/psi/mysql_thread.h>
#include <mysqld_error.h>
#include <sql/debug_sync.h>
#include <sql/sql_class.h>

#include "masking_functions/bookshelf.hpp"
#include "masking_functions/command_service_tuple.hpp"
#include "masking_functions/primitive_singleton.hpp"
#include "masking_functions/query_builder.hpp"
#include "masking_functions/sql_context.hpp"
#include "masking_functions/sys_vars.hpp"

extern REQUIRES_SERVICE_PLACEHOLDER(log_builtins);

namespace {

using global_command_services = masking_functions::primitive_singleton<
    masking_functions::command_service_tuple>;

constexpr const char psi_category_name[]{"masking_functions"};
constexpr const char flusher_thd_psi_name[]{"masking_functions_dict_flusher"};
constexpr const char flusher_thd_psi_os_name[]{"mf_flusher"};

}  // anonymous namespace

namespace masking_functions {

query_cache::query_cache(query_builder_ptr query_builder,
                         std::uint64_t flusher_interval_seconds)
    : dict_query_builder_{std::move(query_builder)},
      dict_cache_{},
      dict_cache_mutex_{},
      flusher_interval_seconds_{flusher_interval_seconds},
      is_flusher_stopped_{true} {
  // we do not initialize m_dict_cache with create_dict_cache_internal() here
  // as this constructor is called from the component initialization method
  // and any call to mysql_command_query service may mess up with current THD

  // the cache will be loaded during the first call to one of the dictionary
  // functions or by the flusher thread
  if (flusher_interval_seconds_ > 0) {
    PSI_thread_info thread_info{&psi_flusher_thread_key_,
                                flusher_thd_psi_name,
                                flusher_thd_psi_os_name,
                                PSI_FLAG_SINGLETON,
                                0,
                                PSI_DOCUMENT_ME};
    mysql_thread_register(psi_category_name, &thread_info, 1);

    const auto res =
        mysql_thread_create(psi_flusher_thread_key_, &flusher_thread_,
                            &flusher_thread_attr_, run_dict_flusher, this);

    if (res != 0) {
      LogComponentErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG,
                      "Cannot initialize dictionary flusher");
    } else {
      is_flusher_stopped_ = false;
    }
  }
}

query_cache::~query_cache() {
  if (!is_flusher_stopped_) {
    is_flusher_stopped_ = true;
    flusher_condition_var_.notify_one();
  }
}

bool query_cache::contains(const std::string &dictionary_name,
                           const std::string &term) const {
  shared_lock_type read_lock{};
  unique_lock_type write_lock{};
  const auto &acquired_dict_cache{
      acquire_dict_cache_shared(read_lock, write_lock)};
  return acquired_dict_cache.contains(dictionary_name, term);
}

std::string query_cache::get_random(const std::string &dictionary_name) const {
  shared_lock_type read_lock{};
  unique_lock_type write_lock{};
  const auto &acquired_dict_cache{
      acquire_dict_cache_shared(read_lock, write_lock)};
  return std::string{acquired_dict_cache.get_random(dictionary_name)};
}

bool query_cache::remove(const std::string &dictionary_name) {
  masking_functions::sql_context sql_ctx{global_command_services::instance()};
  auto query{dict_query_builder_->delete_for_dictionary(dictionary_name)};

  unique_lock_type write_lock{};
  auto &acquired_dict_cache{acquire_dict_cache_unique(write_lock)};

  // there is a chance that a user can delete the dictionary from the
  // dictionary table directly (not via UDF function) and execute_dml()
  // will return false here, whereas cache operation will return true -
  // this is why we rely only on the result of the cache operation
  sql_ctx.execute_dml(query);
  return acquired_dict_cache.remove(dictionary_name);
}

bool query_cache::remove(const std::string &dictionary_name,
                         const std::string &term) {
  masking_functions::sql_context sql_ctx{global_command_services::instance()};
  auto query{dict_query_builder_->delete_for_dictionary_and_term(
      dictionary_name, term)};

  unique_lock_type write_lock{};
  auto &acquired_dict_cache{acquire_dict_cache_unique(write_lock)};

  // similarly to another remove() method, we ignore the result of the
  // sql operation and rely only on the result of the cache modification
  sql_ctx.execute_dml(query);
  return acquired_dict_cache.remove(dictionary_name, term);
}

bool query_cache::insert(const std::string &dictionary_name,
                         const std::string &term) {
  masking_functions::sql_context sql_ctx{global_command_services::instance()};
  auto query{dict_query_builder_->insert_ignore_record(dictionary_name, term)};

  unique_lock_type write_lock{};
  auto &acquired_dict_cache{acquire_dict_cache_unique(write_lock)};

  // here, as cache insert may throw, we start the 2-phase operation
  // with this cache insert because it can be easily reversed without throwing
  const auto result{acquired_dict_cache.insert(dictionary_name, term)};
  try {
    sql_ctx.execute_dml(query);
  } catch (...) {
    dict_cache_->remove(dictionary_name, term);
    throw;
  }

  return result;
}

void query_cache::reload_cache() {
  unique_lock_type dict_cache_write_lock{dict_cache_mutex_};

  auto local_dict_cache{create_dict_cache_internal()};
  if (!local_dict_cache) {
    throw std::runtime_error{"Cannot load dictionary cache"};
  }

  dict_cache_ = std::move(local_dict_cache);
}

void query_cache::init_thd() noexcept {
  auto *thd = new THD;
  my_thread_init();
  thd->set_new_thread_id();
  thd->thread_stack = reinterpret_cast<char *>(&thd);
  thd->store_globals();
  flusher_thd_.reset(thd);
}

void query_cache::release_thd() noexcept { my_thread_end(); }

void query_cache::dict_flusher() noexcept {
#ifdef HAVE_PSI_THREAD_INTERFACE
  {
    struct PSI_thread *psi = flusher_thd_->get_psi();
    PSI_THREAD_CALL(set_thread_id)(psi, flusher_thd_->thread_id());
    PSI_THREAD_CALL(set_thread_THD)(psi, flusher_thd_.get());
    PSI_THREAD_CALL(set_thread_command)(flusher_thd_->get_command());
    PSI_THREAD_CALL(set_thread_info)
    (STRING_WITH_LEN("Masking functions component cache flusher"));
  }
#endif

  while (!is_flusher_stopped_) {
    std::unique_lock lock{flusher_mutex_};
    const auto wait_started_at = std::chrono::system_clock::now();
    flusher_condition_var_.wait_for(
        lock, std::chrono::seconds{flusher_interval_seconds_},
        [this, wait_started_at] {
          return std::chrono::duration_cast<std::chrono::seconds>(
                     std::chrono::system_clock::now() - wait_started_at) >=
                     std::chrono::seconds{flusher_interval_seconds_} ||
                 is_flusher_stopped_.load();
        });

    if (!is_flusher_stopped_) {
      {
        unique_lock_type dict_cache_write_lock{dict_cache_mutex_};
        auto local_dict_cache{create_dict_cache_internal()};
        if (local_dict_cache) {
          dict_cache_ = std::move(local_dict_cache);
        }
      }

      DBUG_EXECUTE_IF("masking_functions_signal_on_cache_reload", {
        const char act[] = "now SIGNAL masking_functions_cache_reload_done";
        assert(!debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
      };);
    }
  }
}

void *query_cache::run_dict_flusher(void *arg) {
  auto *self = reinterpret_cast<masking_functions::query_cache *>(arg);
  self->init_thd();
  self->dict_flusher();
  self->release_thd();
  return nullptr;
}

bookshelf_ptr query_cache::create_dict_cache_internal() const {
  bookshelf_ptr result;
  try {
    masking_functions::sql_context sql_ctx{global_command_services::instance()};
    auto query{dict_query_builder_->select_all_from_dictionary()};
    auto local_dict_cache{std::make_unique<bookshelf>()};
    sql_context::row_callback<2> result_inserter{[&terms = *local_dict_cache](
                                                     const auto &field_values) {
      terms.insert(std::string{field_values[0]}, std::string{field_values[1]});
    }};
    sql_ctx.execute_select(query, result_inserter);
    result = std::move(local_dict_cache);
  } catch (...) {
  }

  return result;
}

const bookshelf &query_cache::acquire_dict_cache_shared(
    shared_lock_type &read_lock, unique_lock_type &write_lock) const {
  read_lock = shared_lock_type{dict_cache_mutex_};
  if (!dict_cache_) {
    // upgrading to a unique_lock
    read_lock.unlock();
    acquire_dict_cache_unique(write_lock);
  }
  return *dict_cache_;
}

bookshelf &query_cache::acquire_dict_cache_unique(
    unique_lock_type &write_lock) const {
  write_lock = unique_lock_type{dict_cache_mutex_};
  if (!dict_cache_) {
    auto local_dict_cache{create_dict_cache_internal()};
    if (!local_dict_cache) {
      throw std::runtime_error{"Cannot load dictionary cache"};
    }
    dict_cache_ = std::move(local_dict_cache);
  }
  return *dict_cache_;
}

}  // namespace masking_functions
