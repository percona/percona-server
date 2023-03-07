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

#include "plugin/audit_log_filter/audit_encryption.h"

#include "plugin/audit_log_filter/sys_vars.h"

#include "my_rapidjson_size_t.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include <openssl/rand.h>

#include <boost/algorithm/hex.hpp>

#include <algorithm>
#include <memory>
#include <mutex>
#include <random>
#include <utility>

namespace audit_log_filter::encryption {
namespace {

std::size_t get_random_iterations() noexcept {
  static std::mutex m;
  static std::random_device r;
  static std::default_random_engine el(r());

  std::lock_guard<std::mutex> guard{m};

  const auto mean = SysVars::get_key_derivation_iter_count_mean();
  std::uniform_int_distribution<std::size_t> dist(static_cast<int>(mean * 0.9),
                                                  static_cast<int>(mean * 1.1));

  return dist(el);
}

SaltType get_random_salt() noexcept {
  SaltType salt(PKCS5_SALT_LEN);
  assert(salt.size() == PKCS5_SALT_LEN);
  RAND_bytes(salt.data(), PKCS5_SALT_LEN);
  return salt;
}

std::string make_json_string(const std::string &password,
                             const std::string &salt,
                             const std::size_t iterations) {
  rapidjson::Document doc;
  doc.SetObject();
  doc.AddMember(
      "password",
      rapidjson::Value()
          .SetString(password.c_str(), password.length(), doc.GetAllocator())
          .Move(),
      doc.GetAllocator());
  doc.AddMember("salt",
                rapidjson::Value()
                    .SetString(salt.c_str(), salt.length(), doc.GetAllocator())
                    .Move(),
                doc.GetAllocator());
  doc.AddMember("iterations", rapidjson::Value().SetInt(iterations).Move(),
                doc.GetAllocator());

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);

  return buffer.GetString();
}

}  // namespace

EncryptionOptions::EncryptionOptions() : EncryptionOptions("", {}, 0) {}

EncryptionOptions::EncryptionOptions(std::string password, SaltType salt,
                                     std::size_t iterations)
    : m_password{std::move(password)},
      m_salt{std::move(salt)},
      m_iterations{iterations} {}

std::unique_ptr<EncryptionOptions> EncryptionOptions::generate(
    const std::string &password) noexcept {
  return std::unique_ptr<EncryptionOptions>(new EncryptionOptions{
      password, get_random_salt(), get_random_iterations()});
}

std::unique_ptr<EncryptionOptions> EncryptionOptions::from_json_string(
    const std::string &json_string) noexcept {
  rapidjson::Document doc;
  doc.Parse(json_string.c_str());

  if (doc.HasParseError() || !doc.IsObject() || !doc.HasMember("password") ||
      !doc["password"].IsString() || !doc.HasMember("iterations") ||
      !doc["iterations"].IsUint() || !doc.HasMember("salt") ||
      !doc["salt"].IsString()) {
    return {};
  }

  SaltType salt;
  const std::string salt_hex_chars(doc["salt"].GetString());
  boost::algorithm::unhex(salt_hex_chars, std::back_inserter(salt));

  assert(salt.size() == PKCS5_SALT_LEN);

  return std::unique_ptr<EncryptionOptions>(new EncryptionOptions{
      doc["password"].GetString(), salt, doc["iterations"].GetUint()});
}

std::size_t EncryptionOptions::get_iterations() const noexcept {
  return m_iterations;
}

std::string const &EncryptionOptions::get_password() const noexcept {
  return m_password;
}

SaltType const &EncryptionOptions::get_salt() const noexcept { return m_salt; }

bool EncryptionOptions::check_valid() const noexcept {
  return m_iterations > 0 && !m_password.empty() && !m_salt.empty();
}

std::string EncryptionOptions::to_json_string() const noexcept {
  std::string salt_hex_string;
  boost::algorithm::hex(m_salt.cbegin(), m_salt.cend(),
                        std::back_inserter(salt_hex_string));

  return make_json_string(m_password, salt_hex_string, m_iterations);
}

}  // namespace audit_log_filter::encryption
