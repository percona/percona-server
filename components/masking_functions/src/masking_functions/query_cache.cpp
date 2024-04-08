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

#include "masking_functions/command_service_tuple.hpp"
#include "masking_functions/primitive_singleton.hpp"
#include "masking_functions/query_builder.hpp"
#include "masking_functions/sql_context.hpp"
#include "masking_functions/sys_vars.hpp"

#include <mysql/components/services/log_builtins.h>
#include <mysql/psi/mysql_thread.h>
#include <mysqld_error.h>
#include <sql/debug_sync.h>
#include <sql/sql_class.h>

#include <chrono>
#include <string_view>

extern REQUIRES_SERVICE_PLACEHOLDER(log_builtins);

namespace masking_functions {
namespace {

constexpr std::string_view psi_category_name{"masking_functions"};
constexpr std::string_view flusher_thd_psi_name{
    "masking_functions_dict_flusher"};
constexpr std::string_view flusher_thd_psi_os_name{"mf_flusher"};

using global_command_services = masking_functions::primitive_singleton<
    masking_functions::command_service_tuple>;
using global_query_builder =
    masking_functions::primitive_singleton<masking_functions::query_builder>;

void *run_dict_flusher(void *arg) {
  auto *self = reinterpret_cast<masking_functions::query_cache *>(arg);
  self->init_thd();
  self->dict_flusher();
  self->release_thd();
  return nullptr;
}

}  // namespace

query_cache::query_cache()
    : m_flusher_interval_seconds{sys_vars::get_flush_interval_seconds()},
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

bool query_cache::load_cache() {
  auto query = global_query_builder::instance().select_all_from_dictionary();
  auto result =
      masking_functions::sql_context{global_command_services::instance()}
          .query_list(query);

  if (result.has_value()) {
    std::unique_lock dict_write_lock{m_dict_mut};
    m_dict_cache = std::move(result.value());
  }

  return result.has_value();
}

bool query_cache::contains(const std::string &dictionary_name,
                           const std::string &term) const {
  std::shared_lock dict_read_lock{m_dict_mut};
  return m_dict_cache.contains(dictionary_name, term);
}

optional_string query_cache::get(const std::string &dictionary_name) const {
  std::shared_lock dict_read_lock{m_dict_mut};
  return m_dict_cache.get(dictionary_name);
}

bool query_cache::remove(const std::string &dictionary_name) {
  std::unique_lock dict_write_lock{m_dict_mut};

  masking_functions::sql_context sql_ctx{global_command_services::instance()};
  auto query =
      global_query_builder::instance().delete_for_dictionary(dictionary_name);

  if (!sql_ctx.execute(query)) {
    return false;
  }

  return m_dict_cache.remove(dictionary_name);
}

bool query_cache::remove(const std::string &dictionary_name,
                         const std::string &term) {
  std::shared_lock dict_read_lock{m_dict_mut};

  masking_functions::sql_context sql_ctx{global_command_services::instance()};
  auto query = global_query_builder::instance().delete_for_dictionary_and_term(
      dictionary_name, term);

  if (!sql_ctx.execute(query)) {
    return false;
  }

  return m_dict_cache.remove(dictionary_name, term);
}

bool query_cache::insert(const std::string &dictionary_name,
                         const std::string &term) {
  std::unique_lock dict_write_lock{m_dict_mut};

  masking_functions::sql_context sql_ctx{global_command_services::instance()};
  auto query = global_query_builder::instance().insert_ignore_record(
      dictionary_name, term);

  if (!sql_ctx.execute(query)) {
    return false;
  }

  return m_dict_cache.insert(dictionary_name, term);
}

}  // namespace masking_functions
