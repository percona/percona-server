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

#include "components/audit_log_filter/audit_keyring.h"

#include "components/audit_log_filter/audit_error_log.h"
#include "components/audit_log_filter/audit_password.h"
#include "components/audit_log_filter/log_writer/file_handle.h"
#include "components/audit_log_filter/sys_vars.h"

#include "my_dbug.h"
#include "mysql/my_loglevel.h"

#include <mysql/components/my_service.h>
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
#include <limits>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include <time.h>  // timelocal(), timegm()

namespace audit_log_filter::audit_keyring {
namespace {
inline constexpr const char *kAuthId = "audit_log";
const std::string kOptionsKeyTimestampFormat{"%Y%m%dT%H%M%S"};

constexpr auto file_opt_id_pattern(R"(.*\.(\d{8}T\d{6}-\d+)\.enc)");
constexpr auto keyring_opt_id_pattern(R"(.*\-(\d{8}T\d{6}\-\d+).*)");

struct OptionInfo {
  std::string timestamp;
  ulonglong seq_num;
  ulonglong data_age_days;
  std::string data_id;
};

using OptionsIdList = std::vector<OptionInfo>;

/**
 * @brief Get the timestamp and the sequence number of an options ID
 *
 * The timestamp is the local time the ID was made at, as it is written in
 * the ID. The IDs are ordered by it and by the sequence number then. The
 * time since the epoch is not used for the order: a local time is repeated
 * when the daylight saving time ends, and its conversion depends on the
 * conversions made before it then.
 *
 * @param [in] options_id Options ID
 * @param [out] timestamp Timestamp of the ID
 * @param [out] seq_num Sequence number of the ID
 * @return true in case of success, false if options_id is not an options ID
 */
bool parse_keyring_options_id(const std::string &options_id,
                              std::string &timestamp, ulonglong &seq_num) {
  static const std::regex timestamp_regex(R"(.*-(\d{8}T\d{6})-(\d*).*)");
  std::smatch pieces_match;

  if (!std::regex_match(options_id, pieces_match, timestamp_regex)) {
    return false;
  }

  timestamp = pieces_match[1].str();
  seq_num = std::stoull(pieces_match[2].str());

  return true;
}

/**
 * @brief Get the time since the epoch of a timestamp of an options ID
 *
 * @param [in] timestamp Timestamp of an options ID, a local time
 * @return Seconds since the epoch
 */
ulonglong get_keyring_options_timestamp_time(const std::string &timestamp) {
  std::tm tm{};
  std::istringstream ss(timestamp);
  ss >> std::get_time(&tm, kOptionsKeyTimestampFormat.c_str());
  tm.tm_isdst = -1;
  return static_cast<ulonglong>(timelocal(&tm));
}

/**
 * @brief Get the timestamp of an options ID that is one second after
 *        the given one
 *
 * The timestamps are handled as UTC times here, so that the result is after
 * the given timestamp in the order of the IDs for any local time.
 *
 * @param [in] timestamp Timestamp of an options ID
 * @return Timestamp of the next second
 */
std::string get_next_keyring_options_timestamp(const std::string &timestamp) {
  std::tm tm{};
  std::istringstream ss(timestamp);
  ss >> std::get_time(&tm, kOptionsKeyTimestampFormat.c_str());
  std::time_t t = timegm(&tm) + 1;
  gmtime_r(&t, &tm);
  std::ostringstream next_timestamp;
  next_timestamp << std::put_time(&tm, kOptionsKeyTimestampFormat.c_str());
  return next_timestamp.str();
}

/**
 * @brief Check if an options ID is ordered before another one
 *
 * @param [in] options_id Options ID
 * @param [in] other_options_id Options ID to compare with
 * @return true if options_id is before other_options_id or if only
 *         other_options_id is an options ID, false otherwise
 */
bool is_keyring_options_id_before(const std::string &options_id,
                                  const std::string &other_options_id) {
  std::string timestamp;
  std::string other_timestamp;
  ulonglong seq_num = 0;
  ulonglong other_seq_num = 0;

  if (!parse_keyring_options_id(other_options_id, other_timestamp,
                                other_seq_num)) {
    return false;
  }

  if (!parse_keyring_options_id(options_id, timestamp, seq_num)) {
    return true;
  }

  return (timestamp == other_timestamp) ? seq_num < other_seq_num
                                        : timestamp < other_timestamp;
}

/**
 * @brief Get the options IDs stored in the keyring, ordered from the earliest
 *        to the latest one
 *
 * @param [out] list Options IDs
 * @param [in] with_age Get the age of the options too, it is zero otherwise
 * @return true in case of success, false otherwise
 */
bool get_keyring_options_key_list_sorted(OptionsIdList &list,
                                         bool with_age = true) {
  list.clear();

  my_service<SERVICE_TYPE(keyring_keys_metadata_iterator)> iterator_srv(
      "keyring_keys_metadata_iterator", SysVars::get_comp_registry_srv());

  if (!iterator_srv.is_valid()) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init keyring iterator service");
    return false;
  }

  my_h_keyring_keys_metadata_iterator forward_iterator = nullptr;

  if (iterator_srv->init(&forward_iterator)) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
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

  auto time_now = with_age ? std::time(nullptr) : 0;

  if (with_age) {
    DBUG_EXECUTE_IF("audit_log_filter_debug_timestamp", {
      time_now = std::chrono::time_point_cast<std::chrono::seconds>(
                     SysVars::get_debug_time_point_for_encryption())
                     .time_since_epoch()
                     .count();
    });
  }

  while (is_iter_valid) {
    data_id.assign(1024, '\0');
    auth_id.assign(1024, '\0');

    if (iterator_srv->get(forward_iterator, data_id.data(), 1024,
                          auth_id.data(), 1024) == true) {
      break;
    }

    if (auth_id.find(kAuthId) != std::string::npos) {
      std::string timestamp;
      ulonglong seq_num = 0;

      if (parse_keyring_options_id(data_id, timestamp, seq_num)) {
        // The ID is ahead of the clock after the clock was set back
        const auto time_now_ull = static_cast<ulonglong>(time_now);
        const auto timestamp_time =
            get_keyring_options_timestamp_time(timestamp);
        const ulonglong data_age_days =
            timestamp_time < time_now_ull
                ? (time_now_ull - timestamp_time) / (60 * 60 * 24)
                : 0;

        list.push_back({timestamp, seq_num, data_age_days, data_id});
      }
    }

    is_iter_valid = !iterator_srv->next(forward_iterator);
  }

  std::sort(list.begin(), list.end(), [](const auto &a, const auto &b) {
    return (a.timestamp == b.timestamp) ? a.seq_num < b.seq_num
                                        : a.timestamp < b.timestamp;
  });

  return true;
}

bool get_active_keyring_options_key(std::string &options_id,
                                    bool with_age = true) {
  OptionsIdList id_list;

  if (!get_keyring_options_key_list_sorted(id_list, with_age)) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to fetch options ids");
    return false;
  }

  if (!id_list.empty()) {
    options_id = id_list.back().data_id;
  }

  return true;
}

bool get_keyring_options(const std::string &options_id,
                         std::string &options_json_str) {
  my_service<SERVICE_TYPE(keyring_reader_with_status)> reader_srv(
      "keyring_reader_with_status", SysVars::get_comp_registry_srv());

  if (!reader_srv.is_valid()) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to create keyring reader service");
    return false;
  }

  my_h_keyring_reader_object reader_object = nullptr;

  if (reader_srv->init(options_id.c_str(), kAuthId, &reader_object)) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
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
    LogComponentErr(ERROR_LEVEL, ER_AUDIT_KEYRING_KEY_NOT_FOUND,
                    options_id.c_str());
    return false;
  }

  size_t data_length = 0;
  size_t data_type_length = 0;

  if (reader_srv->fetch_length(reader_object, &data_length,
                               &data_type_length)) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "No keyring data found");
    return false;
  }

  auto data_buffer = std::make_unique<unsigned char[]>(data_length);
  auto data_type_buffer = std::make_unique<char[]>(data_type_length + 1);

  if (data_buffer.get() == nullptr || data_type_buffer.get() == nullptr) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
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
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to fetch keyring data");
    return true;
  }

  options_json_str =
      std::string{reinterpret_cast<char *>(data_buffer.get()), data_length};

  return true;
}

/**
 * @brief Generate an ID for new options
 *
 * The options with the latest ID become the active ones when the component
 * starts, see get_active_keyring_options_key(). The ID is built from the
 * current local time, which is behind the latest ID after the clock or the
 * time zone was changed, or after the daylight saving time ended. The new ID
 * takes the timestamp of the latest one then and the sequence number after
 * it.
 *
 * @param [out] options_id Generated ID
 * @param [in] latest_options_id ID the new one has to follow, may be empty
 * @return true in case of success, false otherwise
 */
bool generate_keyring_options_id(std::string &options_id,
                                 const std::string &latest_options_id) {
  my_service<SERVICE_TYPE(keyring_reader_with_status)> reader_srv(
      "keyring_reader_with_status", SysVars::get_comp_registry_srv());

  if (!reader_srv.is_valid()) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to create keyring reader service");
    return false;
  }

  my_h_keyring_reader_object reader_object = nullptr;
  std::stringstream id_prefix;
  std::time_t t =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  ulonglong key_seq = 1;
  bool is_generated = false;

  DBUG_EXECUTE_IF("audit_log_filter_debug_timestamp", {
    t = std::chrono::system_clock::to_time_t(
        SysVars::get_debug_time_point_for_encryption());
  });

  std::ostringstream local_timestamp;
  local_timestamp << std::put_time(std::localtime(&t),
                                   kOptionsKeyTimestampFormat.c_str());
  std::string timestamp = local_timestamp.str();

  const auto build_id = [&id_prefix, &timestamp, &key_seq] {
    id_prefix.str("");
    id_prefix << kAuthId << "-" << timestamp << "-" << key_seq;
  };

  // The largest sequence number is followed by the first one of the next
  // second
  const auto next_key_seq = [&timestamp, &key_seq] {
    if (key_seq == std::numeric_limits<ulonglong>::max()) {
      timestamp = get_next_keyring_options_timestamp(timestamp);
      key_seq = 1;
    } else {
      ++key_seq;
    }
  };

  std::string latest_timestamp;
  ulonglong latest_seq_num = 0;

  if (parse_keyring_options_id(latest_options_id, latest_timestamp,
                               latest_seq_num) &&
      timestamp <= latest_timestamp) {
    timestamp = latest_timestamp;
    key_seq = latest_seq_num;
    next_key_seq();
  }

  while (!is_generated) {
    build_id();

    if (reader_srv->init(id_prefix.str().c_str(), kAuthId, &reader_object)) {
      LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "Failed to init keyring reader service");
      return false;
    }

    if (reader_object == nullptr) {
      // Found free ID
      is_generated = true;
    } else {
      next_key_seq();
    }

    reader_srv->deinit(reader_object);
    reader_object = nullptr;
  }

  options_id = id_prefix.str();

  return true;
}

bool set_keyring_options(const std::string &options_id,
                         const std::string &options_json_str) {
  my_service<SERVICE_TYPE(keyring_writer)> writer_srv(
      "keyring_writer", SysVars::get_comp_registry_srv());

  if (!writer_srv.is_valid()) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init keyring writer service");
    return false;
  }

  DBUG_EXECUTE_IF("audit_log_filter_fail_keyring_store", {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to store keyring data");
    return false;
  });

  if (writer_srv->store(
          options_id.c_str(), kAuthId,
          reinterpret_cast<const unsigned char *>(options_json_str.c_str()),
          options_json_str.length(), "SECRET")) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to store keyring data");
    return false;
  }

  return true;
}

bool generate_keyring_options(std::string &options_id,
                              const std::string &latest_options_id) {
  if (!generate_keyring_options_id(options_id, latest_options_id)) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to generate options ID");
    return false;
  }

  std::string password;

  if (!encryption::generate_password(password)) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to generate password");
    return false;
  }

  const auto options = encryption::EncryptionOptions::generate(password);

  if (options == nullptr || !options->check_valid()) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to generate options");
    return false;
  }

  return set_keyring_options(options_id, options->to_json_string());
}

/**
 * @brief Replace active options holding a password with the structure of
 *        the passwords generated by earlier versions with newly generated
 *        ones
 *
 * The replaced options stay in the keyring for the existing log files. The
 * active options are kept when they cannot be read, when they are not valid
 * or when the new ones cannot be generated.
 *
 * @param [in,out] options_id ID of the active options
 */
void replace_legacy_generated_options(std::string &options_id) {
  const auto options = get_encryption_options(options_id);

  if (options == nullptr || !options->check_valid() ||
      !encryption::is_legacy_generated_password(options->get_password())) {
    return;
  }

  // The ID of the active options is padded with '\0', see
  // get_keyring_options_key_list_sorted(), the messages need it without
  // the padding
  const std::string old_options_id{options_id.c_str()};
  std::string new_options_id;

  if (!generate_keyring_options(new_options_id, old_options_id)) {
    const std::string message =
        "Failed to replace log encryption password with ID '" + old_options_id +
        "' generated by an earlier server version";
    LogComponentErr(WARNING_LEVEL, ER_LOG_PRINTF_MSG, message.c_str());
    return;
  }

  const std::string message =
      "Log encryption password with ID '" + old_options_id +
      "' generated by an earlier server version was replaced with newly "
      "generated password with ID '" +
      new_options_id +
      "'. Existing log files stay encrypted with the earlier password.";
  LogComponentErr(WARNING_LEVEL, ER_LOG_PRINTF_MSG, message.c_str());

  options_id = new_options_id;
}

}  // namespace

bool check_keyring_initialized() noexcept {
  my_service<SERVICE_TYPE(keyring_component_status)> component_status(
      "keyring_component_status", SysVars::get_comp_registry_srv());

  if (!component_status.is_valid()) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init keyring status service");
    return false;
  }

  return component_status->is_initialized();
}

bool check_generate_initial_encryption_options() noexcept {
  std::string options_id;

  if (!get_active_keyring_options_key(options_id)) {
    return false;
  }

  if (options_id.empty()) {
    if (!generate_keyring_options(options_id, "")) {
      return false;
    }
  } else {
    replace_legacy_generated_options(options_id);
  }

  SysVars::set_encryption_options_id(options_id);

  return true;
}

std::unique_ptr<encryption::EncryptionOptions>
get_encryption_options() noexcept {
  return get_encryption_options(SysVars::get_encryption_options_id());
}

std::unique_ptr<encryption::EncryptionOptions> get_encryption_options(
    const std::string &options_id) noexcept {
  std::string options_json_str;

  if (options_id.empty() ||
      !get_keyring_options(options_id, options_json_str)) {
    return nullptr;
  }

  return encryption::EncryptionOptions::from_json_string(options_json_str);
}

bool set_encryption_options(const std::string &password) noexcept {
  std::string options_id;
  std::string latest_options_id;

  // The options with the latest ID are the active ones after a restart. The
  // keyring holds an ID later than the one of the active options after it
  // was reloaded with other content. The age of the options is not needed.
  if (!get_active_keyring_options_key(latest_options_id, false)) {
    return false;
  }

  const auto active_options_id = SysVars::get_encryption_options_id();

  if (is_keyring_options_id_before(latest_options_id, active_options_id)) {
    latest_options_id = active_options_id;
  }

  if (!generate_keyring_options_id(options_id, latest_options_id)) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to generate options ID");
    return false;
  }

  const auto options = encryption::EncryptionOptions::generate(password);

  if (options == nullptr || !options->check_valid()) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to generate options");
    return false;
  }

  if (!set_keyring_options(options_id, options->to_json_string())) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "Failed to set options");
    return false;
  }

  SysVars::set_encryption_options_id(options_id);

  return true;
}

void prune_encryption_options(
    uint64_t remove_after_days,
    const std::vector<std::string> &existing_log_names) noexcept {
  OptionsIdList id_list;

  if (!get_keyring_options_key_list_sorted(id_list)) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to fetch options ids");
    return;
  }

  if (id_list.empty()) {
    return;
  }

  my_service<SERVICE_TYPE(keyring_writer)> writer_srv(
      "keyring_writer", SysVars::get_comp_registry_srv());

  if (!writer_srv.is_valid()) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init keyring writer service");
    return;
  }

  const std::regex file_opt_id_regex(file_opt_id_pattern);
  const std::regex keyring_opt_id_regex(keyring_opt_id_pattern);

  std::unordered_set<std::string> used_opts_ids;

  for (const auto &file_name : existing_log_names) {
    std::smatch file_match;
    if (std::regex_match(file_name, file_match, file_opt_id_regex)) {
      used_opts_ids.insert(file_match[1].str());
    }
  }

  // The active options are not used by any log file when the log is not
  // encrypted. Both IDs may be padded with '\0', see
  // get_keyring_options_key_list_sorted().
  const std::string active_options_id{
      SysVars::get_encryption_options_id().c_str()};

  for (const auto &el : id_list) {
    if (active_options_id == el.data_id.c_str()) {
      continue;
    }

    std::smatch keyring_match;
    if (!std::regex_match(el.data_id, keyring_match, keyring_opt_id_regex) ||
        used_opts_ids.count(keyring_match[1].str())) {
      // options_id is still used by some encrypted log
      // or failed to match key_id
      continue;
    }

    if (el.data_age_days < remove_after_days) {
      break;
    }

    if (writer_srv->remove(el.data_id.c_str(), kAuthId)) {
      LogComponentErr(ERROR_LEVEL, ER_AUDIT_KEYRING_OPTIONS_REMOVE_FAILURE,
                      el.data_id.c_str());
    }
  }
}

std::string get_options_id_timestamp(const std::string &options_id) noexcept {
  return options_id.substr(strlen(kAuthId) + 1);
}

std::string get_options_id_for_file_name(
    const std::string &file_name) noexcept {
  std::stringstream ss;
  std::smatch file_match;
  const std::regex opt_id_regex(file_opt_id_pattern);

  if (std::regex_match(file_name, file_match, opt_id_regex)) {
    ss << kAuthId << "-" << file_match[1].str();
  }

  return ss.str();
}

}  // namespace audit_log_filter::audit_keyring
