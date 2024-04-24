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

constexpr std::string_view psi_category_name{"masking_functions"};
constexpr std::string_view flusher_thd_psi_name{
    "masking_functions_dict_flusher"};
constexpr std::string_view flusher_thd_psi_os_name{"mf_flusher"};

}  // anonymous namespace

namespace masking_functions {

query_cache::query_cache(query_builder_ptr query_builder,
                         std::uint64_t flusher_interval_seconds)
    : m_query_builder{std::move(query_builder)},
      m_flusher_interval_seconds{flusher_interval_seconds},
      m_is_flusher_stopped{true} {
  load_cache();

  if (m_flusher_interval_seconds > 0) {
    PSI_thread_info thread_info{&m_psi_flusher_thread_key,
                                flusher_thd_psi_name.data(),
                                flusher_thd_psi_os_name.data(),
                                PSI_FLAG_SINGLETON,
                                0,
                                PSI_DOCUMENT_ME};
    mysql_thread_register(psi_category_name.data(), &thread_info, 1);

    const auto res =
        mysql_thread_create(m_psi_flusher_thread_key, &m_flusher_thread,
                            &m_flusher_thread_attr, run_dict_flusher, this);

    if (res != 0) {
      LogComponentErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG,
                      "Cannot initialize dictionary flusher");
    } else {
      m_is_flusher_stopped = false;
    }
  }
}

query_cache::~query_cache() {
  if (!m_is_flusher_stopped) {
    m_is_flusher_stopped = true;
    m_flusher_condition_var.notify_one();
  }
}

bool query_cache::load_cache() {
  masking_functions::sql_context sql_ctx{global_command_services::instance()};
  auto query = m_query_builder->select_all_from_dictionary();
  auto result = sql_ctx.query_list(query);

  if (result) {
    // TODO: in c++20 change to m_dict_cache to std::atomic<bookshelf_ptr>
    std::atomic_store(&m_dict_cache, result);
  }

  return static_cast<bool>(result);
}

bool query_cache::contains(const std::string &dictionary_name,
                           const std::string &term) const {
  return m_dict_cache->contains(dictionary_name, term);
}

optional_string query_cache::get_random(
    const std::string &dictionary_name) const {
  return m_dict_cache->get_random(dictionary_name);
}

bool query_cache::remove(const std::string &dictionary_name) {
  masking_functions::sql_context sql_ctx{global_command_services::instance()};
  auto query = m_query_builder->delete_for_dictionary(dictionary_name);

  if (!sql_ctx.execute(query)) {
    return false;
  }

  return m_dict_cache->remove(dictionary_name);
}

bool query_cache::remove(const std::string &dictionary_name,
                         const std::string &term) {
  masking_functions::sql_context sql_ctx{global_command_services::instance()};
  auto query =
      m_query_builder->delete_for_dictionary_and_term(dictionary_name, term);

  if (!sql_ctx.execute(query)) {
    return false;
  }

  return m_dict_cache->remove(dictionary_name, term);
}

bool query_cache::insert(const std::string &dictionary_name,
                         const std::string &term) {
  masking_functions::sql_context sql_ctx{global_command_services::instance()};
  auto query = m_query_builder->insert_ignore_record(dictionary_name, term);

  if (!sql_ctx.execute(query)) {
    return false;
  }

  return m_dict_cache->insert(dictionary_name, term);
}

void query_cache::init_thd() noexcept {
  auto *thd = new THD;
  my_thread_init();
  thd->set_new_thread_id();
  thd->thread_stack = reinterpret_cast<char *>(&thd);
  thd->store_globals();
  m_flusher_thd.reset(thd);
}

void query_cache::release_thd() noexcept { my_thread_end(); }

void query_cache::dict_flusher() noexcept {
#ifdef HAVE_PSI_THREAD_INTERFACE
  {
    struct PSI_thread *psi = m_flusher_thd->get_psi();
    PSI_THREAD_CALL(set_thread_id)(psi, m_flusher_thd->thread_id());
    PSI_THREAD_CALL(set_thread_THD)(psi, m_flusher_thd.get());
    PSI_THREAD_CALL(set_thread_command)(m_flusher_thd->get_command());
    PSI_THREAD_CALL(set_thread_info)
    (STRING_WITH_LEN("Masking functions component cache flusher"));
  }
#endif

  while (!m_is_flusher_stopped) {
    std::unique_lock lock{m_flusher_mutex};
    const auto wait_started_at = std::chrono::system_clock::now();
    m_flusher_condition_var.wait_for(
        lock, std::chrono::seconds{m_flusher_interval_seconds},
        [this, wait_started_at] {
          return std::chrono::duration_cast<std::chrono::seconds>(
                     std::chrono::system_clock::now() - wait_started_at) >=
                     std::chrono::seconds{m_flusher_interval_seconds} ||
                 m_is_flusher_stopped.load();
        });

    if (!m_is_flusher_stopped) {
      load_cache();

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

}  // namespace masking_functions
