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

#include "plugin/audit_log_filter/event_field_condition/function.h"

#include <set>
#include <string>
#include <unordered_map>

namespace audit_log_filter::event_field_condition {
namespace {

const std::string_view kFuncNameStringFind{"string_find"};

std::string arg_to_string(const FunctionArg &arg,
                          const AuditRecordFieldsList &event_fields) {
  assert(arg.arg_type == FunctionArgType::String);
  if (arg.source_type == FunctionArgSourceType::String) {
    return arg.value;
  } else if (arg.source_type == FunctionArgSourceType::Field) {
    const auto it = event_fields.find(arg.value);
    if (it != event_fields.cend()) {
      return it->second;
    }
  }

  return "";
}

}  // namespace

EventFieldConditionFunction::EventFieldConditionFunction(std::string name,
                                                         FunctionArgsList args,
                                                         AuditAction action)
    : EventFieldConditionBase{action},
      m_name{std::move(name)},
      m_args{std::move(args)} {}

AuditAction EventFieldConditionFunction::check_applies(
    const AuditRecordFieldsList &fields) const noexcept {
  if (m_name == kFuncNameStringFind) {
    // string_find(text, substr)
    std::string text = arg_to_string(m_args[0], fields);
    std::string substr = arg_to_string(m_args[1], fields);

    auto pos = text.find(substr);

    return pos != std::string::npos ? get_match_action() : AuditAction::Skip;
  }

  return AuditAction::Skip;
}

bool EventFieldConditionFunction::check_function_name(
    const std::string &name) noexcept {
  static const std::set<std::string_view> known_funcs{kFuncNameStringFind};
  return known_funcs.count(name) > 0;
}

FunctionArgType EventFieldConditionFunction::get_function_arg_type(
    const std::string &type_name) noexcept {
  static const std::unordered_map<std::string, FunctionArgType> arg_type_map{
      {"string", FunctionArgType::String},
  };

  const auto it = arg_type_map.find(type_name);
  return (it != arg_type_map.cend()) ? it->second : FunctionArgType::None;
}

FunctionArgSourceType EventFieldConditionFunction::get_function_arg_source_type(
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

bool EventFieldConditionFunction::validate_args(
    const std::string &func_name, const FunctionArgsList &args) noexcept {
  if (func_name == kFuncNameStringFind) {
    return args.size() == 2 &&
           std::all_of(args.cbegin(), args.cend(), [](const auto &arg) {
             return arg.arg_type == FunctionArgType::String;
           });
  }

  return false;
}

}  // namespace audit_log_filter::event_field_condition
