/*****************************************************************************

Copyright (c) 2026, Percona Inc.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is designed to work with certain software (including
but not limited to OpenSSL) that is licensed under separate terms,
as designated in a particular file or component or in included license
documentation.  The authors of MySQL hereby grant you an additional
permission to link the program and your derivative works with the
separately licensed software that they have either included with
the program or referenced in the documentation.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

#include <gtest/gtest.h>
#include <cstring>

#include "vec0aux.h"

namespace innodb_vec0aux_unittest {

/* Bug 2: parse must reject trailing junk. MTR sibling:
   suite/percona/t/vector_aux_parse_trailing.test */
TEST(vec0aux, ParseRequiresNulAfterSecondId) {
  EXPECT_TRUE(vec_aux_is_aux_table_name("test/percona_vec_hnsw_1_2"));
  EXPECT_TRUE(vec_aux_is_aux_table_name("percona_vec_hnsw_1_2"));
  EXPECT_FALSE(vec_aux_is_aux_table_name("test/percona_vec_hnsw_1_2xyz"));
  EXPECT_FALSE(vec_aux_is_aux_table_name("test/percona_vec_hnsw_1_2_extra"));
}

}  // namespace innodb_vec0aux_unittest
