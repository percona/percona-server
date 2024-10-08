/* Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA */

#ifndef MASKING_FUNCTIONS_COMPONENT_SYS_VARIABLE_SERVICE_TUPLE_HPP
#define MASKING_FUNCTIONS_COMPONENT_SYS_VARIABLE_SERVICE_TUPLE_HPP

#include <mysql/components/service.h>

#include <mysql/components/services/component_sys_var_service.h>

#include "masking_functions/component_sys_variable_service_tuple_fwd.hpp"

namespace masking_functions {

// A set of MySQL query services required to perform system variable
// registration / unregistration.
// It is recommended to be used in a combination with the
// 'primitive_singleton' class template.
//
// primitive_singleton<component_sys_variable_service_tuple>::instance() =
//   component_sys_variable_service_tuple{
//     component_sys_variable_register,
//     component_sys_variable_unregister
//   };
// ...
// sql_context
// ctx{primitive_singleton<component_sys_variable_service_tuple>::instance()};
struct component_sys_variable_service_tuple {
  SERVICE_TYPE(component_sys_variable_register) * registrator;
  SERVICE_TYPE(component_sys_variable_unregister) * unregistrator;
};

}  // namespace masking_functions

#endif  // MASKING_FUNCTIONS_COMPONENT_SYS_VARIABLE_SERVICE_TUPLE_HPP
