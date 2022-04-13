/* Copyright (c) 2022 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#ifndef AUDIT_LOG_FILTER_EVENT_FILTER_FUNCTION_H_INCLUDED
#define AUDIT_LOG_FILTER_EVENT_FILTER_FUNCTION_H_INCLUDED

#include "plugin/audit_log_filter/event_filter_function/base.h"

namespace audit_log_filter {

using namespace event_filter_function;

/**
 * @brief Get event filter function type.
 *
 * @param func_name Function name
 * @return One of @ref EventFilterFunctionType
 */
EventFilterFunctionType get_filter_function_type(
    const std::string &func_name) noexcept;

/**
 * @brief Validate function arguments.
 *
 * @param func_type Function type
 * @param args Function arguments list
 * @param expected_return_type Function expected return type
 * @return true in case arguments are valid, false otherwise
 */
bool validate_filter_function_args(
    EventFilterFunctionType func_type, const FunctionArgsList &args,
    FunctionReturnType expected_return_type) noexcept;

/**
 * @brief Get function argument type.
 *
 * @param type_name Argument type name
 * @return Argument type defined by @ref FunctionArgType,
 *         @ref FunctionArgType::None in case unknown type name is provided
 */
FunctionArgType get_filter_function_arg_type(
    const std::string &type_name) noexcept;

/**
 * @brief Get function argument value source type.
 *
 * @param type_name Source type name
 * @return Value source type defined by @ref FunctionArgSourceType,
 *         @ref FunctionArgSourceType::None in case unknown type name
 *         is provided
 */
FunctionArgSourceType get_filter_function_arg_source_type(
    const std::string &type_name) noexcept;

/**
 * @brief Get an instance of filter function of specified type.
 *
 * @param [in] function_type Function type,
 *                           @see event_filter_function::EventFilterFunctionType
 * @return An instance of event filter function
 */
std::unique_ptr<EventFilterFunctionBase> get_event_filter_function(
    EventFilterFunctionType function_type, const FunctionArgsList &args);

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_EVENT_FILTER_FUNCTION_H_INCLUDED
