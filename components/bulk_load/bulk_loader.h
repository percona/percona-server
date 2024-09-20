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

#include "bulk_loader/bulk_loader_abstract.h"

#include "mysql/components/services/bulk_load_service.h"

namespace Bulk_load {

Bulk_loader_impl *create_loader(THD *thd, my_thread_id connection_id,
                                const TABLE *table, Bulk_source src,
                                const CHARSET_INFO *charset) noexcept;

void drop_loader(THD *thd, Bulk_loader_impl *loader) noexcept;

}  // namespace Bulk_load
