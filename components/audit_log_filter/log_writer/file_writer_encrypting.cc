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

#include "components/audit_log_filter/audit_error_log.h"
#include "components/audit_log_filter/audit_keyring.h"
#include "components/audit_log_filter/sys_vars.h"

#include <openssl/err.h>

#include <include/scope_guard.h>
#include <cstring>
#include <string>

namespace audit_log_filter::log_writer {
namespace {

const size_t kEvpKeyLength = 32;
const size_t kEncryptChunkSize = 1024 * 1024;
const char magic[] = "Salted__";

}  // namespace

FileWriterEncrypting::FileWriterEncrypting(
    std::unique_ptr<FileWriterBase> file_writer)
    : FileWriterDecoratorBase(std::move(file_writer)),
      m_cipher{EVP_aes_256_cbc()},
      m_ctx{nullptr},
      m_key{nullptr},
      m_iv{nullptr},
      m_out_buff{nullptr} {}

FileWriterEncrypting::~FileWriterEncrypting() {
  if (m_ctx != nullptr) {
    ERR_clear_error();
    EVP_CIPHER_CTX_free(m_ctx);
    m_ctx = nullptr;
  }
}

bool FileWriterEncrypting::init() noexcept {
  if (m_cipher == nullptr) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "EVP_aes_256_cbc init failed");
    return false;
  }

  m_key = std::make_unique<unsigned char[]>(kEvpKeyLength);
  m_iv = std::make_unique<unsigned char[]>(EVP_MAX_IV_LENGTH);

  if (m_key == nullptr || m_iv == nullptr) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init key buffer");
    return false;
  }

  m_out_buff = std::make_unique<unsigned char[]>(
      kEncryptChunkSize + EVP_CIPHER_block_size(m_cipher));

  if (m_out_buff == nullptr) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to init out buffer");
    return false;
  }

  return FileWriterDecoratorBase::init();
}

bool FileWriterEncrypting::open() noexcept {
  assert(m_key != nullptr && m_iv != nullptr && m_out_buff != nullptr);

  std::string keyring_key_id = SysVars::get_encryption_options_id();
  const auto options = audit_keyring::get_encryption_options(keyring_key_id);

  if (options == nullptr || !options->check_valid()) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Failed to fetch options for id %s",
                    keyring_key_id.c_str());
    return false;
  }

  const auto &keyring_password = options->get_password();
  const auto keyring_iterations = options->get_iterations();
  const auto &keyring_salt = options->get_salt();

  if (keyring_password.empty()) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "Empty password for id %s",
                    keyring_key_id.c_str());
    return false;
  }

  if (keyring_iterations < 1) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "Bad iterations count for id %s", keyring_key_id.c_str());
    return false;
  }

  if (keyring_salt.empty()) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "Empty salt for id %s",
                    keyring_key_id.c_str());
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
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "PKCS5_PBKDF2_HMAC error: %s",
                    ERR_error_string(ERR_peek_error(), nullptr));
    return false;
  }

  memcpy(m_key.get(), tmp_key_iv, ik_len);
  memcpy(m_iv.get(), tmp_key_iv + ik_len, iv_len);

  m_ctx = EVP_CIPHER_CTX_new();
  if (m_ctx == nullptr) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "EVP_CIPHER_CTX_new failed");
    return false;
  }

  if (EVP_CipherInit_ex(m_ctx, m_cipher, nullptr, m_key.get(), m_iv.get(), 1) !=
      1) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
                    "EVP_CipherInit_ex error: %s",
                    ERR_error_string(ERR_peek_error(), nullptr));
    ERR_clear_error();
    EVP_CIPHER_CTX_free(m_ctx);
    m_ctx = nullptr;
    return false;
  }

  if (!FileWriterDecoratorBase::open()) {
    return false;
  }

  FileWriterDecoratorBase::write(magic, sizeof(magic) - 1);
  FileWriterDecoratorBase::write(
      reinterpret_cast<const char *>(keyring_salt.data()), keyring_salt.size());

  return true;
}

void FileWriterEncrypting::close() noexcept {
  int out_size = 0;

  if (EVP_EncryptFinal_ex(m_ctx, m_out_buff.get(), &out_size) != 1) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
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
    size_t chunk_size = size - encrypted_size > kEncryptChunkSize
                            ? kEncryptChunkSize
                            : size - encrypted_size;

    if (EVP_EncryptUpdate(
            m_ctx, m_out_buff.get(), &out_size,
            reinterpret_cast<const unsigned char *>(record + encrypted_size),
            chunk_size) != 1) {
      LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG,
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
