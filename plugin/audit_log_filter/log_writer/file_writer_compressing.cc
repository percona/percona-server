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

#include "file_writer_compressing.h"

#include "plugin/audit_log_filter/audit_error_log.h"

namespace audit_log_filter::log_writer {

FileWriterCompressing::FileWriterCompressing(
    std::unique_ptr<FileWriterBase> file_writer)
    : FileWriterDecoratorBase(std::move(file_writer)) {}

FileWriterCompressing::~FileWriterCompressing() { deflateEnd(&m_strm); }

bool FileWriterCompressing::init() noexcept {
  return FileWriterDecoratorBase::init();
}

bool FileWriterCompressing::open() noexcept {
  m_strm.zalloc = Z_NULL;
  m_strm.zfree = Z_NULL;
  m_strm.opaque = Z_NULL;

  auto ret = deflateInit2(&m_strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                          MAX_WBITS + 16, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);

  if (ret != Z_OK) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init compressing: %i", ret);
    return false;
  }

  return FileWriterDecoratorBase::open();
}

void FileWriterCompressing::close() noexcept {
  m_flush = Z_FINISH;
  do_deflate();
  deflateEnd(&m_strm);

  FileWriterDecoratorBase::close();
}

void FileWriterCompressing::write(const char *record, size_t size) noexcept {
  m_strm.avail_in = size;
  m_strm.next_in =
      reinterpret_cast<unsigned char *>(const_cast<char *>(record));
  m_flush = Z_NO_FLUSH;

  do_deflate();
}

void FileWriterCompressing::do_deflate() noexcept {
  do {
    m_strm.avail_out = COMPRESSION_CHUNK;
    m_strm.next_out = m_out_buff;

    [[maybe_unused]] int ret = deflate(&m_strm, m_flush);
    assert(ret != Z_STREAM_ERROR);

    size_t compressed_size = COMPRESSION_CHUNK - m_strm.avail_out;

    if (compressed_size > 0) {
      FileWriterDecoratorBase::write(reinterpret_cast<const char *>(m_out_buff),
                                     compressed_size);
    }
  } while (m_strm.avail_out == 0);

  assert(m_strm.avail_in == 0);
}

}  // namespace audit_log_filter::log_writer
