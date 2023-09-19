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

#ifndef AUDIT_LOG_FILTER_JSON_READER_FILE_READER_DECRYPTING_H_INCLUDED
#define AUDIT_LOG_FILTER_JSON_READER_FILE_READER_DECRYPTING_H_INCLUDED

#include "components/audit_log_filter/json_reader/file_reader_decorator_base.h"

#include <openssl/evp.h>

#include <memory>

namespace audit_log_filter::json_reader {

class FileReaderDecrypting final : public FileReaderDecoratorBase {
 public:
  explicit FileReaderDecrypting(std::unique_ptr<FileReaderBase> file_reader);

  ~FileReaderDecrypting() override;

  bool init() noexcept override;
  bool open(FileInfo *file_info) noexcept override;
  void close() noexcept override;
  ReadStatus read(unsigned char *out_buffer, size_t out_buffer_size,
                  size_t *read_size) noexcept override;

 private:
  const EVP_CIPHER *m_cipher;
  EVP_CIPHER_CTX *m_ctx;
  std::unique_ptr<unsigned char[]> m_key;
  std::unique_ptr<unsigned char[]> m_iv;
  std::unique_ptr<unsigned char[]> m_in_buff;
  const size_t m_in_buf_size;
};

}  // namespace audit_log_filter::json_reader

#endif  // AUDIT_LOG_FILTER_JSON_READER_FILE_READER_DECRYPTING_H_INCLUDED
