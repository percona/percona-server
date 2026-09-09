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

/* Bug 1: innobase_build_col_map. The hidden columns are laid out
FTS_DOC_ID then percona_vec_aux_id, and the slot cursor has to advance
with the NEW table's hidden columns. The committed loop advanced it only
when the OLD table had FTS_DOC_ID, so old=vec-only plus new=FTS+vec
mapped vec onto slot 0, which is the new FTS_DOC_ID.

Modelled here rather than driven through ALTER because the mapper is
file-static; vector_alter_add_fulltext covers the SQL path. */
static size_t col_map_vec_slot(bool old_has_doc_id, bool new_has_doc_id) {
  size_t new_hidden_slot = 0;
  if (old_has_doc_id) {
    if (new_has_doc_id) new_hidden_slot++;
  } else if (new_has_doc_id) {
    new_hidden_slot++;
  }
  return new_hidden_slot;
}

TEST(vec0aux, ColMapVecSlotFollowsNewTableLayout) {
  /* old vec only, new FTS_DOC_ID + vec: vec is the second hidden column. */
  EXPECT_EQ(col_map_vec_slot(false, true), 1u);
  /* both already had FTS_DOC_ID: unchanged. */
  EXPECT_EQ(col_map_vec_slot(true, true), 1u);
  /* new table has no FTS_DOC_ID: vec is the only hidden column. */
  EXPECT_EQ(col_map_vec_slot(false, false), 0u);
  EXPECT_EQ(col_map_vec_slot(true, false), 0u);
}

}  // namespace innodb_vec0aux_unittest
