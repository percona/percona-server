/*****************************************************************************

Copyright (c) 1997, 2026, Oracle and/or its affiliates.

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

/** @file include/read0read.h
 Cursor read

 Created 2/16/1997 Heikki Tuuri
 *******************************************************/

#ifndef read0read_h
#define read0read_h

#include <stddef.h>
#include <algorithm>

#include "read0types.h"
#include "trx0types.h"
#include "univ.i"

/** The MVCC read view manager */
class MVCC {
 public:
  /** Constructor
  @param size           Number of views to pre-allocate */
  explicit MVCC(ulint size);

  /** Destructor.
  Free all the views in the m_free list */
  ~MVCC();

  /** Copy the donor's open view and insert the copy immediately after it.
  The caller holds trx_sys->mutex and the donor transaction mutex.
  @param[in,out] view  Receiver view to overwrite, or nullptr to allocate one
  @param[in] from_trx  Transaction that owns the donor view */
  void view_clone(ReadView *&view, trx_t *from_trx);

  /** Allocate and create a view.
  @param view   View owned by this class created for the caller. Must be
  freed by calling view_close()
  @param trx    Transaction instance of caller */
  void view_open(ReadView *&view, trx_t *trx);

  /**
  Close a transaction view. A caller without trx_sys->mutex may cache an
  ordinary view for reuse. A clone always returns to the free list.
  @param[in,out] view   Transaction's view (possibly already tagged closed)
  @param[in] own_mutex  Whether the caller holds trx_sys->mutex */
  void view_close(ReadView *&view, bool own_mutex);

  /** Build the purge snapshot from the oldest open view, or from the active
  transactions if there is no open view. The caller owns the destination.
  @param[out] view  Preallocated purge view */
  void clone_oldest_view(ReadView *view);

  /**
  @return the number of active views */
  ulint size() const;

  /**
  @return true for a non-null, untagged pointer. A cached view stays tagged
  until MVCC::view_open() opens it again. */
  static bool is_view_active(ReadView *view) {
    ut_a(view != reinterpret_cast<ReadView *>(0x1));

    return (view != nullptr && !(intptr_t(view) & 0x1));
  }

  /**
  Set the view creator transaction id. Note: This should be set only
  for views created by RW transactions.
  @param view   Set the creator trx id for this view
  @param id     Transaction id to set */
  static void set_view_creator_trx_id(ReadView *view, trx_id_t id) {
    ut_ad(id > 0);

    view->creator_trx_id(id);
  }

 private:
  /**
  Validates a read view list. */
  bool validate() const;

  /** Take a view from the free list or allocate one.
  Requires trx_sys->mutex. */
  inline ReadView *get_view();

 public:
  /** Return the oldest open view, or nullptr. Cached views stay linked so
  their owners can reuse them. Requires trx_sys->mutex. */
  ReadView *get_oldest_view() const;

 private:
  // Prevent copying
  MVCC(const MVCC &);
  MVCC &operator=(const MVCC &);

 private:
  typedef UT_LIST_BASE_NODE_T(ReadView, m_view_list) view_list_t;

  /** Free views ready for reuse. */
  view_list_t m_free;

  /** Open and cached views in descending order of m_low_limit_no.
  Purge searches from the tail for the oldest open view. */
  view_list_t m_views;
};

#endif /* read0read_h */
