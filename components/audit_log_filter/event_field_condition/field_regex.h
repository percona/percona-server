/* Copyright (c) 2026 Percona LLC and/or its affiliates. All rights reserved.

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

#ifndef AUDIT_LOG_FILTER_EVENT_FIELD_CONDITION_FIELD_REGEX_H_INCLUDED
#define AUDIT_LOG_FILTER_EVENT_FIELD_CONDITION_FIELD_REGEX_H_INCLUDED

#include "base.h"

#include "components/audit_log_filter/audit_regex.h"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <string_view>

namespace audit_log_filter::event_field_condition {

/**
 * @brief Maximum size of a diagnostic text produced by
 *        @ref make_diagnostic_text, including a trailing "..." in case
 *        the text was truncated.
 */
constexpr std::size_t kMaxDiagnosticTextBytes = 96;

/**
 * @brief Make a bounded printable representation of an arbitrary string for
 *        use in diagnostics (filter names, field names, JSON keys and
 *        regex pattern previews).
 *
 * Backslash is written as "\\", apostrophe as "'", control bytes
 * U+0000..U+001F and U+007F as "\u00XX" using uppercase hexadecimal digits.
 * Bytes which are not part of a valid UTF-8 sequence are written as "\xXX".
 * Valid UTF-8 code points are kept intact. The result is limited to
 * @ref kMaxDiagnosticTextBytes bytes including a final "..." added in case
 * the input was truncated. Truncation happens only between complete escapes
 * or code points.
 *
 * Intended to be called only while constructing conditions or diagnostics,
 * never while matching.
 *
 * @param input Input string, may contain any bytes including NUL
 * @return Bounded escaped representation
 * @throw std::bad_alloc if memory allocation fails
 */
[[nodiscard]] std::string make_diagnostic_text(std::string_view input);

/**
 * @brief Limits regex failure warnings of a single condition to at most one
 *        per interval.
 *
 * The first failure is eligible immediately. Timestamps are monotonic
 * durations since an arbitrary epoch, keeping sub-second precision.
 */
class RegexWarningLimiter {
 public:
  static constexpr std::chrono::nanoseconds kInterval{std::chrono::seconds{60}};

  /**
   * @brief Try to acquire a warning slot.
   *
   * Under contention only one caller acquires the slot.
   *
   * @param now Current monotonic time
   * @return true in case the caller may emit a warning, false otherwise
   */
  [[nodiscard]] bool try_acquire(std::chrono::nanoseconds now) noexcept;

 private:
  static constexpr int64_t kNeverWarned = std::numeric_limits<int64_t>::min();

  std::atomic<int64_t> m_last_warning{kNeverWarned};
};

/**
 * @brief Arguments of a regex runtime failure warning. All members point to
 *        precomputed or static strings.
 */
struct RegexFailureReport {
  const char *filter_name;
  const char *pattern_preview;
  const char *field_name;
  const char *category;
  const char *status;
};

/**
 * @brief Destination of regex runtime failure reports. Production
 *        implementation increments Audit_log_filter_regex_match_errors and
 *        writes a warning into the error log. Unit tests provide their own
 *        implementation.
 */
namespace regex_failure_sink {
void count_error() noexcept;
void warn(const RegexFailureReport &report) noexcept;
}  // namespace regex_failure_sink

/**
 * @brief Precomputed bounded diagnostic strings of a regex condition.
 */
struct RegexConditionDiagnostics {
  std::string filter_name;
  std::string field_name;
  std::string pattern_preview;
};

/**
 * @brief Condition matching a string event field against an ICU regular
 *        expression, searching anywhere in the field value.
 */
class EventFieldConditionRegex : public EventFieldConditionBase {
 public:
  EventFieldConditionRegex(
      std::string field_name, std::unique_ptr<regex::CompiledRegex> regex,
      RegexConditionDiagnostics diagnostics,
      regex::RegexLimits limits = regex::kDefaultRegexLimits) noexcept;

  /**
   * @brief Check if logical condition applies to provided event fields.
   *
   * Missing and non-string fields do not match. Engine failures do not match
   * and are reported using @ref report_failure.
   *
   * @param fields Event fields list
   * @return true in case condition applies to an audit event, false otherwise
   */
  [[nodiscard]] bool check_applies(
      const AuditRecordFieldsList &fields) const noexcept override;

  /**
   * @brief Evaluate regex against event fields without reporting failures.
   *
   * @param fields Event fields list
   * @param error Receives error details in case of RegexMatchResult::Error
   * @return Match result
   */
  [[nodiscard]] regex::RegexMatchResult evaluate(
      const AuditRecordFieldsList &fields,
      regex::RegexError &error) const noexcept;

  /**
   * @brief Report an evaluation failure: count it and emit a rate-limited
   *        warning.
   *
   * @param error Evaluation error details
   * @param now Current monotonic time
   */
  void report_failure(const regex::RegexError &error,
                      std::chrono::nanoseconds now) const noexcept;

 private:
  std::string m_field_name;
  std::unique_ptr<regex::CompiledRegex> m_regex;
  RegexConditionDiagnostics m_diagnostics;
  regex::RegexLimits m_limits;
  mutable RegexWarningLimiter m_limiter;
};

}  // namespace audit_log_filter::event_field_condition

#endif  // AUDIT_LOG_FILTER_EVENT_FIELD_CONDITION_FIELD_REGEX_H_INCLUDED
