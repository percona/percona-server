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

#include "components/audit_log_filter/event_field_condition/field_regex.h"

#include "my_dbug.h"

#ifndef AUDIT_LOG_FILTER_REGEX_TEST_SINK
#include "components/audit_log_filter/audit_error_log.h"
#include "components/audit_log_filter/sys_vars.h"
#endif

#include <variant>

namespace audit_log_filter::event_field_condition {
namespace {

/**
 * @brief Get length of a valid UTF-8 sequence starting at position pos.
 *
 * @return Sequence length, or 0 in case the bytes at pos do not start
 *         a valid (shortest form, non-surrogate, <= U+10FFFF) sequence
 */
std::size_t valid_utf8_sequence_length(std::string_view input,
                                       std::size_t pos) noexcept {
  const auto byte_at = [&input](std::size_t i) {
    return static_cast<unsigned char>(input[i]);
  };
  const auto is_cont = [](unsigned char c) { return (c & 0xC0) == 0x80; };

  const unsigned char lead = byte_at(pos);
  const std::size_t remaining = input.size() - pos;

  if (lead < 0x80) {
    return 1;
  }

  if (lead >= 0xC2 && lead <= 0xDF) {
    return remaining >= 2 && is_cont(byte_at(pos + 1)) ? 2 : 0;
  }

  if (lead >= 0xE0 && lead <= 0xEF) {
    if (remaining < 3) return 0;
    const unsigned char second = byte_at(pos + 1);
    if (!is_cont(second) || !is_cont(byte_at(pos + 2))) return 0;
    // Overlong forms
    if (lead == 0xE0 && second < 0xA0) return 0;
    // UTF-16 surrogates
    if (lead == 0xED && second >= 0xA0) return 0;
    return 3;
  }

  if (lead >= 0xF0 && lead <= 0xF4) {
    if (remaining < 4) return 0;
    const unsigned char second = byte_at(pos + 1);
    if (!is_cont(second) || !is_cont(byte_at(pos + 2)) ||
        !is_cont(byte_at(pos + 3)))
      return 0;
    // Overlong forms
    if (lead == 0xF0 && second < 0x90) return 0;
    // Above U+10FFFF
    if (lead == 0xF4 && second >= 0x90) return 0;
    return 4;
  }

  return 0;
}

}  // namespace

std::string make_diagnostic_text(std::string_view input) {
  static constexpr std::string_view kEllipsis{"..."};
  static constexpr char kHexDigits[] = "0123456789ABCDEF";
  static constexpr std::size_t kTruncatedLimit =
      kMaxDiagnosticTextBytes - kEllipsis.size();

  std::string out;
  out.reserve(kMaxDiagnosticTextBytes + 8);

  // Output length at the last complete unit boundary fitting the limit
  // left for truncated output.
  std::size_t truncate_length = 0;
  std::size_t pos = 0;

  while (pos < input.size()) {
    const auto c = static_cast<unsigned char>(input[pos]);
    std::size_t consumed = 1;

    if (c == '\\') {
      out.append("\\\\");
    } else if (c == '\'') {
      out.append("\\u0027");
    } else if (c < 0x20 || c == 0x7F) {
      out.append("\\u00");
      out.push_back(kHexDigits[c >> 4]);
      out.push_back(kHexDigits[c & 0x0F]);
    } else {
      consumed = valid_utf8_sequence_length(input, pos);

      if (consumed == 0) {
        consumed = 1;
        out.append("\\x");
        out.push_back(kHexDigits[c >> 4]);
        out.push_back(kHexDigits[c & 0x0F]);
      } else {
        out.append(input.substr(pos, consumed));
      }
    }

    pos += consumed;

    if (out.size() > kMaxDiagnosticTextBytes) {
      out.resize(truncate_length);
      out.append(kEllipsis);
      return out;
    }

    if (out.size() <= kTruncatedLimit) {
      truncate_length = out.size();
    }
  }

  return out;
}

bool RegexWarningLimiter::try_acquire(std::chrono::nanoseconds now) noexcept {
  const int64_t now_ns = now.count();
  int64_t last = m_last_warning.load(std::memory_order_relaxed);

  while (true) {
    // Check the sentinel before subtracting to avoid overflow
    if (last != kNeverWarned && now_ns - last < kInterval.count()) {
      return false;
    }

    if (m_last_warning.compare_exchange_weak(last, now_ns,
                                             std::memory_order_acq_rel,
                                             std::memory_order_relaxed)) {
      return true;
    }
  }
}

EventFieldConditionRegex::EventFieldConditionRegex(
    std::string field_name, std::unique_ptr<regex::CompiledRegex> regex,
    RegexConditionDiagnostics diagnostics, regex::RegexLimits limits) noexcept
    : m_field_name{std::move(field_name)},
      m_regex{std::move(regex)},
      m_diagnostics{std::move(diagnostics)},
      m_limits{limits} {}

regex::RegexMatchResult EventFieldConditionRegex::evaluate(
    const AuditRecordFieldsList &fields,
    regex::RegexError &error) const noexcept {
  const auto field = fields.find(m_field_name);

  if (field == fields.cend()) {
    return regex::RegexMatchResult::NoMatch;
  }

  const auto *value = std::get_if<std::string>(&field->second);

  if (value == nullptr) {
    return regex::RegexMatchResult::NoMatch;
  }

  DBUG_EXECUTE_IF("audit_log_filter_regex_runtime_error", {
    error = regex::make_allocation_error();
    return regex::RegexMatchResult::Error;
  });

  return m_regex->find(*value, m_limits, error);
}

bool EventFieldConditionRegex::check_applies(
    const AuditRecordFieldsList &fields) const noexcept {
  regex::RegexError error;
  const auto result = evaluate(fields, error);

  if (result == regex::RegexMatchResult::Error) {
    report_failure(error,
                   std::chrono::duration_cast<std::chrono::nanoseconds>(
                       std::chrono::steady_clock::now().time_since_epoch()));
    return false;
  }

  return result == regex::RegexMatchResult::Match;
}

void EventFieldConditionRegex::report_failure(
    const regex::RegexError &error,
    std::chrono::nanoseconds now) const noexcept {
  // Every failure is counted, even if its warning is suppressed
  regex_failure_sink::count_error();

  if (!m_limiter.try_acquire(now)) {
    return;
  }

  regex_failure_sink::warn(
      {m_diagnostics.filter_name.c_str(), m_diagnostics.pattern_preview.c_str(),
       m_diagnostics.field_name.c_str(), regex::category_name(error.category),
       regex::status_name(error.status)});
}

#ifndef AUDIT_LOG_FILTER_REGEX_TEST_SINK
namespace regex_failure_sink {

void count_error() noexcept { SysVars::inc_regex_match_errors(); }

void warn(const RegexFailureReport &report) noexcept {
  LogComponentErr(WARNING_LEVEL, ER_AUDIT_FILTER_REGEX_MATCH_FAILURE,
                  report.filter_name, report.pattern_preview, report.field_name,
                  report.category, report.status);
}

}  // namespace regex_failure_sink
#endif

}  // namespace audit_log_filter::event_field_condition
