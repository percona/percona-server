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

#include "components/audit_log_filter/json_reader/file_reader_decompressing.h"

#include "components/audit_log_filter/audit_error_log.h"

#include <string>

namespace audit_log_filter::json_reader {

FileReaderDecompressing::FileReaderDecompressing(
    std::unique_ptr<FileReaderBase> file_reader)
    : FileReaderDecoratorBase(std::move(file_reader)) {}

FileReaderDecompressing::~FileReaderDecompressing() {
  if (is_opened) {
    close();
  }
}

bool FileReaderDecompressing::init() noexcept {
  return FileReaderDecoratorBase::init();
}

bool FileReaderDecompressing::open(FileInfo *file_info) noexcept {
  if (!FileReaderDecoratorBase::open(file_info)) {
    return false;
  }

  if (ReadStatus::Error == FileReaderDecoratorBase::read(
                               m_in_buff, kInBufferSize,
                               reinterpret_cast<size_t *>(&m_strm.avail_in))) {
    FileReaderDecoratorBase::close();
    return false;
  }

  m_strm.zalloc = Z_NULL;
  m_strm.zfree = Z_NULL;
  m_strm.opaque = Z_NULL;
  m_strm.next_in = m_in_buff;

  auto ret = inflateInit2(&m_strm, MAX_WBITS + 16);

  if (ret != Z_OK) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init decompressing: %i", ret);
    FileReaderDecoratorBase::close();
    return false;
  }

  is_opened = true;

  return true;
}

void FileReaderDecompressing::close() noexcept {
  is_opened = false;
  inflateEnd(&m_strm);
  FileReaderDecoratorBase::close();
}

ReadStatus FileReaderDecompressing::read(unsigned char *out_buffer,
                                         const size_t out_buffer_size,
                                         size_t *read_size) noexcept {
  auto status = ReadStatus::Ok;

  if (m_strm.avail_in == 0) {
    status = FileReaderDecoratorBase::read(
        m_in_buff, kInBufferSize, reinterpret_cast<size_t *>(&m_strm.avail_in));

    if (status == ReadStatus::Error) {
      inflateEnd(&m_strm);
      return status;
    }

    m_strm.next_in = m_in_buff;
  }

  m_strm.next_out = out_buffer;
  m_strm.avail_out = out_buffer_size;

  int ret = inflate(&m_strm, Z_SYNC_FLUSH);

  *read_size = out_buffer_size - m_strm.avail_out;

  if (ret == Z_STREAM_END) {
    status = ReadStatus::Eof;
  } else if (ret != Z_OK) {
    status = ReadStatus::Error;
    inflateEnd(&m_strm);
  }

  return status;
}

}  // namespace audit_log_filter::json_reader
