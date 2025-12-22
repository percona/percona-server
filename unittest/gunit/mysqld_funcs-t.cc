/* Copyright (c) 2019, 2025, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is designed to work with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have either included with
   the program or referenced in the documentation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <gtest/gtest.h>
#include <string.h>
#include "my_config.h"
#include "sql/manifest_file_option_parser_helper.h"

#ifdef HAVE_GETPWNAM
#include "my_getpwnam.h"  // PasswdValue
#endif                    /* HAVE_GETPWNAM */

// Unit tests for functions in mysqld.cc.
namespace mysqld_funcs_unit_test {
#ifdef HAVE_GETPWNAM
PasswdValue check_user_drv(const char *user);

TEST(MysqldFuncs, CheckUser) {
  EXPECT_TRUE(check_user_drv("root").IsVoid());

  if (geteuid() == 0) {
    // Running as root
    EXPECT_FALSE(check_user_drv("0").IsVoid());
    EXPECT_FALSE(check_user_drv("1").IsVoid());
    EXPECT_FALSE(check_user_drv("bin").IsVoid());
  } else {
    // These would trigger unireg_abort if run as root, and
    // unireg_abort currently triggers crash if run in a unit test
    EXPECT_TRUE(check_user_drv(nullptr).IsVoid());
    EXPECT_TRUE(check_user_drv("thereisnosuchuser___").IsVoid());
    EXPECT_TRUE(check_user_drv("0").IsVoid());
    EXPECT_TRUE(check_user_drv("0abc").IsVoid());
    EXPECT_TRUE(check_user_drv("1").IsVoid());
    EXPECT_TRUE(check_user_drv("bin").IsVoid());
  }
}
#endif /* HAVE_GETPWNAM */

constexpr const char *lorem_ipsum_510 =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed ut "
    "perspiciatis unde omnis iste natus error sit voluptatem accusantium "
    "doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo "
    "inventore veritatis et quasi architecto beatae vitae dicta sunt "
    "explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut "
    "odit aut fugit, sed quia consequuntur magni dolores eos qui ratione "
    "voluptatem sequi nesciunt. Neque porro quisquam.";

TEST(MysqldFuncs, CheckManifestFileOptionParserHelper) {
  const char *argv[] = {"path", "--datadir=d", "--plugin-dir=p"};
  constexpr int argc = std::size(argv);

  strncpy(mysql_real_data_home, lorem_ipsum_510,
          std::size(mysql_real_data_home));
  strncpy(opt_plugin_dir, lorem_ipsum_510, std::size(opt_plugin_dir));

  {
    Manifest_file_option_parser_helper obj{argc, const_cast<char **>(argv)};
#ifdef _WIN32
    EXPECT_TRUE(strcmp(mysql_real_data_home, "d\\") == 0);
    EXPECT_TRUE(strcmp(opt_plugin_dir, "\\p\\") == 0);
#else
    EXPECT_TRUE(strcmp(mysql_real_data_home, "d/") == 0);
    EXPECT_TRUE(strcmp(opt_plugin_dir, "/p/") == 0);
#endif
  }

  // mysql_real_data_home and opt_plugin_dir must be preserved
  EXPECT_TRUE(strcmp(mysql_real_data_home, lorem_ipsum_510) == 0);
  EXPECT_TRUE(strcmp(opt_plugin_dir, lorem_ipsum_510) == 0);
}

}  // namespace mysqld_funcs_unit_test
