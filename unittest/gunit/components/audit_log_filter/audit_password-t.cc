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

#include "components/audit_log_filter/audit_password.h"

#include <stdlib.h>  // random(), srandom()

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <set>
#include <string>
#include <vector>

namespace audit_log_filter::encryption {
namespace {

constexpr char kAlphanumerics[] =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
constexpr std::size_t kAlphanumericsSize = sizeof(kAlphanumerics) - 1;

// Bytes returned by fake_random_bytes(), one entry per call. Calls past the
// last entry fail.
std::vector<std::vector<unsigned char>> fake_calls;
std::size_t fake_call_count = 0;

int fake_random_bytes(unsigned char *buf, int num) {
  if (fake_call_count >= fake_calls.size()) {
    ++fake_call_count;
    return 0;
  }

  const auto &bytes = fake_calls[fake_call_count++];
  EXPECT_EQ(bytes.size(), static_cast<std::size_t>(num));
  std::copy(bytes.begin(), bytes.end(), buf);
  return 1;
}

template <typename Func>
std::vector<unsigned char> make_call(Func byte_at) {
  std::vector<unsigned char> bytes(kGeneratedPasswordLength);
  for (std::size_t i = 0; i < bytes.size(); ++i) {
    bytes[i] = static_cast<unsigned char>(byte_at(i));
  }
  return bytes;
}

class AuditPasswordFake : public ::testing::Test {
 protected:
  void SetUp() override {
    fake_calls.clear();
    fake_call_count = 0;
  }
};

TEST(AuditPassword, Format) {
  std::string password;
  ASSERT_TRUE(generate_password(password));
  EXPECT_EQ(password.size(), kGeneratedPasswordLength);
  EXPECT_EQ(password.find_first_not_of(kAlphanumerics), std::string::npos)
      << password;
}

// 1000 passwords hold 126000 characters, the chance that any of the 62
// alphanumerics is missing is below 62 * (61/62)^126000.
TEST(AuditPassword, UsesAllAlphanumerics) {
  const std::set<char> expected(std::begin(kAlphanumerics),
                                std::end(kAlphanumerics) - 1);
  std::set<char> seen;
  std::string password;

  for (int i = 0; i < 1000; ++i) {
    ASSERT_TRUE(generate_password(password));
    seen.insert(password.begin(), password.end());
  }

  EXPECT_EQ(seen, expected);
}

// The password must not depend on the state of the libc generator.
TEST(AuditPassword, IndependentOfLibcSeed) {
  std::string first;
  std::string second;

  srand(1);
  srandom(1);
  ASSERT_TRUE(generate_password(first));

  srand(1);
  srandom(1);
  ASSERT_TRUE(generate_password(second));

  EXPECT_NE(first, second);
}

// 256 % 62 = 8, bytes 248-255 would make '0'-'7' more likely and are skipped,
// byte 247 is the last one used.
TEST_F(AuditPasswordFake, SkipsBiasedBytes) {
  fake_calls.push_back(make_call([](std::size_t i) { return 248 + i % 8; }));
  fake_calls.push_back(make_call([](std::size_t i) { return 247 - i; }));

  std::string expected;
  for (std::size_t i = 0; i < kGeneratedPasswordLength; ++i) {
    expected.push_back(kAlphanumerics[(247 - i) % kAlphanumericsSize]);
  }

  std::string password;
  ASSERT_TRUE(generate_password(password, fake_random_bytes));
  EXPECT_EQ(password, expected);
  EXPECT_EQ(fake_call_count, 2U);
}

TEST_F(AuditPasswordFake, FailsOnFirstCall) {
  std::string password = "stale";
  EXPECT_FALSE(generate_password(password, fake_random_bytes));
  EXPECT_TRUE(password.empty());
  EXPECT_EQ(fake_call_count, 1U);
}

// The first call fills half of the password, the second one fails, no partial
// password is returned.
TEST_F(AuditPasswordFake, FailsOnLaterCall) {
  fake_calls.push_back(
      make_call([](std::size_t i) { return i % 2 == 0 ? 255 : i; }));

  std::string password;
  EXPECT_FALSE(generate_password(password, fake_random_bytes));
  EXPECT_TRUE(password.empty());
  EXPECT_EQ(fake_call_count, 2U);
}

}  // namespace
}  // namespace audit_log_filter::encryption
