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

#include "components/audit_log_filter/event_filter_function.h"

#include "components/audit_log_filter/event_filter_function/query_digest.h"
#include "components/audit_log_filter/event_filter_function/string_find.h"

#include <memory>
#include <string_view>
#include <unordered_map>

namespace audit_log_filter {
namespace {

const std::string_view kFuncNameStringFind{"string_find"};
const std::string_view kFuncNameQueryDigest{"query_digest"};

}  // namespace

EventFilterFunctionType get_filter_function_type(
    const std::string &func_name) noexcept {
  static const std::unordered_map<std::string_view, EventFilterFunctionType>
      func_name_to_type{
          {kFuncNameStringFind, EventFilterFunctionType::StringFind},
          {kFuncNameQueryDigest, EventFilterFunctionType::QueryDigest}};

  const auto it = func_name_to_type.find(func_name);

  return it != func_name_to_type.cend() ? it->second
                                        : EventFilterFunctionType::Unknown;
}

bool validate_filter_function_args(
    const EventFilterFunctionType func_type, const FunctionArgsList &args,
    const FunctionReturnType expected_return_type) noexcept {
  switch (func_type) {
    case EventFilterFunctionType::StringFind:
      return EventFilterFunctionStringFind::validate_args(args,
                                                          expected_return_type);
    case EventFilterFunctionType::QueryDigest:
      return EventFilterFunctionQueryDigest::validate_args(
          args, expected_return_type);
    case EventFilterFunctionType::Unknown:
    default:
      assert(false);
  }

  return false;
}

FunctionArgType get_filter_function_arg_type(
    const std::string &type_name) noexcept {
  static const std::unordered_map<std::string, FunctionArgType> arg_type_map{
      {"string", FunctionArgType::String},
  };

  const auto it = arg_type_map.find(type_name);
  return (it != arg_type_map.cend()) ? it->second : FunctionArgType::None;
}

FunctionArgSourceType get_filter_function_arg_source_type(
    const std::string &type_name) noexcept {
  static const std::unordered_map<std::string, FunctionArgSourceType>
      arg_source_map{
          {"string", FunctionArgSourceType::String},
          {"field", FunctionArgSourceType::Field},
      };

  const auto it = arg_source_map.find(type_name);
  return (it != arg_source_map.cend()) ? it->second
                                       : FunctionArgSourceType::None;
}

template <EventFilterFunctionType FunctionType>
std::unique_ptr<EventFilterFunctionBase> create_helper(
    const FunctionArgsList &args) {
  return std::make_unique<EventFilterFunction<FunctionType>>(args);
}

std::unique_ptr<EventFilterFunctionBase> get_event_filter_function(
    EventFilterFunctionType function_type, const FunctionArgsList &args) {
  using CreateFunc =
      std::unique_ptr<EventFilterFunctionBase> (*)(const FunctionArgsList &);
  static const CreateFunc
      funcs[static_cast<int>(EventFilterFunctionType::Unknown)] = {
          create_helper<EventFilterFunctionType::StringFind>,
          create_helper<EventFilterFunctionType::QueryDigest>};
  return (*funcs[static_cast<int>(function_type)])(args);
}

}  // namespace audit_log_filter
