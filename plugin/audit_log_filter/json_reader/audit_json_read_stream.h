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

#ifndef AUDIT_LOG_FILTER_JSON_READER_AUDIT_JSON_READER_H_INCLUDED
#define AUDIT_LOG_FILTER_JSON_READER_AUDIT_JSON_READER_H_INCLUDED

#include "plugin/audit_log_filter/json_reader/file_reader_base.h"

#include <memory>

namespace audit_log_filter {

struct FileInfo;

namespace json_reader {

/**
 * @brief File stream reader for JSON audit logs. Implements access to
 *        compressed and encrypted log files. Based on
 *        rapidjson/filereadstream.h implementation.
 */
class AuditJsonReadStream {
  static inline size_t const kStreamBufferSize = 32768;

 public:
  typedef unsigned char Ch;

  AuditJsonReadStream();

  bool init() noexcept;
  bool open(FileInfo *file_info) noexcept;
  void close() noexcept;
  [[nodiscard]] bool check_eof_reached() const noexcept;

  [[nodiscard]] Ch Peek() const;
  [[nodiscard]] Ch Take();
  [[nodiscard]] size_t Tell() const;

  void Put(Ch);
  void Flush();
  Ch *PutBegin();
  size_t PutEnd(Ch *);

  [[nodiscard]] const Ch *Peek4() const;

 private:
  bool read() noexcept;

 private:
  std::unique_ptr<FileReaderBase> m_file_reader;
  std::unique_ptr<Ch[]> m_buffer;
  Ch *m_buffer_last;
  Ch *m_current;
  size_t m_read_count;
  size_t m_count;  // Number of characters read
  bool m_eof;
};

}  // namespace json_reader
}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_JSON_READER_AUDIT_JSON_READER_H_INCLUDED
