/* Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include "plugin/audit_log_filter/audit_keyring.h"

#include "plugin/audit_log_filter/audit_error_log.h"
#include "plugin/audit_log_filter/log_writer/file_handle.h"
#include "plugin/audit_log_filter/sys_vars.h"

#include <mysql/components/my_service.h>
#include <mysql/components/services/keyring_generator.h>
#include <mysql/components/services/keyring_keys_metadata_iterator.h>
#include <mysql/components/services/keyring_metadata_query.h>
#include <mysql/components/services/keyring_reader_with_status.h>
#include <mysql/components/services/keyring_writer.h>

#include <scope_guard.h>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace audit_log_filter::audit_keyring {
namespace {
inline constexpr const char *kAuthId = "audit_log";
const std::string kPasswordKeyTimestampFormat{"%Y%m%dT%H%M%S"};

using PasswordIdListEl = std::pair<ulonglong, std::string>;
using PasswordIdList = std::vector<PasswordIdListEl>;

void get_random_string(std::string &str) {
  static const char alphanumerics[] =
      "0123456789ABCDEFGHIJKLMNOPQRSTWXYZabcdefghijklmnopqrstuvwxyz";
  static const auto alphanumerics_size = sizeof(alphanumerics) - 1;
  static const auto rand_string_size = 126;

  str.clear();
  str.reserve(rand_string_size);

  for (int i = rand_string_size; i > 0; i--) {
    str.push_back(alphanumerics[random() % alphanumerics_size]);
  }
}

bool get_keyring_password_key_list_sorted(PasswordIdList &list) {
  list.clear();

  my_service<SERVICE_TYPE(keyring_keys_metadata_iterator)> iterator_srv(
      "keyring_keys_metadata_iterator", SysVars::get_comp_regystry_srv());

  if (!iterator_srv.is_valid()) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init keyring iterator service");
    return false;
  }

  my_h_keyring_keys_metadata_iterator forward_iterator = nullptr;

  if (iterator_srv->init(&forward_iterator)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init keyring iterator");
    return false;
  }

  auto cleanup_object = create_scope_guard([&] {
    if (forward_iterator != nullptr) {
      iterator_srv->deinit(forward_iterator);
    }
    forward_iterator = nullptr;
  });

  std::string data_id;
  std::string auth_id;
  bool is_iter_valid = iterator_srv->is_valid(forward_iterator);

  const std::regex timestamp_regex(R"(.*-(\d{8}T\d{6})-.*)");
  auto time_now = std::time(nullptr);

  DBUG_EXECUTE_IF("audit_log_filter_debug_timestamp", {
    time_now = std::chrono::time_point_cast<std::chrono::seconds>(
                   SysVars::get_debug_time_point_for_encryption())
                   .time_since_epoch()
                   .count();
  });

  while (is_iter_valid) {
    data_id.assign(1024, '\0');
    auth_id.assign(1024, '\0');

    if (iterator_srv->get(forward_iterator, data_id.data(), 1024,
                          auth_id.data(), 1024) == true) {
      break;
    }

    std::smatch pieces_match;
    ulonglong password_age_days = 0;

    if (std::regex_match(data_id, pieces_match, timestamp_regex)) {
      std::tm tm{};
      std::istringstream ss(pieces_match[1].str());
      ss >> std::get_time(&tm, kPasswordKeyTimestampFormat.c_str());
      tm.tm_isdst = -1;
      password_age_days = (time_now - timelocal(&tm)) / (60 * 60 * 24);
    }

    list.emplace_back(password_age_days, data_id);
    is_iter_valid = !iterator_srv->next(forward_iterator);
  }

  std::make_heap(list.begin(), list.end(), [](const auto &a, const auto &b) {
    return a.first < b.first;
  });

  return true;
}

bool get_active_keyring_password_key(std::string &password_id) {
  PasswordIdList id_list;

  if (!get_keyring_password_key_list_sorted(id_list)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to fetch password ids");
    return false;
  }

  if (!id_list.empty()) {
    password_id = id_list.back().second;
  }

  return true;
}

bool get_keyring_password(const std::string &password_id,
                          std::string &password) {
  my_service<SERVICE_TYPE(keyring_reader_with_status)> reader_srv(
      "keyring_reader_with_status", SysVars::get_comp_regystry_srv());

  if (!reader_srv.is_valid()) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to create keyring reader service");
    return false;
  }

  my_h_keyring_reader_object reader_object = nullptr;

  if (reader_srv->init(password_id.c_str(), kAuthId, &reader_object)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init keyring reader service");
    return false;
  }

  auto cleanup_object = create_scope_guard([&] {
    if (reader_object != nullptr) {
      reader_srv->deinit(reader_object);
      reader_object = nullptr;
    }
  });

  if (reader_object == nullptr) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "No data found for key '%s'", password_id.c_str());
    return false;
  }

  size_t data_length = 0;
  size_t data_type_length = 0;

  if (reader_srv->fetch_length(reader_object, &data_length,
                               &data_type_length)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "No keyring data found");
    return false;
  }

  auto data_buffer = std::make_unique<unsigned char[]>(data_length);
  auto data_type_buffer = std::make_unique<char[]>(data_type_length + 1);

  if (data_buffer.get() == nullptr || data_type_buffer.get() == nullptr) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to fetch keyring data");
    return false;
  }

  memset(data_buffer.get(), 0, data_length);
  memset(data_type_buffer.get(), 0, data_type_length + 1);

  size_t fetched_data_length = 0;
  size_t fetched_data_type_length = 0;

  if (reader_srv->fetch(reader_object, data_buffer.get(), data_length,
                        &fetched_data_length, data_type_buffer.get(),
                        data_type_length, &fetched_data_type_length)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to fetch keyring data");
    return true;
  }

  password = reinterpret_cast<char *>(data_buffer.get());

  return true;
}

bool generate_keyring_password_id(std::string &password_id) {
  my_service<SERVICE_TYPE(keyring_reader_with_status)> reader_srv(
      "keyring_reader_with_status", SysVars::get_comp_regystry_srv());

  if (!reader_srv.is_valid()) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to create keyring reader service");
    return false;
  }

  my_h_keyring_reader_object reader_object = nullptr;
  std::stringstream id_prefix;
  std::time_t t =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  uint key_seq = 1;
  bool is_generated = false;

  DBUG_EXECUTE_IF("audit_log_filter_debug_timestamp", {
    t = std::chrono::system_clock::to_time_t(
        SysVars::get_debug_time_point_for_encryption());
  });

  while (!is_generated) {
    id_prefix.str("");
    id_prefix << kAuthId << "-"
              << std::put_time(std::localtime(&t), "%Y%m%dT%H%M%S") << "-"
              << key_seq;

    if (reader_srv->init(id_prefix.str().c_str(), kAuthId, &reader_object)) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Failed to init keyring reader service");
      return false;
    }

    if (reader_object == nullptr) {
      // Found free ID
      is_generated = true;
    } else {
      ++key_seq;
    }

    reader_srv->deinit(reader_object);
    reader_object = nullptr;
  }

  password_id = id_prefix.str();

  return true;
}

bool set_keyring_password(const std::string &password_id,
                          const std::string &password) {
  my_service<SERVICE_TYPE(keyring_writer)> writer_srv(
      "keyring_writer", SysVars::get_comp_regystry_srv());

  if (!writer_srv.is_valid()) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init keyring writer service");
    return false;
  }

  if (writer_srv->store(
          password_id.c_str(), kAuthId,
          reinterpret_cast<const unsigned char *>(password.c_str()),
          password.length() + 1, "AES")) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to store keyring data");
    return false;
  }

  return true;
}

bool generate_keyring_password(std::string &password_id) {
  if (!generate_keyring_password_id(password_id)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to generate password ID");
    return false;
  }

  std::string password;
  get_random_string(password);

  return set_keyring_password(password_id, password);
}

}  // namespace

bool check_keyring_initialized() noexcept {
  my_service<SERVICE_TYPE(keyring_component_status)> component_status(
      "keyring_component_status", SysVars::get_comp_regystry_srv());

  if (!component_status.is_valid()) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init keyring status service");
    return false;
  }

  return component_status->is_initialized();
}

bool check_generate_initial_password() noexcept {
  std::string password_id;

  if (!get_active_keyring_password_key(password_id)) {
    return false;
  }

  if (password_id.empty() && !generate_keyring_password(password_id)) {
    return false;
  }

  SysVars::set_encryption_password_id(password_id);

  return true;
}

bool get_encryption_password(std::string &password) noexcept {
  password.clear();
  std::string password_id;

  if (!get_active_keyring_password_key(password_id)) {
    return false;
  }

  return get_encryption_password(password_id, password);
}

bool get_encryption_password(const std::string &password_id,
                             std::string &password) noexcept {
  password.clear();

  if (password_id.empty() || !get_keyring_password(password_id, password)) {
    return false;
  }

  return true;
}

bool set_encryption_password(const std::string &password) noexcept {
  std::string password_id;

  if (!generate_keyring_password_id(password_id)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to generate password ID");
    return false;
  }

  if (!set_keyring_password(password_id, password)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "Failed to set password");
    return false;
  }

  SysVars::set_encryption_password_id(password_id);

  return true;
}

void prune_encryption_passwords(
    uint64_t remove_after_days,
    const std::vector<std::string> &existing_log_names) noexcept {
  PasswordIdList id_list;

  if (!get_keyring_password_key_list_sorted(id_list)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to fetch password ids");
    return;
  }

  if (id_list.empty()) {
    return;
  }

  my_service<SERVICE_TYPE(keyring_writer)> writer_srv(
      "keyring_writer", SysVars::get_comp_regystry_srv());

  if (!writer_srv.is_valid()) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init keyring writer service");
    return;
  }

  const std::regex file_pwd_id_regex(R"(.*\.(\d{8}T\d{6}-\d+)\.enc)");
  const std::regex keyring_pwd_id_regex(R"(.*\-(\d{8}T\d{6}\-\d+).*)");

  std::unordered_set<std::string> used_password_ids;

  for (const auto &file_name : existing_log_names) {
    std::smatch file_match;
    if (std::regex_match(file_name, file_match, file_pwd_id_regex)) {
      used_password_ids.insert(file_match[1].str());
    }
  }

  for (const auto &el : id_list) {
    std::smatch keyring_match;
    if (!std::regex_match(el.second, keyring_match, keyring_pwd_id_regex) ||
        used_password_ids.count(keyring_match[1].str())) {
      // password_id is still used by some encrypted log
      // or failed to match key_id
      continue;
    }

    if (el.first < remove_after_days) {
      break;
    }

    if (writer_srv->remove(el.second.c_str(), kAuthId)) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Failed to remove password with ID: %s",
                      el.second.c_str());
    }
  }
}

std::string get_password_id_timestamp(const std::string &password_id) noexcept {
  return password_id.substr(strlen(kAuthId) + 1);
}
}  // namespace audit_log_filter::audit_keyring
