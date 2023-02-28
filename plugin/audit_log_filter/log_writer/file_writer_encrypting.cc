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

#include "file_writer_encrypting.h"

#include "plugin/audit_log_filter/audit_error_log.h"
#include "plugin/audit_log_filter/audit_keyring.h"
#include "plugin/audit_log_filter/sys_vars.h"

#include <openssl/err.h>
#include <openssl/evp.h>

#include <include/scope_guard.h>
#include <cstring>
#include <string>

namespace audit_log_filter::log_writer {
namespace {

const size_t kEvpKeyLength = 32;
const size_t kEncryptChunkSize = 1024 * 1024;

}  // namespace

FileWriterEncrypting::FileWriterEncrypting(
    std::unique_ptr<FileWriterBase> file_writer)
    : FileWriterDecoratorBase(std::move(file_writer)),
      m_cipher{EVP_aes_256_cbc()} {}

FileWriterEncrypting::~FileWriterEncrypting() {
  if (m_ctx != nullptr) {
    ERR_clear_error();
    EVP_CIPHER_CTX_free(m_ctx);
    m_ctx = nullptr;
  }
}

bool FileWriterEncrypting::init() noexcept {
  if (m_cipher == nullptr) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "EVP_aes_256_cbc init failed");
    return false;
  }

  m_key = std::make_unique<unsigned char[]>(kEvpKeyLength);
  m_iv = std::make_unique<unsigned char[]>(EVP_MAX_IV_LENGTH);

  if (m_key == nullptr || m_iv == nullptr) {
    return false;
  }

  m_out_buff = std::make_unique<unsigned char[]>(kEncryptChunkSize + EVP_CIPHER_block_size(m_cipher));

  if (m_out_buff == nullptr) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "Failed to init out buffer");
    return false;
  }

  return FileWriterDecoratorBase::init();
}

bool FileWriterEncrypting::open() noexcept {
  std::string keyring_key_id = SysVars::get_encryption_password_id();
  std::string keyring_password;

  if (!audit_keyring::get_encryption_password(keyring_key_id,
                                              keyring_password)) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to fetch password for id %s",
                    keyring_key_id.c_str());
    return false;
  }

  if (keyring_password.empty()) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "Empty password for id %s",
                    keyring_key_id.c_str());
    return false;
  }

  // Derive key and default iv concatenated into a temporary buffer
  unsigned char tmp_key_iv[kEvpKeyLength + EVP_MAX_IV_LENGTH];
  auto ik_len = EVP_CIPHER_get_key_length(m_cipher);
  auto iv_len = EVP_CIPHER_get_iv_length(m_cipher);
  const int iterations = 1;

  if (!PKCS5_PBKDF2_HMAC(keyring_password.data(), keyring_password.size(),
                         nullptr, 0, iterations, EVP_sha256(), ik_len + iv_len,
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

  if (EVP_EncryptInit_ex(m_ctx, m_cipher, nullptr, m_key.get(), m_iv.get()) !=
      1) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "EVP_EncryptInit error: %s",
                    ERR_error_string(ERR_peek_error(), nullptr));
    ERR_clear_error();
    return false;
  }

  return FileWriterDecoratorBase::open();
}

void FileWriterEncrypting::close() noexcept {
  int out_size = 0;

  if (EVP_EncryptFinal_ex(m_ctx, m_out_buff.get(), &out_size) != 1) {
    LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "EVP_EncryptFinal error: %s",
                    ERR_error_string(ERR_peek_error(), nullptr));
  }

  if (out_size > 0) {
    FileWriterDecoratorBase::write(
        reinterpret_cast<const char *>(m_out_buff.get()), out_size);
  }

  ERR_clear_error();
  EVP_CIPHER_CTX_free(m_ctx);
  m_ctx = nullptr;

  FileWriterDecoratorBase::close();
}

void FileWriterEncrypting::write(const char *record, size_t size) noexcept {
  size_t encrypted_size = 0;

  auto cleanup_guard = create_scope_guard([&] { ERR_clear_error(); });

  while (encrypted_size < size) {
    int out_size = 0;
    size_t chunk_size = size - encrypted_size > kEncryptChunkSize ? kEncryptChunkSize : size - encrypted_size;

    if (EVP_EncryptUpdate(m_ctx, m_out_buff.get(), &out_size,
                          reinterpret_cast<const unsigned char *>(record + encrypted_size),
                          chunk_size) != 1) {
      LogPluginErrMsg(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                      "EVP_EncryptUpdate error: %s",
                      ERR_error_string(ERR_peek_error(), nullptr));
      return;
    }

    if (out_size > 0) {
      FileWriterDecoratorBase::write(
          reinterpret_cast<const char *>(m_out_buff.get()), out_size);
    }

    encrypted_size += chunk_size;
  }
}

}  // namespace audit_log_filter::log_writer
