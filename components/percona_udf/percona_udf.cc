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

#include <array>
#include <bitset>

#include <boost/preprocessor/stringize.hpp>

#include <mysql/components/component_implementation.h>
#include <mysqlpp/udf_registration.hpp>

#include "percona_udf.h"

// defined as a macro because needed both raw and stringized
#define CURRENT_COMPONENT_NAME percona_udf
#define CURRENT_COMPONENT_NAME_STR BOOST_PP_STRINGIZE(CURRENT_COMPONENT_NAME)

REQUIRES_SERVICE_PLACEHOLDER(udf_registration);

#define DECLARE_UDF_INFO_NO_DEINIT(NAME, TYPE) \
  mysqlpp::udf_info { #NAME, TYPE, (Udf_func_any)&NAME, &NAME##_init, nullptr }

static const std::array known_udfs{
    DECLARE_UDF_INFO_NO_DEINIT(fnv_64, INT_RESULT),
    DECLARE_UDF_INFO_NO_DEINIT(fnv1a_64, INT_RESULT),
    DECLARE_UDF_INFO_NO_DEINIT(murmur_hash, INT_RESULT)};

#undef DECLARE_UDF_INFO_NO_DEINIT
#undef DECLARE_UDF_INFO

using udf_bitset_type =
    std::bitset<std::tuple_size<decltype(known_udfs)>::value>;
static udf_bitset_type registered_udfs;

static mysql_service_status_t component_init() {
  mysqlpp::register_udfs(mysql_service_udf_registration, known_udfs,
                         registered_udfs);
  return registered_udfs.all() ? 0 : 1;
}

static mysql_service_status_t component_deinit() {
  mysqlpp::unregister_udfs(mysql_service_udf_registration, known_udfs,
                           registered_udfs);

  return registered_udfs.none() ? 0 : 1;
}

// clang-format off
BEGIN_COMPONENT_PROVIDES(CURRENT_COMPONENT_NAME)
END_COMPONENT_PROVIDES();

BEGIN_COMPONENT_REQUIRES(CURRENT_COMPONENT_NAME)
  REQUIRES_SERVICE(udf_registration),
END_COMPONENT_REQUIRES();

BEGIN_COMPONENT_METADATA(CURRENT_COMPONENT_NAME)
  METADATA("mysql.author", "Percona Corporation"),
  METADATA("mysql.license", "GPL"),
END_COMPONENT_METADATA();

DECLARE_COMPONENT(CURRENT_COMPONENT_NAME, CURRENT_COMPONENT_NAME_STR)
  component_init,
  component_deinit,
END_DECLARE_COMPONENT();
// clang-format on

DECLARE_LIBRARY_COMPONENTS &COMPONENT_REF(CURRENT_COMPONENT_NAME)
    END_DECLARE_LIBRARY_COMPONENTS
