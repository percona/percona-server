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

#pragma once

#include "bulk_loader_abstract.h"

#include "components/bulk_load/data_stream/stream_local/data_stream_local.h"

namespace Bulk_load {

template <>
class Bulk_loader_base<Bulk_source::LOCAL> : public Bulk_loader_impl {
 public:
  Bulk_loader_base(THD *thd, my_thread_id connection_id,
                   const TABLE *table, const CHARSET_INFO *charset);

 private:
  std::unique_ptr<DataStreamAbstract> get_data_stream() noexcept override;
};

using Bulk_loader_local = Bulk_loader_base<Bulk_source::LOCAL>;

}  // namespace Bulk_load
