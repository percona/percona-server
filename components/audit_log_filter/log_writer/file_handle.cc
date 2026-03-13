/* Copyright (c) 2022 Percona LLC and/or its affiliates. All rights reserved.

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

#include "components/audit_log_filter/log_writer/file_handle.h"
#include "components/audit_log_filter/audit_error_log.h"
#include "components/audit_log_filter/audit_psi_info.h"
#include "components/audit_log_filter/log_writer/file_name.h"
#include "components/audit_log_filter/sys_vars.h"

#include <mysql/psi/mysql_mutex.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <regex>
#include <string>

namespace {
const std::string kRotationTimeFormat{"%Y%m%dT%H%M%S"};

bool is_regular_file(const std::filesystem::directory_entry &entry) noexcept {
  std::error_code ec;
  return entry.is_regular_file(ec) && !ec;
}

bool get_entry_file_size(const std::filesystem::directory_entry &entry,
                         uintmax_t *size) noexcept {
  std::error_code ec;
  *size = entry.file_size(ec);
  if (ec) {
    *size = 0;
    return false;
  }
  return true;
}

template <typename Callback>
void for_each_directory_entry(const std::string &working_dir_name,
                              Callback callback) noexcept {
  std::error_code ec;
  auto it = std::filesystem::directory_iterator(working_dir_name, ec);
  const auto end = std::filesystem::directory_iterator{};

  if (ec) {
    LogComponentErr(WARNING_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to list audit log directory '%s': %s",
                    working_dir_name.c_str(), ec.message().c_str());
  }

  for (; !ec && it != end; it.increment(ec)) {
    if (!callback(*it)) {
      break;
    }
  }
}
}

#if defined(HAVE_PSI_INTERFACE)
static PSI_mutex_key key_LOCK_audit_filter_service;
static PSI_mutex_info mutex_list[] = {
    {&key_LOCK_audit_filter_service, "file_handle::m_lock", PSI_FLAG_SINGLETON,
     PSI_VOLATILITY_UNKNOWN, PSI_DOCUMENT_ME}};
#else
#define key_LOCK_audit_filter_service nullptr
#endif /*HAVE_PSI_INTERFACE && !FLOGGER_NO_PSI*/

namespace audit_log_filter::log_writer {

bool FileHandle::open_file(std::filesystem::path file_path) noexcept {
  assert(!m_file.is_open() && m_path.empty());
  m_path = std::move(file_path);
  m_file.open(m_path, std::ios::out | std::ios_base::app);

  if (!m_file.is_open()) {
    m_file.close();
    m_path.clear();
    return false;
  }

#ifdef HAVE_PSI_INTERFACE
  mysql_mutex_register(AUDIT_LOG_FILTER_PSI_CATEGORY, mutex_list,
                       array_elements(mutex_list));
#endif /* HAVE_PSI_INTERFACE */

  mysql_mutex_init(key_LOCK_audit_filter_service, &m_lock, MY_MUTEX_INIT_FAST);

  return true;
}

bool FileHandle::close_file() noexcept {
  if (!m_file.is_open() && m_path.empty()) {
    return true;
  }

  m_file.close();
  m_path.clear();

  mysql_mutex_destroy(&m_lock);

  return !m_file.fail();
}

void FileHandle::write_file(const std::string &record) noexcept {
  write_file(record.c_str(), record.length());
}

void FileHandle::write_file(const char *record, const size_t size) noexcept {
  m_file.write(record, size);
  m_file.flush();
}

uint64_t FileHandle::get_file_size() const noexcept {
  assert(m_file.is_open());
  std::error_code ec;
  auto size = std::filesystem::file_size(m_path, ec);
  return ec ? 0 : size;
}

std::filesystem::path FileHandle::get_file_path() const noexcept {
  assert(m_file.is_open());
  return m_path;
}

void FileHandle::flush() noexcept {
  assert(m_file.is_open());
  m_file.flush();
}

std::filesystem::path FileHandle::get_not_rotated_file_path(
    const std::string &working_dir_name,
    const std::string &file_name) noexcept {
  const auto base_file_name = FileName::from_path(file_name).get_base_name();

  if (base_file_name.empty()) {
    return {};
  }

  std::filesystem::path result;
  for_each_directory_entry(working_dir_name, [&](const auto &entry) {
    if (is_regular_file(entry) &&
        entry.path().filename().string().find(base_file_name) !=
            std::string::npos &&
        !FileName::from_path(entry.path().filename()).is_rotated()) {
      result = entry.path();
      return false;
    }
    return true;
  });

  return result;
}

uint64_t FileHandle::get_total_log_size(const std::string &working_dir_name,
                                        const std::string &file_name) noexcept {
  auto base_name = std::filesystem::path{file_name}.filename();
  while (base_name.has_extension()) {
    base_name.replace_extension();
  }

  if (base_name.empty()) {
    return 0;
  }

  uint64_t size = 0;
  for_each_directory_entry(working_dir_name, [&](const auto &entry) {
    auto entry_file_name = entry.path().filename();

    while (entry_file_name.has_extension()) {
      entry_file_name.replace_extension();
    }

    if (is_regular_file(entry) && entry_file_name == base_name) {
      uintmax_t fsize = 0;
      if (get_entry_file_size(entry, &fsize)) {
        size += fsize;
      }
    }
    return true;
  });

  return size;
}

bool FileHandle::remove_file(const std::filesystem::path &path) noexcept {
  std::error_code ec;
  return std::filesystem::remove(path, ec);
}

bool FileHandle::remove_file_footer(
    const std::filesystem::path &file_path,
    const std::string &expected_footer) noexcept {
  assert(expected_footer.length() > 0);

  std::fstream file;
  file.open(file_path, std::ios::in);

  if (!file.is_open()) {
    return false;
  }

  file.seekg(-expected_footer.length(), std::ios_base::end);

  if (file.fail()) {
    file.close();
    return true;
  }

  std::string file_footer;
  std::getline(file, file_footer);

  if (file.fail()) {
    file.close();
    return true;
  }

  file.close();

  if (expected_footer.back() == '\n') {
    file_footer.push_back('\n');
  }

  if (expected_footer != file_footer) {
    return true;
  }

  std::error_code ec;
  auto current_size = std::filesystem::file_size(file_path, ec);
  if (!ec && current_size >= expected_footer.size()) {
    std::filesystem::resize_file(file_path,
                                 current_size - expected_footer.size(), ec);
  }
  if (ec) {
    const auto path_str = file_path.string();
    LogComponentErr(WARNING_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to remove footer from audit log '%s': %s",
                    path_str.c_str(), ec.message().c_str());
    return false;
  }

  return true;
}

void FileHandle::rotate(const std::filesystem::path &current_file_path,
                        FileRotationResult *result) noexcept {
  std::error_code ec;
  if (!std::filesystem::exists(current_file_path, ec)) {
    result->error_code = ec.value();
    if (ec) {
      result->status_string = ec.message();
    }
    return;
  }

  std::time_t t =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  DBUG_EXECUTE_IF("audit_log_filter_debug_timestamp", {
    t = std::chrono::system_clock::to_time_t(
        SysVars::get_debug_time_point_for_rotation());
  });

  const auto filename_str = current_file_path.filename().string();
  auto first_ext_pos = filename_str.find_first_of('.');

  std::string base_file_name_str;
  std::string extensions_str;

  if (first_ext_pos == std::string::npos) {
    base_file_name_str = filename_str;
  } else {
    base_file_name_str = filename_str.substr(0, first_ext_pos);
    extensions_str = filename_str.substr(first_ext_pos);
  }

  std::filesystem::path new_file_path;
  std::string new_file_name_str;
  std::size_t seq = 0;

  do {
    std::string seq_str;
    if (seq != 0) {
      seq_str = "-" + std::to_string(seq);
    }
    seq++;

    std::stringstream new_file_name;
    new_file_name << base_file_name_str << "."
                  << std::put_time(std::localtime(&t),
                                   kRotationTimeFormat.c_str())
                  << seq_str << extensions_str;
    new_file_name_str = new_file_name.str();

    new_file_path = current_file_path;
    new_file_path.replace_filename(new_file_name_str);
  } while (std::filesystem::exists(new_file_path, ec));

  if (ec) {
    result->error_code = ec.value();
    result->status_string = ec.message();
    return;
  }

  std::filesystem::rename(current_file_path, new_file_path, ec);

  result->error_code = ec.value();

  if (result->error_code == 0) {
    result->status_string = new_file_name_str;
  } else {
    result->status_string = ec.message();
  }
}

PruneFilesList FileHandle::get_prune_files(
    const std::string &working_dir_name,
    const std::string &file_name) noexcept {
  PruneFilesList prune_files;

  const auto base_file_name = FileName::from_path(file_name).get_base_name();

  if (base_file_name.empty()) {
    return prune_files;
  }

  auto time_now = std::chrono::system_clock::now();

  DBUG_EXECUTE_IF("audit_log_filter_debug_timestamp", {
    // This will return the time of the newest rotated log + 1 minute so
    // file age will be calculated properly for files which are subject
    // for age based pruning.
    time_now = SysVars::get_debug_time_point_for_rotation();
  });

  for_each_directory_entry(working_dir_name, [&](const auto &entry) {
    if (is_regular_file(entry) &&
        entry.path().filename().string().find(base_file_name) !=
            std::string::npos) {
      auto parsed_file_name = FileName::from_path(entry.path().filename());

      if (parsed_file_name.is_rotated()) {
        auto timestamp = parsed_file_name.get_rotation_time().timestamp.value();
        uintmax_t fsize = 0;
        if (get_entry_file_size(entry, &fsize)) {
          prune_files.push_back({entry.path(), fsize, time_now - timestamp});
        }
      }
    }
    return true;
  });

  return prune_files;
}

std::vector<std::string> FileHandle::get_log_names_list(
    const std::string &working_dir_name,
    const std::string &file_name) noexcept {
  std::vector<std::string> list;

  auto base_file_name =
      std::filesystem::path{file_name}.replace_extension().string();

  if (base_file_name.empty()) {
    return list;
  }

  for_each_directory_entry(working_dir_name, [&](const auto &entry) {
    const auto name = entry.path().filename().string();

    if (is_regular_file(entry) &&
        name.find(base_file_name) != std::string::npos) {
      list.push_back(name);
    }
    return true;
  });

  return list;
}

}  // namespace audit_log_filter::log_writer
