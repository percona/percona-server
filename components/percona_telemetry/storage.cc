/* Copyright (c) 2024 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "logger.h"
#include "storage.h"

namespace fs = std::filesystem;

namespace {
constexpr auto path_separator = std::filesystem::path::preferred_separator;

// Let's keep it as std::string, because there is no way to string + string_vew
const std::string TMP_FILE_EXTENSION(".tmp");
const std::string JSON_FILE_EXTENSION(".json");

class file_stream_exception : public std::exception {
 public:
  file_stream_exception(const char *what) : what_(what) {}
  file_stream_exception(const std::string &what) : what_(what) {}
  const char *what() const noexcept override { return what_.c_str(); }

 private:
  std::string what_;
};

static std::string random_uuid() {
  static thread_local boost::uuids::random_generator gen;
  auto generated = gen();
  return boost::uuids::to_string(generated);
}

}  // namespace

Storage::Storage(Config &config, Logger &logger)
    : config_(config), logger_(logger), uuid_(random_uuid()) {}

void Storage::clean_old_reports(std::chrono::seconds current_time) {
  auto oldest_timestamp = current_time - config_.history_keep_interval();
  auto oldest_unconditional_cleanup_timestamp =
      current_time - config_.unconditional_history_cleanup_interval();

  std::vector<std::string> files_to_delete;

  for (const auto &entry :
       fs::directory_iterator(config_.telemetry_storage_dir_path())) {
    if (entry.path().extension() == TMP_FILE_EXTENSION ||
        entry.path().extension() == JSON_FILE_EXTENSION) {
      auto filename = entry.path().stem().string();
      // filename should be <timestamp>-<uuid_>.json/tmp
      size_t delimiter_pos = filename.find("-");
      if (delimiter_pos == std::string::npos || delimiter_pos == 0 ||
          delimiter_pos == filename.length() - 1) {
        logger_.warning("Skipping file deletion %s", filename.c_str());
        continue;
      }
      auto file_timestamp_str = filename.substr(0, delimiter_pos);
      auto file_uuid = filename.substr(delimiter_pos + 1);
      bool our_file = (file_uuid == uuid_);

      try {
        auto file_timestamp =
            std::chrono::seconds(std::stoul(file_timestamp_str));
        // We need to delete the file in 2 cases:
        // 1. This is our file and it is older than this instance's
        // history_keep_interval
        // 2. This is someone's else file, but it is older than the highest
        // possible history_keep_interval
        if (our_file && (file_timestamp < oldest_timestamp)) {
          logger_.info(
              "Scheduling file %s owned by this server for deletion because it "
              "is older than %ld seconds",
              entry.path().filename().c_str(),
              config_.history_keep_interval().count());
          files_to_delete.push_back(entry.path());
        } else if (!our_file &&
                   (file_timestamp < oldest_unconditional_cleanup_timestamp)) {
          logger_.info(
              "Scheduling file %s owned by other server for deletion because "
              "it is older than %ld seconds",
              entry.path().filename().c_str(),
              config_.unconditional_history_cleanup_interval().count());
          files_to_delete.push_back(entry.path());
        }
      } catch (const std::invalid_argument &e) {
        logger_.warning(
            "Skipping file deletion %s. Can not determine timestamp.",
            filename.c_str());
      } catch (const std::out_of_range &e) {
        logger_.warning("Skipping file deletion %s. Timestamp out of range.",
                        filename.c_str());
      }
    }
  }

  for (const auto &path : files_to_delete) {
    std::error_code ec;
    logger_.info("Removing telemetry file: %s", path.c_str());
    if (!fs::remove(path, ec)) {
      logger_.warning("Failed to remove file %s, ec: %d, msg: %s", path.c_str(),
                      ec.value(), ec.message().c_str());
    }
  }
}

bool Storage::store_report(const std::string &report) {
  try {
    auto now = std::chrono::system_clock::now();
    auto current_time = std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch());

    clean_old_reports(current_time);

    /* The report can be empty if we failed to collect all metrics.
       In such a case skip creation of an empty file. */
    if (report.length() == 0) {
      return false;
    }

    std::string filename = config_.telemetry_storage_dir_path() +
                           path_separator +
                           std::to_string(current_time.count()) + "-" + uuid_;
    std::string tmp_filename = filename + TMP_FILE_EXTENSION;
    std::string json_filename = filename + JSON_FILE_EXTENSION;
    std::ofstream file_stream(tmp_filename);
    if (!file_stream.is_open()) throw file_stream_exception(strerror(errno));

    if ((file_stream.write(report.c_str(), report.length())).fail())
      throw file_stream_exception(strerror(errno));

    // close to flush
    file_stream.close();
    std::error_code ec;
    fs::rename(tmp_filename, json_filename, ec);
    if (ec.value() != 0) {
      throw file_stream_exception(ec.message());
    }

    fs::permissions(json_filename,
                    fs::perms::owner_read | fs::perms::owner_write |
                        fs::perms::group_read | fs::perms::others_read,
                    fs::perm_options::add);

    logger_.info("Created telemetry file: %s", json_filename.c_str());
    return false;
  } catch (const std::exception &e) {
    logger_.warning("Problem during telemetry file write: %s", e.what());
    return true;
  }
}
