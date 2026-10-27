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

#include <initializer_list>

#include "storage/innobase/include/ut0log.h"
#include "storage/innobase/include/ut0stateful_latching_rules.h"

namespace innodb_ut0stateful_latching_rules_unittest {

/* The io_fix states of buf0buf.cc. */
enum io_fix_t { NONE, READ, WRITE, PIN };

using Rules = ut::Stateful_latching_rules<io_fix_t, 3, 4, 6>;

/* The rules of buf0buf.cc, with the states listed out of order: the rules must
not depend on the order. */
const Rules rules{
    {PIN, READ, NONE, WRITE},
    {
        {NONE, {0, 2}, READ},
        {READ, {0, 2}, NONE},
        {NONE, {0, 1, 2}, WRITE},
        {WRITE, {0, 1, 2}, NONE},
        {NONE, {0}, PIN},
        {PIN, {0}, NONE},
    },
};

Rules::latches_set_t latches(std::initializer_list<int> ids) {
  Rules::latches_set_t result;
  for (const int id : ids) {
    result[id] = true;
  }
  return result;
}

TEST(stateful_latching_rules, allowed_transitions) {
  rules.on_transition(NONE, READ, latches({0, 2}));
  rules.on_transition(WRITE, NONE, latches({0, 1, 2}));
  rules.on_transition(PIN, NONE, latches({0, 1}));
  rules.on_transition(READ, READ, latches({}));
}

TEST(stateful_latching_rules, latches_let_distinguish) {
  /* No edge into or out of WRITE is possible without latch #1. */
  rules.assert_latches_let_distinguish(latches({1}), {WRITE});
  rules.assert_latches_let_distinguish(latches({2}), {NONE, PIN}, {READ});
  rules.assert_latches_let_distinguish(latches({0}), {PIN});
}

TEST(stateful_latching_rulesDeathTest, disallowed_transitions) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  EXPECT_DEATH_IF_SUPPORTED(rules.on_transition(NONE, READ, latches({0})), "");
  EXPECT_DEATH_IF_SUPPORTED(
      rules.on_transition(READ, WRITE, latches({0, 1, 2})), "");
}

TEST(stateful_latching_rulesDeathTest, latches_do_not_let_distinguish) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  EXPECT_DEATH_IF_SUPPORTED(
      rules.assert_latches_let_distinguish(latches({2}), {PIN}), "");
  EXPECT_DEATH_IF_SUPPORTED(
      rules.assert_latches_let_distinguish(latches({}), {READ}), "");
}

}  // namespace innodb_ut0stateful_latching_rules_unittest
