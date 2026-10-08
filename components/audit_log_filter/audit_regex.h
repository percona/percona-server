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

#ifndef AUDIT_LOG_FILTER_AUDIT_REGEX_H_INCLUDED
#define AUDIT_LOG_FILTER_AUDIT_REGEX_H_INCLUDED

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>

namespace audit_log_filter::regex {

/**
 * @brief Maximum size of a decoded regex pattern in UTF-8 bytes.
 */
constexpr std::size_t kMaxPatternBytes = 16384;

/**
 * @brief ICU engine execution limits applied to every match.
 *
 * The time limit is expressed in ICU engine-work units, the stack limit
 * in bytes of backtracking stack. A value of 0 disables the limit.
 */
struct RegexLimits {
  int32_t time_limit;
  int32_t stack_limit;
};

/**
 * @brief Production limits, matching the server defaults of
 *        regexp_time_limit and regexp_stack_limit without reading
 *        session variables.
 */
constexpr RegexLimits kDefaultRegexLimits{32, 8000000};

enum class RegexErrorCategory {
  None,
  // Pattern exceeds kMaxPatternBytes
  Size,
  // Pattern is not valid UTF-8
  Encoding,
  // Memory allocation failure
  Allocation,
  // ICU rejected the pattern syntax
  Syntax,
  // Match exceeded the time limit
  Timeout,
  // Match exceeded the backtracking stack limit
  Stack,
  // Any other engine or data loading failure
  Engine
};

/**
 * @brief Fixed-size error information, filled without allocating.
 */
struct RegexError {
  RegexErrorCategory category{RegexErrorCategory::None};
  // ICU UErrorCode value
  int32_t status{0};
  // ICU reported pattern line, 0 when not available
  int32_t line{0};
  // ICU reported character position within the line, -1 when not available
  int32_t offset{-1};

  /**
   * @brief Check if ICU supplied a meaningful pattern position.
   */
  [[nodiscard]] bool has_position() const noexcept {
    return line > 0 && offset >= 0;
  }
};

enum class RegexMatchResult { Match, NoMatch, Error };

/**
 * @brief Get static name of an ICU status code, e.g. "U_REGEX_TIME_OUT".
 *
 * @param status ICU UErrorCode value
 * @return Pointer to a static null-terminated string
 */
[[nodiscard]] const char *status_name(int32_t status) noexcept;

/**
 * @brief Get static name of an error category used in runtime diagnostics.
 *
 * @param category Error category
 * @return Pointer to a static null-terminated string
 */
[[nodiscard]] const char *category_name(RegexErrorCategory category) noexcept;

/**
 * @brief Make error information describing a memory allocation failure.
 *
 * @return Allocation failure error information
 */
[[nodiscard]] RegexError make_allocation_error() noexcept;

namespace detail {

/**
 * @brief Make error information for a failed pattern compilation.
 *
 * A status not indicating failure, used when ICU returned no pattern without
 * reporting an error, is normalized to U_MEMORY_ALLOCATION_ERROR before
 * classification. The position is kept only for non-allocation errors for
 * which ICU supplied one.
 *
 * @param status ICU UErrorCode value
 * @param line ICU reported pattern line, 0 when not available
 * @param offset ICU reported position within the line, -1 when not available
 * @return Error information
 */
[[nodiscard]] RegexError compile_failure(int32_t status, int32_t line,
                                         int32_t offset) noexcept;

/**
 * @brief Make error information for a failed match operation.
 *
 * A status not indicating failure, used when ICU returned no object without
 * reporting an error, is normalized to U_MEMORY_ALLOCATION_ERROR before
 * classification.
 *
 * @param status ICU UErrorCode value
 * @return Error information
 */
[[nodiscard]] RegexError match_failure(int32_t status) noexcept;

}  // namespace detail

/**
 * @brief Immutable compiled ICU regular expression.
 *
 * The compiled pattern owns its own copy of the pattern text and may be
 * shared by concurrent evaluations. Each evaluation creates its own matcher
 * and subject text, so no mutable state is shared between threads.
 */
class CompiledRegex {
 public:
  ~CompiledRegex();

  CompiledRegex(const CompiledRegex &) = delete;
  CompiledRegex &operator=(const CompiledRegex &) = delete;

  /**
   * @brief Compile a UTF-8 pattern.
   *
   * The pattern is read using its explicit length and may contain NUL bytes.
   * Malformed UTF-8 is rejected.
   *
   * @param pattern UTF-8 pattern, does not have to outlive the result
   * @param error Receives error details in case of failure
   * @return Compiled pattern or nullptr in case of failure
   */
  [[nodiscard]] static std::unique_ptr<CompiledRegex> compile(
      std::string_view pattern, RegexError &error) noexcept;

  /**
   * @brief Search the pattern anywhere in a UTF-8 subject using the default
   *        execution limits.
   *
   * Malformed subject sequences are decoded as U+FFFD.
   *
   * @param subject UTF-8 subject, read using its explicit length
   * @param error Receives error details in case of RegexMatchResult::Error
   * @return Match result
   */
  [[nodiscard]] RegexMatchResult find(std::string_view subject,
                                      RegexError &error) const noexcept;

  /**
   * @brief Search the pattern anywhere in a UTF-8 subject.
   *
   * @param subject UTF-8 subject, read using its explicit length
   * @param limits Execution limits
   * @param error Receives error details in case of RegexMatchResult::Error
   * @return Match result
   */
  [[nodiscard]] RegexMatchResult find(std::string_view subject,
                                      const RegexLimits &limits,
                                      RegexError &error) const noexcept;

 private:
  struct Impl;
  explicit CompiledRegex(Impl *impl) noexcept;

  Impl *m_impl;
};

}  // namespace audit_log_filter::regex

#endif  // AUDIT_LOG_FILTER_AUDIT_REGEX_H_INCLUDED
