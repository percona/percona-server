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

#include "plugin/audit_log_filter/log_writer/file_handle.h"
#include "plugin/audit_log_filter/audit_psi_info.h"
#include "plugin/audit_log_filter/sys_vars.h"

#include <mysql/psi/mysql_mutex.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <regex>
#include <string>

namespace {
const std::string kRotationTimeFormat{"%Y%m%dT%H%M%S"};
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

bool FileHandle::init_buffer(const size_t buffer_size,
                             const bool drop_if_full) noexcept {
  auto flush_cb = [this](const char *buf, size_t size) {
    write_file(buf, size);
  };

  return m_buffer.init(buffer_size, drop_if_full, flush_cb);
}

void FileHandle::close_buffer() noexcept { m_buffer.shutdown(); }

void FileHandle::write_file(const std::string &record) noexcept {
  write_file(record.c_str(), record.length());
}

void FileHandle::write_file(const char *record, const size_t size) noexcept {
  m_file.write(record, size);
  m_file.flush();
}

void FileHandle::write_buffer(const std::string &record) noexcept {
  m_buffer.write(record.c_str(), record.length());
}

uint64_t FileHandle::get_file_size() const noexcept {
  assert(m_file.is_open());
  return std::filesystem::exists(m_path) ? std::filesystem::file_size(m_path)
                                         : 0;
}

uint64_t FileHandle::get_total_log_size(const std::string &working_dir_name,
                                        const std::string &file_name) noexcept {
  auto base_name = std::filesystem::path{file_name}.filename();
  while (base_name.has_extension()) {
    base_name.replace_extension();
  }

  uint64_t size = 0;

  for (const auto &entry :
       std::filesystem::directory_iterator(working_dir_name)) {
    auto entry_file_name = entry.path().filename();

    while (entry_file_name.has_extension()) {
      entry_file_name.replace_extension();
    }

    if (entry.is_regular_file() && entry_file_name == base_name) {
      size += entry.file_size();
    }
  }

  return size;
}

bool FileHandle::remove_file(const std::filesystem::path &path) noexcept {
  std::error_code ec;
  return std::filesystem::remove(path, ec);
}

void FileHandle::remove_file_footer(
    const std::filesystem::path &file_path,
    const std::string &expected_footer) const noexcept {
  assert(expected_footer.length() > 0);

  std::fstream file;
  file.open(file_path, std::ios::in);

  if (!file.is_open()) {
    return;
  }

  file.seekg(-expected_footer.length(), std::ios_base::end);

  if (file.fail()) {
    file.close();
    return;
  }

  std::string file_footer;
  std::getline(file, file_footer);

  if (file.fail()) {
    file.close();
    return;
  }

  file.close();

  if (expected_footer.back() == '\n') {
    file_footer.push_back('\n');
  }

  if (expected_footer == file_footer) {
    std::filesystem::resize_file(
        file_path,
        std::filesystem::file_size(file_path) - expected_footer.size());
  }
}

std::error_code FileHandle::rotate(const std::string &working_dir_name,
                                   const std::string &file_name) noexcept {
  auto current_file_path = std::filesystem::path{working_dir_name} /
                           std::filesystem::path{file_name};

  if (!std::filesystem::exists(current_file_path)) {
    return std::error_code{};
  }

  std::time_t t =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  DBUG_EXECUTE_IF("audit_log_filter_debug_timestamp", {
    t = std::chrono::system_clock::to_time_t(SysVars::get_debug_time_point());
  });

  std::stringstream new_file_name;
  new_file_name << current_file_path.filename().replace_extension().c_str()
                << "."
                << std::put_time(std::localtime(&t),
                                 kRotationTimeFormat.c_str())
                << current_file_path.extension().c_str();

  std::filesystem::path new_file_path{current_file_path};
  new_file_path.replace_filename(new_file_name.str());
  std::error_code ec;

  std::filesystem::rename(current_file_path, new_file_path, ec);

  return ec;
}

PruneFilesList FileHandle::get_prune_files(
    const std::string &working_dir_name,
    const std::string &file_name) noexcept {
  PruneFilesList prune_files;
  const std::regex log_time_regex(R"(.*\.(\d{8}T\d{6})\..*)");
  auto base_file_name =
      std::filesystem::path{file_name}.replace_extension().string();
  auto time_now = std::time(nullptr);

  for (const auto &entry :
       std::filesystem::directory_iterator{working_dir_name}) {
    const auto name = entry.path().filename().string();

    if (entry.is_regular_file() &&
        name.find(base_file_name) != std::string::npos) {
      std::smatch pieces_match;

      if (std::regex_match(name, pieces_match, log_time_regex)) {
        std::tm tm{};
        std::istringstream ss(pieces_match[1].str());
        ss >> std::get_time(&tm, kRotationTimeFormat.c_str());
        tm.tm_isdst = -1;
        auto time_rotated = timelocal(&tm);

        prune_files.push_back(
            {entry.path(), entry.file_size(),
             static_cast<ulonglong>(time_now - time_rotated)});
      }
    }
  }

  return prune_files;
}

}  // namespace audit_log_filter::log_writer
