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

#include "components/audit_log_filter/event_filter_function/string_find.h"

#include <algorithm>
#include <functional>

namespace audit_log_filter::event_filter_function {

EventFilterFunction<EventFilterFunctionType::StringFind>::EventFilterFunction(
    FunctionArgsList args)
    : EventFilterFunctionBase{std::move(args)} {}

bool EventFilterFunctionStringFind::validate_args(
    const FunctionArgsList &args,
    const FunctionReturnType expected_return_type [[maybe_unused]]) noexcept {
  return args.size() == 2 &&
         std::all_of(args.cbegin(), args.cend(), [](const auto &arg) {
           return arg.arg_type == FunctionArgType::String;
         });
}

bool EventFilterFunctionStringFind::exec(const AuditRecordFieldsList &fields,
                                         bool &result) noexcept {
  std::string text = arg_to_string<0>(fields);
  std::string substr = arg_to_string<1>(fields);

  result = text.find(substr) != std::string::npos;

  return true;
}

}  // namespace audit_log_filter::event_filter_function
