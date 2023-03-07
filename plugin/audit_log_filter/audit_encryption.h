/* Copyright (c) 2022 Percona LLC and/or its affiliates. All rights reserved.

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

#ifndef AUDIT_LOG_FILTER_ENCRYPTION_H_INCLUDED
#define AUDIT_LOG_FILTER_ENCRYPTION_H_INCLUDED

#include <openssl/evp.h>

#include <memory>
#include <string>
#include <vector>

namespace audit_log_filter::encryption {

using SaltType = std::vector<unsigned char>;

class EncryptionOptions {
 private:
  EncryptionOptions(std::string password, SaltType salt,
                    std::size_t iterations);

 public:
  EncryptionOptions();

  /**
   * @brief Generate EncryptionOptions instance
   *
   * @param password Encryption password
   * @return Instance of EncryptionOptions
   */
  static std::unique_ptr<EncryptionOptions> generate(
      const std::string &password) noexcept;

  /**
   * @brief Restore EncryptionOptions instance from its JSON representation
   *
   * @param json_string JSON string representing EncryptionOptions data
   * @return Instance of EncryptionOptions
   */
  static std::unique_ptr<EncryptionOptions> from_json_string(
      const std::string &json_string) noexcept;

  /**
   * @brief Get iterations count
   *
   * @return Iterations count
   */
  [[nodiscard]] std::size_t get_iterations() const noexcept;

  /**
   * @brief Get encryption password
   *
   * @return Encryption password
   */
  [[nodiscard]] std::string const &get_password() const noexcept;

  /**
   * @brief Get encryption salt
   *
   * @return Encryption salt
   */
  [[nodiscard]] SaltType const &get_salt() const noexcept;

  /**
   * @brief Check if EncryptionOptions instance is valid
   *
   * @return true in case EncryptionOptions instance is valid, false otherwise
   */
  [[nodiscard]] bool check_valid() const noexcept;

  /**
   * @brief Convert EncryptionOptions instance to JSON string
   *
   * @return JSON string representing EncryptionOptions instance
   */
  [[nodiscard]] std::string to_json_string() const noexcept;

 private:
  std::string m_password;
  SaltType m_salt;
  std::size_t m_iterations;
};

}  // namespace audit_log_filter::encryption

#endif  // AUDIT_LOG_FILTER_ENCRYPTION_H_INCLUDED
