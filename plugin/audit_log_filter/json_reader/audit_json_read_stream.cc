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

#include "plugin/audit_log_filter/json_reader/audit_json_read_stream.h"

#include "plugin/audit_log_filter/audit_log_reader.h"
#include "plugin/audit_log_filter/json_reader/file_reader.h"
#include "plugin/audit_log_filter/json_reader/file_reader_decompressing.h"
#include "plugin/audit_log_filter/json_reader/file_reader_decrypting.h"

#include <cassert>

namespace audit_log_filter::json_reader {

namespace {

std::unique_ptr<FileReaderBase> get_file_reader(const FileInfo *file_info) {
  std::unique_ptr<FileReaderBase> reader = std::make_unique<FileReader>();

  if (file_info->is_encrypted) {
    reader = std::make_unique<FileReaderDecrypting>(std::move(reader));
  }

  if (file_info->is_compressed) {
    reader = std::make_unique<FileReaderDecompressing>(std::move(reader));
  }

  return reader;
}

}  // namespace

AuditJsonReadStream::AuditJsonReadStream()
    : m_file_reader{nullptr},
      m_buffer{nullptr},
      m_buffer_last{nullptr},
      m_current{nullptr},
      m_read_count{0},
      m_count{0},
      m_eof{false} {}

bool AuditJsonReadStream::init() noexcept {
  assert(kStreamBufferSize >= 4);
  m_buffer = std::make_unique<Ch[]>(kStreamBufferSize);

  if (m_buffer == nullptr) {
    return false;
  }

  return true;
}

bool AuditJsonReadStream::open(FileInfo *file_info) noexcept {
  assert(m_buffer != nullptr);

  m_file_reader = get_file_reader(file_info);

  if (m_file_reader == nullptr || !m_file_reader->init() ||
      !m_file_reader->open(file_info)) {
    return false;
  }

  memset(m_buffer.get(), 0, kStreamBufferSize);
  m_current = m_buffer.get();
  m_buffer_last = m_current;
  m_read_count = 0;
  m_count = 0;
  m_eof = false;

  return read();
}

void AuditJsonReadStream::close() noexcept {
  m_file_reader->close();
  m_file_reader.reset();
}

bool AuditJsonReadStream::check_eof_reached() const noexcept {
  return m_eof && m_current == m_buffer_last;
}

AuditJsonReadStream::Ch AuditJsonReadStream::Peek() const { return *m_current; }

AuditJsonReadStream::Ch AuditJsonReadStream::Take() {
  Ch c = *m_current;
  read();
  return c;
}

size_t AuditJsonReadStream::Tell() const {
  return m_count + static_cast<size_t>(m_current - m_buffer.get());
}

void AuditJsonReadStream::Put(Ch) { assert(false); }

void AuditJsonReadStream::Flush() { assert(false); }

AuditJsonReadStream::Ch *AuditJsonReadStream::PutBegin() {
  assert(false);
  return nullptr;
}

size_t AuditJsonReadStream::PutEnd(Ch *) {
  assert(false);
  return 0;
}

const AuditJsonReadStream::Ch *AuditJsonReadStream::Peek4() const {
  return (m_current + 4 - !m_eof <= m_buffer_last) ? m_current : nullptr;
}

bool AuditJsonReadStream::read() noexcept {
  if (m_file_reader == nullptr) {
    return false;
  }

  auto status = ReadStatus::Ok;

  if (m_current < m_buffer_last)
    ++m_current;
  else if (!m_eof) {
    m_count += m_read_count;
    status =
        m_file_reader->read(m_buffer.get(), kStreamBufferSize, &m_read_count);
    m_buffer_last = m_buffer.get() + m_read_count - 1;
    m_current = m_buffer.get();

    if (status == ReadStatus::Eof) {
      m_buffer[m_read_count] = '\0';
      ++m_buffer_last;
      m_eof = true;
    }
  }

  return status != ReadStatus::Error;
}

}  // namespace audit_log_filter::json_reader
