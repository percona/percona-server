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

#ifndef AUDIT_LOG_FILTER_LOG_WRITER_FILE_WRITER_ENCRYPTING_H_INCLUDED
#define AUDIT_LOG_FILTER_LOG_WRITER_FILE_WRITER_ENCRYPTING_H_INCLUDED

#include "file_writer_decorator_base.h"

#include <openssl/evp.h>

#include <memory>

namespace audit_log_filter::log_writer {

class FileWriterEncrypting final : public FileWriterDecoratorBase {
 public:
  explicit FileWriterEncrypting(std::unique_ptr<FileWriterBase> file_writer);

  ~FileWriterEncrypting() override;

  /**
   * @brief Init file writer.
   *
   * @return true in case of success, false otherwise
   */
  bool init() noexcept override;

  /**
   * @brief Prepare writer for work with newly opened log file.
   *
   * @return true in case of success, false otherwise
   */
  bool open() noexcept override;

  /**
   * @brief Finish working with currently active log file before
   *        actually closing it.
   */
  void close() noexcept override;

  /**
   * @brief Write file.
   *
   * @param record Log record
   * @param size Log record size
   */
  void write(const char *record, size_t size) noexcept override;

 private:
  const EVP_CIPHER *m_cipher;
  EVP_CIPHER_CTX *m_ctx;
  std::unique_ptr<unsigned char[]> m_key;
  std::unique_ptr<unsigned char[]> m_iv;
  std::unique_ptr<unsigned char[]> m_out_buff;
};

}  // namespace audit_log_filter::log_writer

#endif  // AUDIT_LOG_FILTER_LOG_WRITER_FILE_WRITER_ENCRYPTING_H_INCLUDED
