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

#include "components/audit_log_filter/audit_regex.h"

#include <unicode/parseerr.h>
#include <unicode/regex.h>
#include <unicode/ustring.h>
#include <unicode/utext.h>
#include <unicode/utypes.h>

#include <memory>
#include <new>

namespace audit_log_filter::regex {
namespace {

void set_error(RegexError &error, RegexErrorCategory category,
               UErrorCode status) noexcept {
  error.category = category;
  error.status = static_cast<int32_t>(status);
  error.line = 0;
  error.offset = -1;
}

RegexErrorCategory classify_compile_status(UErrorCode status) noexcept {
  if (status == U_MEMORY_ALLOCATION_ERROR) {
    return RegexErrorCategory::Allocation;
  }

  if (status >= U_REGEX_ERROR_START && status < U_REGEX_ERROR_LIMIT &&
      status != U_REGEX_INTERNAL_ERROR && status != U_REGEX_PATTERN_TOO_BIG) {
    return RegexErrorCategory::Syntax;
  }

  return RegexErrorCategory::Engine;
}

RegexErrorCategory classify_match_status(UErrorCode status) noexcept {
  switch (status) {
    case U_REGEX_TIME_OUT:
      return RegexErrorCategory::Timeout;
    case U_REGEX_STACK_OVERFLOW:
      return RegexErrorCategory::Stack;
    case U_MEMORY_ALLOCATION_ERROR:
      return RegexErrorCategory::Allocation;
    default:
      return RegexErrorCategory::Engine;
  }
}

/*
 * Closes UText on scope exit. Must be declared before the matcher using it
 * so that the matcher is destroyed first.
 */
class UTextGuard {
 public:
  explicit UTextGuard(UText *text) noexcept : m_text{text} {}
  ~UTextGuard() {
    if (m_text != nullptr) {
      utext_close(m_text);
    }
  }

  UTextGuard(const UTextGuard &) = delete;
  UTextGuard &operator=(const UTextGuard &) = delete;

 private:
  UText *m_text;
};

}  // namespace

struct CompiledRegex::Impl {
  // UTF-16 pattern text, the compiled pattern keeps a shallow reference to
  // it. Declared first, so that it is destroyed after the pattern.
  std::unique_ptr<UChar[]> pattern16;
  std::unique_ptr<icu::RegexPattern> pattern;
};

const char *status_name(int32_t status) noexcept {
  return u_errorName(static_cast<UErrorCode>(status));
}

const char *category_name(RegexErrorCategory category) noexcept {
  switch (category) {
    case RegexErrorCategory::None:
      return "none";
    case RegexErrorCategory::Size:
      return "size";
    case RegexErrorCategory::Encoding:
      return "encoding";
    case RegexErrorCategory::Allocation:
      return "allocation";
    case RegexErrorCategory::Syntax:
      return "syntax";
    case RegexErrorCategory::Timeout:
      return "timeout";
    case RegexErrorCategory::Stack:
      return "stack";
    case RegexErrorCategory::Engine:
      return "engine";
  }

  return "engine";
}

RegexError make_allocation_error() noexcept {
  RegexError error;
  set_error(error, RegexErrorCategory::Allocation, U_MEMORY_ALLOCATION_ERROR);
  return error;
}

namespace detail {

RegexError compile_failure(int32_t status, int32_t line,
                           int32_t offset) noexcept {
  // Normalize before classification, a null result without a failure
  // status is an allocation failure.
  const auto effective = U_FAILURE(static_cast<UErrorCode>(status))
                             ? static_cast<UErrorCode>(status)
                             : U_MEMORY_ALLOCATION_ERROR;
  const auto category = classify_compile_status(effective);

  RegexError error;
  set_error(error, category, effective);

  if (category != RegexErrorCategory::Allocation) {
    error.line = line;
    error.offset = offset;
  }

  return error;
}

RegexError match_failure(int32_t status) noexcept {
  const auto effective = U_FAILURE(static_cast<UErrorCode>(status))
                             ? static_cast<UErrorCode>(status)
                             : U_MEMORY_ALLOCATION_ERROR;
  RegexError error;
  set_error(error, classify_match_status(effective), effective);
  return error;
}

}  // namespace detail

CompiledRegex::CompiledRegex(Impl *impl) noexcept : m_impl{impl} {}

CompiledRegex::~CompiledRegex() { delete m_impl; }

std::unique_ptr<CompiledRegex> CompiledRegex::compile(
    std::string_view pattern, RegexError &error) noexcept {
  error = RegexError{};

  if (pattern.size() > kMaxPatternBytes) {
    set_error(error, RegexErrorCategory::Size, U_ILLEGAL_ARGUMENT_ERROR);
    return nullptr;
  }

  std::unique_ptr<Impl> impl{new (std::nothrow) Impl{}};

  if (impl == nullptr) {
    set_error(error, RegexErrorCategory::Allocation, U_MEMORY_ALLOCATION_ERROR);
    return nullptr;
  }

  // The bound above keeps all lengths well within int32_t range.
  const auto length8 = static_cast<int32_t>(pattern.size());
  int32_t length16 = 0;

  if (length8 > 0) {
    // Preflight to get the UTF-16 length, strict conversion rejects
    // malformed UTF-8 instead of substituting it.
    UErrorCode status = U_ZERO_ERROR;
    u_strFromUTF8(nullptr, 0, &length16, pattern.data(), length8, &status);

    if (status == U_BUFFER_OVERFLOW_ERROR ||
        status == U_STRING_NOT_TERMINATED_WARNING) {
      // Expected result of preflighting, must be reset, otherwise the
      // conversion below returns immediately without filling the buffer.
      status = U_ZERO_ERROR;
    } else if (U_FAILURE(status)) {
      set_error(error,
                status == U_INVALID_CHAR_FOUND ? RegexErrorCategory::Encoding
                                               : RegexErrorCategory::Engine,
                status);
      return nullptr;
    }

    if (length16 < 0 || length16 > static_cast<int32_t>(kMaxPatternBytes)) {
      set_error(error, RegexErrorCategory::Size, U_ILLEGAL_ARGUMENT_ERROR);
      return nullptr;
    }

    const int32_t capacity = length16 + 1;
    impl->pattern16.reset(new (std::nothrow) UChar[capacity]);

    if (impl->pattern16 == nullptr) {
      set_error(error, RegexErrorCategory::Allocation,
                U_MEMORY_ALLOCATION_ERROR);
      return nullptr;
    }

    int32_t converted16 = 0;
    u_strFromUTF8(impl->pattern16.get(), capacity, &converted16, pattern.data(),
                  length8, &status);

    if (U_FAILURE(status) || converted16 != length16) {
      set_error(error,
                status == U_INVALID_CHAR_FOUND ? RegexErrorCategory::Encoding
                                               : RegexErrorCategory::Engine,
                U_FAILURE(status) ? status : U_INTERNAL_PROGRAM_ERROR);
      return nullptr;
    }
  }

  static const UChar empty_pattern[] = {0};
  UErrorCode status = U_ZERO_ERROR;
  UText text_storage = UTEXT_INITIALIZER;
  UText *text = utext_openUChars(
      &text_storage,
      impl->pattern16 != nullptr ? impl->pattern16.get() : empty_pattern,
      length16, &status);

  if (U_FAILURE(status) || text == nullptr) {
    error = detail::compile_failure(status, 0, -1);
    return nullptr;
  }

  UTextGuard text_guard{text};
  UParseError parse_error{};
  parse_error.line = 0;
  parse_error.offset = -1;

  // The UText overload is used, because the UnicodeString one does not check
  // the allocation of its own pattern copy and crashes when it fails. ICU
  // keeps a shallow clone of the text, referencing impl->pattern16.
  impl->pattern.reset(icu::RegexPattern::compile(text, 0, parse_error, status));

  if (U_FAILURE(status) || impl->pattern == nullptr) {
    error =
        detail::compile_failure(status, parse_error.line, parse_error.offset);
    return nullptr;
  }

  std::unique_ptr<CompiledRegex> result{new (std::nothrow)
                                            CompiledRegex{impl.get()}};

  if (result == nullptr) {
    set_error(error, RegexErrorCategory::Allocation, U_MEMORY_ALLOCATION_ERROR);
    return nullptr;
  }

  impl.release();
  return result;
}

RegexMatchResult CompiledRegex::find(std::string_view subject,
                                     RegexError &error) const noexcept {
  return find(subject, kDefaultRegexLimits, error);
}

RegexMatchResult CompiledRegex::find(std::string_view subject,
                                     const RegexLimits &limits,
                                     RegexError &error) const noexcept {
  error = RegexError{};

  static const char empty_subject[] = "";
  const char *subject_data =
      subject.data() != nullptr ? subject.data() : empty_subject;

  UErrorCode status = U_ZERO_ERROR;
  UText text_storage = UTEXT_INITIALIZER;
  UText *text = utext_openUTF8(&text_storage, subject_data,
                               static_cast<int64_t>(subject.size()), &status);

  if (U_FAILURE(status) || text == nullptr) {
    error = detail::match_failure(status);
    return RegexMatchResult::Error;
  }

  // The guard is declared before the matcher, so the matcher referencing
  // the text is destroyed first on every exit path.
  UTextGuard text_guard{text};

  std::unique_ptr<icu::RegexMatcher> matcher{m_impl->pattern->matcher(status)};

  if (U_FAILURE(status) || matcher == nullptr) {
    error = detail::match_failure(status);
    return RegexMatchResult::Error;
  }

  // reset() records a failure as deferred status reported by the next
  // checked operation.
  matcher->reset(text);
  matcher->setTimeLimit(limits.time_limit, status);
  matcher->setStackLimit(limits.stack_limit, status);

  if (U_FAILURE(status)) {
    error = detail::match_failure(status);
    return RegexMatchResult::Error;
  }

  const bool found = matcher->find(status);

  if (U_FAILURE(status)) {
    error = detail::match_failure(status);
    return RegexMatchResult::Error;
  }

  return found ? RegexMatchResult::Match : RegexMatchResult::NoMatch;
}

}  // namespace audit_log_filter::regex
