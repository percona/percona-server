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

#include "bulk_load_component.h"

#include <utility>

#include "mysql/components/service_implementation.h"
#include "mysql/components/services/log_builtins.h"

#include "bulk_loader.h"
#include "bulk_loader/bulk_loader_abstract.h"

REQUIRES_SERVICE_PLACEHOLDER(log_builtins);
REQUIRES_SERVICE_PLACEHOLDER(log_builtins_string);

SERVICE_TYPE(log_builtins) *log_bi;
SERVICE_TYPE(log_builtins_string) *log_bs;

namespace Bulk_load {

DEFINE_METHOD(Bulk_loader *, create_bulk_loader,
              (THD *thd, my_thread_id connection_id, const TABLE *table,
               Bulk_source src, const CHARSET_INFO *charset)) {
    return Bulk_load::create_loader(thd, connection_id, table, src, charset);
}

DEFINE_METHOD(void, set_string,
              (Bulk_loader *loader, Bulk_string type, std::string value)) {
  static_cast<Bulk_loader_impl *>(loader)->set_string(type, std::move(value));
}

DEFINE_METHOD(void, set_char,
              (Bulk_loader *loader, Bulk_char type, unsigned char value)) {
  static_cast<Bulk_loader_impl *>(loader)->set_char(type, value);
}

DEFINE_METHOD(void, set_size,
              (Bulk_loader *loader, Bulk_size type, size_t value)) {
  static_cast<Bulk_loader_impl *>(loader)->set_size(type, value);
}

DEFINE_METHOD(void, set_condition,
              (Bulk_loader *loader, Bulk_condition type, bool value)) {
  static_cast<Bulk_loader_impl *>(loader)->set_condition(type, value);
}

DEFINE_METHOD(void, set_compression_algorithm,
              (Bulk_loader *loader, Bulk_compression_algorithm algorithm)) {
  static_cast<Bulk_loader_impl *>(loader)->set_compression_algorithm(algorithm);
}

DEFINE_METHOD(bool, load, (Bulk_loader *loader, size_t &affected_rows)) {
  return static_cast<Bulk_loader_impl *>(loader)->load(affected_rows);
}

DEFINE_METHOD(void, drop_bulk_loader, (THD *thd, Bulk_loader *loader)) {
  Bulk_load::drop_loader(thd, static_cast<Bulk_loader_impl *>(loader));
}

}  // namespace Bulk_load

static mysql_service_status_t component_init() {
  log_bi = mysql_service_log_builtins;
  log_bs = mysql_service_log_builtins_string;

  return 0;
}

static mysql_service_status_t component_deinit() {
  return 0;
}

// clang-format off
BEGIN_SERVICE_IMPLEMENTATION(component_bulk_load, bulk_load_driver)
Bulk_load::create_bulk_loader, Bulk_load::set_string,
  Bulk_load::set_char, Bulk_load::set_size,
  Bulk_load::set_condition, Bulk_load::set_compression_algorithm,
  Bulk_load::load, Bulk_load::drop_bulk_loader,
  END_SERVICE_IMPLEMENTATION();

BEGIN_COMPONENT_PROVIDES(component_bulk_load)
PROVIDES_SERVICE(component_bulk_load, bulk_load_driver),
//  PROVIDES_SERVICE(component_bulk_load, log_builtins),
//  PROVIDES_SERVICE(component_bulk_load, log_builtins_string),
  END_COMPONENT_PROVIDES();

BEGIN_COMPONENT_REQUIRES(component_bulk_load)
REQUIRES_SERVICE(registry), REQUIRES_SERVICE(log_builtins),
  REQUIRES_SERVICE(log_builtins_string),
  // REQUIRES_SERVICE(bulk_data_load),
  //REQUIRES_SERVICE(bulk_data_convert),
  END_COMPONENT_REQUIRES();

BEGIN_COMPONENT_METADATA(component_bulk_load)
  METADATA("mysql.author", "Percona Corporation"),
  METADATA("mysql.license", "GPL"),
END_COMPONENT_METADATA();

DECLARE_COMPONENT(component_bulk_load, "component_bulk_load")
  component_init, component_deinit,
  END_DECLARE_COMPONENT();

DECLARE_LIBRARY_COMPONENTS &COMPONENT_REF(component_bulk_load)
    END_DECLARE_LIBRARY_COMPONENTS

// clang-format on
