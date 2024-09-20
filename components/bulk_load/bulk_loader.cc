/* Copyright (c) 2024, Percona and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation. The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA */

#include "bulk_loader.h"

#include "bulk_loader/bulk_loader_local.h"
#include "bulk_loader/bulk_loader_s3.h"

namespace Bulk_load {

template <Bulk_source BulkSource>
Bulk_loader_impl *create_helper(THD *thd, my_thread_id connection_id,
                                const TABLE *table,
                                const CHARSET_INFO *charset) {
  return new Bulk_loader_base<BulkSource>(thd, connection_id, table, charset);
}

Bulk_loader_impl *create_loader(THD *thd, my_thread_id connection_id,
                                const TABLE *table, Bulk_source src,
                                const CHARSET_INFO *charset) noexcept {
  if (src == Bulk_source::OCI) {
    return nullptr;   // not supported
  }

  using CreateFunc = Bulk_loader_impl *(*)(THD *, my_thread_id, const TABLE *,
                                           const CHARSET_INFO *);
  static const CreateFunc funcs[static_cast<int>(Bulk_source::S3) + 1] = {
      create_helper<Bulk_source::LOCAL>,
      nullptr,
      create_helper<Bulk_source::S3>};
  return (*funcs[static_cast<int>(src)])(thd, connection_id, table, charset);
}

void drop_loader(THD *thd [[maybe_unused]],
                 Bulk_loader_impl *loader [[maybe_unused]]) noexcept {

}

}  // namespace Bulk_load
