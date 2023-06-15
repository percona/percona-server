/* Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#ifndef MYSQLPP_UDF_REGISTRATION_HPP
#define MYSQLPP_UDF_REGISTRATION_HPP

#include <mysqlpp/common_types.hpp>
#include <mysqlpp/udf_context.hpp>
#include <mysqlpp/udf_exception.hpp>

#include <mysql/components/component_implementation.h>
#include <mysql/components/services/udf_registration.h>

extern REQUIRES_SERVICE_PLACEHOLDER(udf_registration);

namespace mysqlpp {
struct udf_info {
  const char *name;
  Item_result return_type;
  Udf_func_any func;
  Udf_func_init init_func;
  Udf_func_deinit deinit_func;
};

template <typename KNOWN_T, typename REGISTERED_T>
void register_udfs(KNOWN_T const &known_udfs, REGISTERED_T &registered_udfs) {
  std::size_t index = 0U;
  for (const auto &element : known_udfs) {
    if (!registered_udfs.test(index)) {
      if (mysql_service_udf_registration->udf_register(
              element.name, element.return_type, element.func,
              element.init_func, element.deinit_func) == 0)
        registered_udfs.set(index);
    }
    ++index;
  }
}

static constexpr std::size_t max_unregister_attempts = 10;
static constexpr auto unregister_sleep_interval = std::chrono::seconds(1);

template <typename KNOWN_T, typename REGISTERED_T>
void unregister_udfs(KNOWN_T const &known_udfs, REGISTERED_T &registered_udfs) {
  int was_present = 0;

  std::size_t index = 0U;

  for (const auto &element : known_udfs) {
    if (registered_udfs.test(index)) {
      std::size_t attempt = 0;
      mysql_service_status_t status = 0;
      while (attempt < max_unregister_attempts &&
             (status = mysql_service_udf_registration->udf_unregister(
                  element.name, &was_present)) != 0 &&
             was_present != 0) {
        std::this_thread::sleep_for(unregister_sleep_interval);
        ++attempt;
      }
      if (status == 0) registered_udfs.reset(index);
    }
    ++index;
  }
}
}  // namespace mysqlpp

#define DECLARE_UDF_INFO(NAME, TYPE)                               \
  mysqlpp::udf_info {                                              \
    #NAME, TYPE, (Udf_func_any)&NAME, &NAME##_init, &NAME##_deinit \
  }

#endif
