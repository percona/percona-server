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

#ifndef AUDIT_LOG_FILTER_EVENT_FILTER_FUNCTION_BASE_H_INCLUDED
#define AUDIT_LOG_FILTER_EVENT_FILTER_FUNCTION_BASE_H_INCLUDED

#include "plugin/audit_log_filter/audit_record.h"

#include <memory>
#include <string>
#include <vector>

namespace audit_log_filter::event_filter_function {

enum class FunctionArgType { String, None };

enum class FunctionArgSourceType { String, Field, None };

enum class FunctionReturnType { Bool, String };

struct FunctionArg {
  FunctionArgType arg_type;
  FunctionArgSourceType source_type;
  std::string value;
};

using FunctionArgsList = std::vector<FunctionArg>;

/**
 * @brief Lists supported functions.
 */
enum class EventFilterFunctionType {
  StringFind,
  QueryDigest,
  // This item must be last in the list
  Unknown
};

class EventFilterFunctionBase {
 public:
  explicit EventFilterFunctionBase(FunctionArgsList args);

  virtual ~EventFilterFunctionBase() = default;

  /**
   * @brief Execute function in boolean context.
   *
   * @param fields Audit event fields list
   * @param result Function result
   * @return true in case function executed successfully, false otherwise
   */
  virtual bool exec(const AuditRecordFieldsList &fields [[maybe_unused]],
                    bool &result [[maybe_unused]]) noexcept {
    return false;
  }

  /**
   * @brief Execute function in string context.
   *
   * @param fields Audit event fields list
   * @param result Function result
   * @return true in case function executed successfully, false otherwise
   */
  virtual bool exec(const AuditRecordFieldsList &fields [[maybe_unused]],
                    std::string &result [[maybe_unused]]) noexcept {
    return false;
  }

  /**
   * @brief Check if has arguments provided.
   *
   * @return true in case function has arguments, false otherwise
   */
  [[nodiscard]] bool has_args() const noexcept;

 protected:
  /**
   * @brief Get string representation of a function argument.
   *
   * @tparam arg_index Argument index
   * @param event_fields Audit event fields list
   * @return String representation of the argument
   */
  template <std::size_t arg_index>
  std::string arg_to_string(
      const AuditRecordFieldsList &event_fields) noexcept {
    assert(arg_index < m_args.size());

    if (m_args[arg_index].source_type == FunctionArgSourceType::String) {
      return m_args[arg_index].value;
    } else if (m_args[arg_index].source_type == FunctionArgSourceType::Field) {
      const auto it = event_fields.find(m_args[arg_index].value);
      if (it != event_fields.cend()) {
        return it->second;
      }
    }

    return "";
  }

 private:
  FunctionArgsList m_args;
};

template <EventFilterFunctionType FunctionType>
class EventFilterFunction;

}  // namespace audit_log_filter::event_filter_function

#endif  // AUDIT_LOG_FILTER_EVENT_FILTER_FUNCTION_BASE_H_INCLUDED
