/* Copyright (c) 2021 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include <gtest/gtest.h>

#include "sql/mysqld.h"  // system_charset_info
#include "sql/mysqld_cs.h"
#include "sql/rpl_event_ctx.h"

#define INSTANCE Rpl_event_ctx::get_instance()

namespace rpl_event_ctx_unittest {

class RplEventCtxTest : public ::testing::Test {
 protected:
  RplEventCtxTest() = default;

  void SetUp() final {
    // Save global settings.
    m_charset = system_charset_info;
    system_charset_info = &my_charset_utf8mb3_bin;
    EXPECT_TRUE(system_charset_info != nullptr);
  }

  void TearDown() final {
    // Restore global settings.
    system_charset_info = m_charset;
  }

 private:
  CHARSET_INFO *m_charset{nullptr};
};

TEST_F(RplEventCtxTest, CmdLineProcessing) {
  std::string value;
  std::ostringstream os;

  /*
    Negative cases - Argument processing
  */
  EXPECT_EQ(1, INSTANCE.process_argument("", os));
  EXPECT_EQ(0, strcmp(os.str().c_str(), "Cannot process empty pattern."));

  EXPECT_EQ(1, INSTANCE.process_argument(".", os));
  EXPECT_EQ(0, strcmp(os.str().c_str(),
                      "The pattern doesn't follow the format "
                      "<db_pattern>.<event_pattern>."));

  EXPECT_EQ(1, INSTANCE.process_argument("..", os));
  EXPECT_EQ(0, strcmp(os.str().c_str(),
                      "The pattern doesn't follow the format "
                      "<db_pattern>.<event_pattern>."));

  EXPECT_EQ(1, INSTANCE.process_argument("dbname", os));
  EXPECT_EQ(0, strcmp(os.str().c_str(),
                      "The pattern doesn't follow the format "
                      "<db_pattern>.<event_pattern>."));

  EXPECT_EQ(1, INSTANCE.process_argument("%", os));
  EXPECT_EQ(0, strcmp(os.str().c_str(),
                      "The pattern doesn't follow the format "
                      "<db_pattern>.<event_pattern>."));

  EXPECT_EQ(1, INSTANCE.process_argument(".dbname.", os));
  EXPECT_EQ(0, strcmp(os.str().c_str(),
                      "The pattern doesn't follow the format "
                      "<db_pattern>.<event_pattern>."));

  EXPECT_EQ(1, INSTANCE.process_argument("dbname..", os));
  EXPECT_EQ(0, strcmp(os.str().c_str(),
                      "The pattern has multiple dot characters. Please restart "
                      "with format <db_pattern>.<event_pattern>."));

  EXPECT_EQ(1, INSTANCE.process_argument("dbname.event.random", os));
  EXPECT_EQ(0, strcmp(os.str().c_str(),
                      "The pattern has multiple dot characters. Please restart "
                      "with format <db_pattern>.<event_pattern>."));

  EXPECT_EQ(1, INSTANCE.process_argument("channel:dbname.event", os));
  EXPECT_EQ(0, strcmp(os.str().c_str(),
                      "This server doesn't support per-channel "
                      "--replica-enable-event feature."));

  EXPECT_EQ(1, INSTANCE.process_argument("dname.channel:event", os));
  EXPECT_EQ(0, strcmp(os.str().c_str(),
                      "This server doesn't support per-channel "
                      "--replica-enable-event feature."));

  /* Reset the output stream before further testing. */
  os.str("");

  /*
    Positive cases - Argument processing
  */

  /* Verify that the list is empty. */
  INSTANCE.get_events_wild_list(value);
  EXPECT_EQ(0, strcmp(value.c_str(), ""));

  /* The below operation should be successful. */
  EXPECT_EQ(0, INSTANCE.process_argument("db%.event", os));
  EXPECT_TRUE(os.str().empty());

  /* Verify that the list has one item. */
  INSTANCE.get_events_wild_list(value);
  EXPECT_EQ(0, strcmp(value.c_str(), "db%.event"));

  /* The below operation should be successful. */
  EXPECT_EQ(0, INSTANCE.process_argument("db.event%", os));
  EXPECT_TRUE(os.str().empty());

  /* Verify that the list has two items. */
  INSTANCE.get_events_wild_list(value);
  EXPECT_EQ(0, strcmp(value.c_str(), "db%.event,db.event%"));

  /* The below operation should be successful. */
  EXPECT_EQ(0, INSTANCE.process_argument("t%est.e%vent", os));
  EXPECT_TRUE(os.str().empty());

  /* Verify that the list has three items. */
  INSTANCE.get_events_wild_list(value);
  EXPECT_EQ(0, strcmp(value.c_str(), "db%.event,db.event%,t%est.e%vent"));

  /*
    Negative cases - Pattern matching
  */
  EXPECT_FALSE(INSTANCE.event_needs_reenable("db123", "event567"));
  EXPECT_FALSE(INSTANCE.event_needs_reenable("db", "evet"));
  EXPECT_FALSE(INSTANCE.event_needs_reenable("db1", "event1"));
  EXPECT_FALSE(INSTANCE.event_needs_reenable("db", "d_event"));
  EXPECT_FALSE(INSTANCE.event_needs_reenable("db1", "1event"));
  EXPECT_FALSE(INSTANCE.event_needs_reenable("testdb", "event2"));
  EXPECT_FALSE(INSTANCE.event_needs_reenable("t1estdb", "event1"));
  EXPECT_FALSE(INSTANCE.event_needs_reenable("test1", "1event"));

  /*
    Positive cases - Pattern matching
  */
  EXPECT_TRUE(INSTANCE.event_needs_reenable("db", "event"));
  EXPECT_TRUE(INSTANCE.event_needs_reenable("db1", "event"));
  EXPECT_TRUE(INSTANCE.event_needs_reenable("db", "event1"));
  EXPECT_TRUE(INSTANCE.event_needs_reenable("test", "event"));
  EXPECT_TRUE(INSTANCE.event_needs_reenable("ttest", "evvent"));
}

}  // namespace rpl_event_ctx_unittest
