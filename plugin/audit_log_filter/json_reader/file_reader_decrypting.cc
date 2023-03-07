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

#include "plugin/audit_log_filter/json_reader/file_reader_decrypting.h"

#include "plugin/audit_log_filter/audit_encryption.h"
#include "plugin/audit_log_filter/audit_error_log.h"
#include "plugin/audit_log_filter/audit_log_reader.h"

#include <openssl/err.h>
#include <openssl/evp.h>

namespace audit_log_filter::json_reader {

namespace {

const size_t kEvpKeyLength = 32;
const size_t kInBufferSize = 32768;

}  // namespace

FileReaderDecrypting::FileReaderDecrypting(
    std::unique_ptr<FileReaderBase> file_reader)
    : FileReaderDecoratorBase(std::move(file_reader)),
      m_cipher{EVP_aes_256_cbc()},
      m_ctx{nullptr},
      m_key{nullptr},
      m_iv{nullptr},
      m_in_buff{nullptr},
      m_in_buf_size{kInBufferSize - EVP_CIPHER_block_size(m_cipher)} {}

FileReaderDecrypting::~FileReaderDecrypting() {
  if (m_ctx != nullptr) {
    close();
  }
}

bool FileReaderDecrypting::init() noexcept {
  if (m_cipher == nullptr) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "EVP_aes_256_cbc init failed");
    return false;
  }

  m_key = std::make_unique<unsigned char[]>(kEvpKeyLength);
  m_iv = std::make_unique<unsigned char[]>(EVP_MAX_IV_LENGTH);

  if (m_key == nullptr || m_iv == nullptr) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "Failed to init key buffer");
    return false;
  }

  m_in_buff = std::make_unique<unsigned char[]>(m_in_buf_size);

  if (m_in_buff == nullptr) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "Failed to init in buffer");
    return false;
  }

  return FileReaderDecoratorBase::init();
}

bool FileReaderDecrypting::open(FileInfo *file_info) noexcept {
  assert(m_key != nullptr && m_iv != nullptr && m_in_buff != nullptr);

  const auto *encryption_options = file_info->encryption_options.get();

  if (!encryption_options->check_valid()) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Invalid options provided for id %s",
                    file_info->encryption_options_id.c_str());
    return false;
  }

  const auto &keyring_password = encryption_options->get_password();
  const auto keyring_iterations = encryption_options->get_iterations();
  const auto &keyring_salt = encryption_options->get_salt();

  if (keyring_password.empty()) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "Empty password for id %s",
                    file_info->encryption_options_id.c_str());
    return false;
  }

  if (keyring_iterations < 1) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Bad iterations count for id %s",
                    file_info->encryption_options_id.c_str());
    return false;
  }

  if (keyring_salt.empty()) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "Empty salt for id %s",
                    file_info->encryption_options_id.c_str());
    return false;
  }

  // Derive key and default iv concatenated into a temporary buffer
  unsigned char tmp_key_iv[kEvpKeyLength + EVP_MAX_IV_LENGTH];

  auto ik_len = EVP_CIPHER_key_length(m_cipher);
  auto iv_len = EVP_CIPHER_iv_length(m_cipher);

  if (!PKCS5_PBKDF2_HMAC(
          keyring_password.data(), static_cast<int>(keyring_password.size()),
          keyring_salt.data(), static_cast<int>(keyring_salt.size()),
          static_cast<int>(keyring_iterations), EVP_sha256(), ik_len + iv_len,
          tmp_key_iv)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "PKCS5_PBKDF2_HMAC error: %s",
                    ERR_error_string(ERR_peek_error(), nullptr));
    return false;
  }

  memcpy(m_key.get(), tmp_key_iv, ik_len);
  memcpy(m_iv.get(), tmp_key_iv + ik_len, iv_len);

  m_ctx = EVP_CIPHER_CTX_new();
  if (m_ctx == nullptr) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "EVP_CIPHER_CTX_new failed");
    return false;
  }

  if (EVP_DecryptInit(m_ctx, m_cipher, m_key.get(), m_iv.get()) != 1) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "EVP_CipherInit_ex error: %s",
                    ERR_error_string(ERR_peek_error(), nullptr));
    ERR_clear_error();
    EVP_CIPHER_CTX_free(m_ctx);
    m_ctx = nullptr;
    return false;
  }

  if (!FileReaderDecoratorBase::open(file_info)) {
    close();
    return false;
  }

  const size_t file_salt_size = keyring_salt.size() + 8;  // "Salted__"
  size_t actual_file_salt_size = 0;
  assert(m_in_buf_size > file_salt_size);
  auto status = FileReaderDecoratorBase::read(m_in_buff.get(), file_salt_size,
                                              &actual_file_salt_size);

  if (status != ReadStatus::Ok || file_salt_size != actual_file_salt_size ||
      memcmp(m_in_buff.get() + 8, keyring_salt.data(), keyring_salt.size()) !=
          0) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "Bad magic number");
    close();
    return false;
  }

  return true;
}

void FileReaderDecrypting::close() noexcept {
  if (m_ctx != nullptr) {
    ERR_clear_error();
    EVP_CIPHER_CTX_free(m_ctx);
    m_ctx = nullptr;
  }

  FileReaderDecoratorBase::close();
}

ReadStatus FileReaderDecrypting::read(unsigned char *out_buffer,
                                      const size_t out_buffer_size,
                                      size_t *read_size) noexcept {
  size_t in_buff_data_size = 0;
  auto status = FileReaderDecoratorBase::read(m_in_buff.get(), m_in_buf_size,
                                              &in_buff_data_size);

  if (status == ReadStatus::Error) {
    return status;
  }

  if (in_buff_data_size == 0) {
    return ReadStatus::Eof;
  }

  auto decrypted_size =
      static_cast<int>(out_buffer_size) - EVP_CIPHER_block_size(m_cipher);

  if (EVP_DecryptUpdate(m_ctx, out_buffer, &decrypted_size, m_in_buff.get(),
                        static_cast<int>(in_buff_data_size)) != 1) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "EVP_DecryptUpdate error: %s",
                    ERR_error_string(ERR_peek_error(), nullptr));
    return ReadStatus::Error;
  }

  *read_size = decrypted_size;

  if (status == ReadStatus::Eof) {
    int final_size = 0;

    if (EVP_DecryptFinal(m_ctx, out_buffer + decrypted_size, &final_size) !=
        1) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "EVP_DecryptFinal error: %s",
                      ERR_error_string(ERR_peek_error(), nullptr));
      return ReadStatus::Error;
    }

    *read_size += final_size;
  }

  return status;
}

}  // namespace audit_log_filter::json_reader
