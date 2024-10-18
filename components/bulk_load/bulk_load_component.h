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

#include <mysql/components/component_implementation.h>
#include <mysql/components/services/bulk_load_service.h>
#include <mysql/components/services/bulk_data_service.h>

#include "bulk_loader/bulk_loader_abstract.h"

extern REQUIRES_SERVICE_PLACEHOLDER(registry);
//extern REQUIRES_SERVICE_PLACEHOLDER(bulk_data_load);
//extern REQUIRES_SERVICE_PLACEHOLDER(bulk_data_convert);

namespace Bulk_load {

DEFINE_METHOD(Bulk_loader *, create_bulk_loader,
              (THD *thd, my_thread_id connection_id, const TABLE *table,
               Bulk_source src, const CHARSET_INFO *charset));

DEFINE_METHOD(void, set_string,
              (Bulk_loader_impl *loader, Bulk_string type, std::string value));

DEFINE_METHOD(void, set_char,
              (Bulk_loader_impl *loader, Bulk_char type, unsigned char value));

DEFINE_METHOD(void, set_size,
              (Bulk_loader_impl *loader, Bulk_size type, size_t value));

DEFINE_METHOD(void, set_condition,
              (Bulk_loader_impl *loader, Bulk_condition type, bool value));

DEFINE_METHOD(void, set_compression_algorithm,
              (Bulk_loader_impl *loader, Bulk_compression_algorithm algorithm));

DEFINE_METHOD(bool, load, (Bulk_loader_impl *loader, size_t &affected_rows));

DEFINE_METHOD(void, drop_bulk_loader, (THD *thd, Bulk_loader_impl *loader));

}  // namespace Bulk_load
