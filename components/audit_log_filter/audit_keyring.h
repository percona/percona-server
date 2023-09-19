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

#ifndef AUDIT_LOG_FILTER_KEYRING_H_INCLUDED
#define AUDIT_LOG_FILTER_KEYRING_H_INCLUDED

#include "components/audit_log_filter/audit_encryption.h"

#include <memory>
#include <string>
#include <vector>

namespace audit_log_filter::audit_keyring {

/**
 * @brief Check if keyring component is initialized.
 *
 * @return true in case keyring component is initialized,
 *         false otherwise
 */
bool check_keyring_initialized() noexcept;

/**
 * @brief Check/generate initial log encryption options
 *        (password, salt and so on).
 *
 * @return true in case initial options exist or were generated successfully,
 *         false otherwise
 */
bool check_generate_initial_encryption_options() noexcept;

/**
 * @brief Get log encryption options.
 *
 * @return Instance of encryption::EncryptionOptions corresponding to
 *         newest options
 */
std::unique_ptr<encryption::EncryptionOptions>
get_encryption_options() noexcept;

/**
 * @brief Get log encryption options.
 *
 * @param [in] options_id Encryption options ID
 * @return Instance of encryption::EncryptionOptions corresponding to
 *         provided options_id
 */
std::unique_ptr<encryption::EncryptionOptions> get_encryption_options(
    const std::string &options_id) noexcept;

/**
 * @brief Set log encryption options.
 *
 * Sets encryption options which consist of provided password and randomly
 * generated salt and iterations count for PBKDF function.
 *
 * @param [in] password Encryption password
 * @return true in case success, false otherwise
 */
bool set_encryption_options(const std::string &password) noexcept;

/**
 * @brief Remove outdated log encryption options.
 *
 * @param [in] remove_after_days Number of days after which archived passwords
 *                               are removed
 * @param [in] existing_log_names List of currently existing audit log names
 *                                (including rotated logs), archived password ID
 *                                is matched against these names to check if
 *                                password is still in use
 */
void prune_encryption_options(
    uint64_t remove_after_days,
    const std::vector<std::string> &existing_log_names) noexcept;

/**
 * @brief Extract timestamp from full keyring options ID.
 *
 * @param options_id Keyring options ID
 * @return Options ID timestamp
 */
std::string get_options_id_timestamp(const std::string &options_id) noexcept;

std::string get_options_id_for_file_name(const std::string &file_name) noexcept;

}  // namespace audit_log_filter::audit_keyring

#endif  // AUDIT_LOG_FILTER_KEYRING_H_INCLUDED
