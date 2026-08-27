/*****************************************************************************

Copyright (c) 2026, Percona Inc.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/**
@file vec/vec0index.cc
Per-index vector runtime.
*/

#include "vec0index.h"

#include "dict0mem.h"
#include "ut0new.h"

void vec_index_runtime_free(dict_index_t *index) {
  ut_ad(index != nullptr);
  if (index->vec == nullptr) return;

  /* Deleting through the base pointer; the virtual destructor is what
  makes that correct for a subtype allocated by an implementation. */
  ut::delete_(index->vec);
  index->vec = nullptr;
}
