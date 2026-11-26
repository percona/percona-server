/* Copyright (c) 2026 Percona LLC and/or its affiliates. All rights reserved.

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

#include <gtest/gtest.h>

#include "components/audit_log_filter/audit_encryption.h"
#include "components/audit_log_filter/sys_vars.h"

#include <string>

namespace audit_log_filter {

// EncryptionOptions::generate() takes the iterations count from the system
// variable
int SysVars::get_key_derivation_iter_count_mean() noexcept { return 1000; }

namespace encryption {
namespace {

std::string options_json(const std::string &salt) {
  return R"({"password":"pwd","salt":")" + salt + R"(","iterations":1000})";
}

TEST(AuditEncryptionOptions, RestoresGeneratedOptions) {
  const auto generated = EncryptionOptions::generate("pwd");
  ASSERT_NE(generated, nullptr);
  ASSERT_TRUE(generated->check_valid());

  const auto restored =
      EncryptionOptions::from_json_string(generated->to_json_string());
  ASSERT_NE(restored, nullptr);
  EXPECT_TRUE(restored->check_valid());
  EXPECT_EQ(restored->get_password(), generated->get_password());
  EXPECT_EQ(restored->get_salt(), generated->get_salt());
  EXPECT_EQ(restored->get_iterations(), generated->get_iterations());
}

TEST(AuditEncryptionOptions, RestoresSalt) {
  const auto options =
      EncryptionOptions::from_json_string(options_json("00017F80FFabCDef"));
  ASSERT_NE(options, nullptr);
  EXPECT_EQ(options->get_password(), "pwd");
  EXPECT_EQ(options->get_iterations(), 1000U);
  EXPECT_EQ(options->get_salt(),
            (SaltType{0x00, 0x01, 0x7F, 0x80, 0xFF, 0xAB, 0xCD, 0xEF}));
}

TEST(AuditEncryptionOptions, RejectsMalformedSalt) {
  // Not hexadecimal
  EXPECT_EQ(EncryptionOptions::from_json_string(options_json("zz")), nullptr);
  EXPECT_EQ(
      EncryptionOptions::from_json_string(options_json("0011223344556g77")),
      nullptr);
  // Half of a byte
  EXPECT_EQ(
      EncryptionOptions::from_json_string(options_json("001122334455667")),
      nullptr);
  // Other lengths
  EXPECT_EQ(EncryptionOptions::from_json_string(options_json("")), nullptr);
  EXPECT_EQ(EncryptionOptions::from_json_string(options_json("00112233445566")),
            nullptr);
  EXPECT_EQ(
      EncryptionOptions::from_json_string(options_json("001122334455667788")),
      nullptr);
  // Characters after a salt of the expected length and a '\0'
  EXPECT_EQ(EncryptionOptions::from_json_string(
                options_json(R"(0011223344556677\u0000tail)")),
            nullptr);
  EXPECT_EQ(EncryptionOptions::from_json_string(
                options_json(R"(0011223344556677\u0000)")),
            nullptr);
}

// A '\0' character does not end the password
TEST(AuditEncryptionOptions, RestoresPasswordWithNul) {
  const auto options = EncryptionOptions::from_json_string(
      R"({"password":"pw\u0000d","salt":"0011223344556677","iterations":1000})");
  ASSERT_NE(options, nullptr);
  EXPECT_TRUE(options->check_valid());
  EXPECT_EQ(options->get_password(), std::string("pw\0d", 4));

  const auto restored =
      EncryptionOptions::from_json_string(options->to_json_string());
  ASSERT_NE(restored, nullptr);
  EXPECT_EQ(restored->get_password(), options->get_password());
}

TEST(AuditEncryptionOptions, RejectsMalformedJson) {
  EXPECT_EQ(EncryptionOptions::from_json_string(""), nullptr);
  EXPECT_EQ(EncryptionOptions::from_json_string("not-json"), nullptr);
  EXPECT_EQ(EncryptionOptions::from_json_string("[]"), nullptr);
  EXPECT_EQ(EncryptionOptions::from_json_string(
                R"({"salt":"0011223344556677","iterations":1000})"),
            nullptr);
  EXPECT_EQ(EncryptionOptions::from_json_string(
                R"({"password":"pwd","iterations":1000})"),
            nullptr);
  EXPECT_EQ(EncryptionOptions::from_json_string(
                R"({"password":"pwd","salt":"0011223344556677"})"),
            nullptr);
  EXPECT_EQ(
      EncryptionOptions::from_json_string(
          R"({"password":1,"salt":"0011223344556677","iterations":1000})"),
      nullptr);
  EXPECT_EQ(EncryptionOptions::from_json_string(
                R"({"password":"pwd","salt":1,"iterations":1000})"),
            nullptr);
  EXPECT_EQ(
      EncryptionOptions::from_json_string(
          R"({"password":"pwd","salt":"0011223344556677","iterations":"x"})"),
      nullptr);
}

}  // namespace
}  // namespace encryption
}  // namespace audit_log_filter
