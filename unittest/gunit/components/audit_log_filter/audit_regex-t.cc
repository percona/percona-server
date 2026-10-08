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

#include <gtest/gtest.h>

#include "components/audit_log_filter/audit_regex.h"
#include "my_config.h"

#if !defined(_MSC_VER) && (defined(HAVE_LSAN) || defined(__SANITIZE_ADDRESS__))
#define AUDIT_REGEX_TEST_LSAN 1
#elif defined(__has_feature)
#if __has_feature(address_sanitizer) || __has_feature(leak_sanitizer)
#define AUDIT_REGEX_TEST_LSAN 1
#endif
#endif

#ifdef AUDIT_REGEX_TEST_LSAN
#include <sanitizer/lsan_interface.h>
#endif

#include <unicode/putil.h>
#include <unicode/uclean.h>
#include <unicode/utypes.h>

#include <atomic>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

/*
 * Allocation failure injection. ICU allocations are routed through
 * u_setMemoryFunctions() and the wrapper's own C++ allocations through the
 * replaced nothrow operator new. When armed with n, the n-th following
 * allocation (counting from 0) of the selected source fails, all others
 * succeed.
 */
namespace alloc_fault {

enum class Source { Cxx, Icu };

std::atomic<long> cxx_countdown{-1};
std::atomic<long> icu_countdown{-1};
std::atomic<bool> fired{false};
// Live ICU allocations, used to detect leaks on failure paths
std::atomic<long> icu_outstanding{0};

bool should_fail(std::atomic<long> &countdown) noexcept {
  long current = countdown.load();
  while (current >= 0) {
    if (countdown.compare_exchange_weak(current, current - 1)) {
      if (current == 0) {
        fired = true;
        return true;
      }
      return false;
    }
  }
  return false;
}

void *U_CALLCONV icu_alloc(const void *, size_t size) {
  if (should_fail(icu_countdown)) return nullptr;
  void *ptr = std::malloc(size);
  if (ptr != nullptr) icu_outstanding.fetch_add(1);
  return ptr;
}

void *U_CALLCONV icu_realloc(const void *, void *ptr, size_t size) {
  if (should_fail(icu_countdown)) return nullptr;
  void *result = std::realloc(ptr, size);
  if (ptr == nullptr && result != nullptr) icu_outstanding.fetch_add(1);
  return result;
}

void U_CALLCONV icu_free(const void *, void *ptr) {
  if (ptr != nullptr) icu_outstanding.fetch_sub(1);
  std::free(ptr);
}

// Must be installed before ICU allocates anything
const bool icu_memory_functions_set = [] {
  UErrorCode status = U_ZERO_ERROR;
  u_setMemoryFunctions(nullptr, icu_alloc, icu_realloc, icu_free, &status);
  return U_SUCCESS(status);
}();

}  // namespace alloc_fault

void *operator new(std::size_t size, const std::nothrow_t &) noexcept {
  if (alloc_fault::should_fail(alloc_fault::cxx_countdown)) return nullptr;
  try {
    return ::operator new(size);
  } catch (...) {
    return nullptr;
  }
}

void *operator new[](std::size_t size, const std::nothrow_t &) noexcept {
  if (alloc_fault::should_fail(alloc_fault::cxx_countdown)) return nullptr;
  try {
    return ::operator new[](size);
  } catch (...) {
    return nullptr;
  }
}

namespace audit_log_filter::regex {
namespace {

using namespace std::string_literals;
using namespace std::string_view_literals;

#ifdef AUDIT_REGEX_TEST_ICU_DATA_DIR
// Must be set before ICU loads any data, as mysqld does at startup
const bool icu_data_directory_set = [] {
  u_setDataDirectory(AUDIT_REGEX_TEST_ICU_DATA_DIR);
  return true;
}();
#endif

std::unique_ptr<CompiledRegex> compile_ok(std::string_view pattern) {
  RegexError error;
  auto compiled = CompiledRegex::compile(pattern, error);
  EXPECT_NE(compiled, nullptr)
      << "pattern: " << pattern << ", status: " << status_name(error.status);
  EXPECT_EQ(error.category, RegexErrorCategory::None);
  return compiled;
}

RegexMatchResult find(const CompiledRegex &compiled, std::string_view subject) {
  RegexError error;
  const auto result = compiled.find(subject, error);
  EXPECT_NE(result, RegexMatchResult::Error)
      << "status: " << status_name(error.status);
  return result;
}

RegexError compile_error(std::string_view pattern) {
  RegexError error;
  EXPECT_EQ(CompiledRegex::compile(pattern, error), nullptr)
      << "pattern: " << pattern;
  return error;
}

TEST(AuditRegex, SearchAnywhere) {
  auto re = compile_ok("^(new_orders|orders|history)[0-9]+$");
  ASSERT_NE(re, nullptr);
  EXPECT_EQ(find(*re, "orders1"), RegexMatchResult::Match);
  EXPECT_EQ(find(*re, "new_orders12"), RegexMatchResult::Match);
  EXPECT_EQ(find(*re, "history3"), RegexMatchResult::Match);
  EXPECT_EQ(find(*re, "orders"), RegexMatchResult::NoMatch);
  EXPECT_EQ(find(*re, "orders_archive"), RegexMatchResult::NoMatch);
  EXPECT_EQ(find(*re, "customer1"), RegexMatchResult::NoMatch);

  auto unanchored = compile_ok("orders");
  ASSERT_NE(unanchored, nullptr);
  EXPECT_EQ(find(*unanchored, "new_orders1"), RegexMatchResult::Match);
  EXPECT_EQ(find(*unanchored, "orders_archive"), RegexMatchResult::Match);
  EXPECT_EQ(find(*unanchored, "order"), RegexMatchResult::NoMatch);
}

TEST(AuditRegex, Anchors) {
  auto dollar = compile_ok("^orders[0-9]+$");
  auto absolute = compile_ok("\\Aorders[0-9]+\\z");
  ASSERT_NE(dollar, nullptr);
  ASSERT_NE(absolute, nullptr);

  // ICU '$' may match before a final line terminator, '\z' may not
  EXPECT_EQ(find(*dollar, "orders1\n"), RegexMatchResult::Match);
  EXPECT_EQ(find(*absolute, "orders1\n"), RegexMatchResult::NoMatch);
  EXPECT_EQ(find(*absolute, "orders1"), RegexMatchResult::Match);
  EXPECT_EQ(find(*absolute, "orders_archive"), RegexMatchResult::NoMatch);
}

TEST(AuditRegex, CaseAndFlags) {
  auto sensitive = compile_ok("^ORDERS[0-9]+$");
  auto insensitive = compile_ok("(?i)^ORDERS[0-9]+$");
  ASSERT_NE(sensitive, nullptr);
  ASSERT_NE(insensitive, nullptr);
  EXPECT_EQ(find(*sensitive, "orders1"), RegexMatchResult::NoMatch);
  EXPECT_EQ(find(*insensitive, "orders1"), RegexMatchResult::Match);
}

TEST(AuditRegex, Unicode) {
  auto re = compile_ok("^zam.wienia[0-9]+$");
  ASSERT_NE(re, nullptr);
  EXPECT_EQ(find(*re, "zamówienia1"), RegexMatchResult::Match);

  auto literal = compile_ok("café");
  ASSERT_NE(literal, nullptr);
  EXPECT_EQ(find(*literal, "SELECT 'café'"), RegexMatchResult::Match);
  // Latin-1 encoded subject is not transcoded
  EXPECT_EQ(find(*literal, "SELECT 'caf\xE9'"), RegexMatchResult::NoMatch);

  auto emoji = compile_ok("^😀+$");
  ASSERT_NE(emoji, nullptr);
  EXPECT_EQ(find(*emoji, "😀😀"), RegexMatchResult::Match);

  auto property = compile_ok("^\\p{L}+$");
  ASSERT_NE(property, nullptr);
  EXPECT_EQ(find(*property, "zażółć"), RegexMatchResult::Match);
  EXPECT_EQ(find(*property, "abc1"), RegexMatchResult::NoMatch);
}

TEST(AuditRegex, MalformedSubjectReplacementDecoding) {
  auto replacement = compile_ok("a\\x{FFFD}b");
  ASSERT_NE(replacement, nullptr);
  EXPECT_EQ(find(*replacement,
                 "a\xFF"
                 "b"),
            RegexMatchResult::Match);
  EXPECT_EQ(find(*replacement,
                 "a\xC3"
                 "b"),
            RegexMatchResult::Match);
  EXPECT_EQ(find(*replacement, "ab"), RegexMatchResult::NoMatch);

  // ASCII marker is still searchable in a non-UTF-8 subject
  auto marker = compile_ok("marker_[0-9]+");
  ASSERT_NE(marker, nullptr);
  EXPECT_EQ(find(*marker, "SELECT 'caf\xE9' /* marker_1 */"),
            RegexMatchResult::Match);
}

TEST(AuditRegex, EmbeddedNul) {
  const auto pattern = "nul_probe_a\0b"s;
  ASSERT_EQ(pattern.size(), 13U);
  auto re = compile_ok(pattern);
  ASSERT_NE(re, nullptr);
  EXPECT_EQ(find(*re, "SELECT 'nul_probe_a\0b' AS p"sv),
            RegexMatchResult::Match);
  EXPECT_EQ(find(*re, "SELECT 'nul_probe_ab' AS p"sv),
            RegexMatchResult::NoMatch);

  // Pattern consisting of one NUL byte is nonempty and valid
  auto nul = compile_ok("\0"sv);
  ASSERT_NE(nul, nullptr);
  EXPECT_EQ(find(*nul, "a\0"sv), RegexMatchResult::Match);
  EXPECT_EQ(find(*nul, "a"sv), RegexMatchResult::NoMatch);

  // Subject text after NUL is searched
  auto tail = compile_ok("tail$");
  ASSERT_NE(tail, nullptr);
  EXPECT_EQ(find(*tail, "head\0tail"sv), RegexMatchResult::Match);
}

TEST(AuditRegex, ExplicitLengthBuffers) {
  // Neither pattern nor subject is null-terminated at its view end
  const char pattern_buf[] = {'o', 'r', 'd', 'e', 'r', 's', '$', 'X'};
  auto re = compile_ok(std::string_view{pattern_buf, 7});
  ASSERT_NE(re, nullptr);

  const char subject_buf[] = {'o', 'r', 'd', 'e', 'r', 's', 'Z'};
  EXPECT_EQ(find(*re, std::string_view{subject_buf, 6}),
            RegexMatchResult::Match);
  EXPECT_EQ(find(*re, std::string_view{subject_buf, 7}),
            RegexMatchResult::NoMatch);
}

TEST(AuditRegex, EmptySubjectAndPattern) {
  auto empty_value = compile_ok("^$");
  auto absolute_empty = compile_ok("\\A\\z");
  ASSERT_NE(empty_value, nullptr);
  ASSERT_NE(absolute_empty, nullptr);

  EXPECT_EQ(find(*empty_value, std::string_view{}), RegexMatchResult::Match);
  EXPECT_EQ(find(*empty_value, ""), RegexMatchResult::Match);
  EXPECT_EQ(find(*absolute_empty, std::string_view{}), RegexMatchResult::Match);
  EXPECT_EQ(find(*empty_value, "orders1"), RegexMatchResult::NoMatch);
  EXPECT_EQ(find(*absolute_empty, "customer1"), RegexMatchResult::NoMatch);

  // The wrapper accepts an empty pattern (public definitions reject it),
  // ICU treats it as matching at every position.
  auto empty_pattern = compile_ok(std::string_view{});
  ASSERT_NE(empty_pattern, nullptr);
  EXPECT_EQ(find(*empty_pattern, std::string_view{}), RegexMatchResult::Match);
  EXPECT_EQ(find(*empty_pattern, "anything"), RegexMatchResult::Match);
}

TEST(AuditRegex, PatternStorageIsOwned) {
  std::unique_ptr<CompiledRegex> re;
  {
    auto temporary = std::make_unique<std::string>("^orders[0-9]+$");
    re = compile_ok(*temporary);
    // Overwrite and release the source storage before matching
    std::memset(temporary->data(), 'x', temporary->size());
  }
  ASSERT_NE(re, nullptr);
  EXPECT_EQ(find(*re, "orders1"), RegexMatchResult::Match);
  EXPECT_EQ(find(*re, "xxxxxxx"), RegexMatchResult::NoMatch);
}

TEST(AuditRegex, StrictPatternEncoding) {
  for (const auto pattern :
       {"\xC3("sv, "abc\xFF"sv, "\xC0\xAF"sv, "\xED\xA0\x80"sv,
        "\xF4\x90\x80\x80"sv, "a\xE2\x82"sv}) {
    const auto error = compile_error(pattern);
    EXPECT_EQ(error.category, RegexErrorCategory::Encoding);
    EXPECT_STREQ(status_name(error.status), "U_INVALID_CHAR_FOUND");
    EXPECT_FALSE(error.has_position());
  }
}

TEST(AuditRegex, PatternSizeBound) {
  const std::string max_pattern(kMaxPatternBytes, 'a');
  auto re = compile_ok(max_pattern);
  ASSERT_NE(re, nullptr);
  EXPECT_EQ(find(*re, max_pattern), RegexMatchResult::Match);

  const auto error = compile_error(std::string(kMaxPatternBytes + 1, 'a'));
  EXPECT_EQ(error.category, RegexErrorCategory::Size);
  EXPECT_FALSE(error.has_position());

  // The bound applies to UTF-8 bytes, not code points
  std::string multibyte;
  while (multibyte.size() + 2 <= kMaxPatternBytes) multibyte += "ó";
  auto multibyte_re = compile_ok(multibyte);
  EXPECT_NE(multibyte_re, nullptr);
  EXPECT_EQ(compile_error(multibyte + "óó").category, RegexErrorCategory::Size);
}

TEST(AuditRegex, CompileErrorPositions) {
  struct Case {
    std::string_view pattern;
    const char *status;
    int32_t line;
    int32_t offset;
  };

  // Positions are ICU line/character positions, neither UTF-16 code unit
  // nor UTF-8 byte offsets: UTF-16 lengths of the emoji cases are 3 and 5.
  const Case cases[] = {
      {"^(orders", "U_REGEX_MISMATCHED_PAREN", 1, 8},
      {"a{2,1}", "U_REGEX_MAX_LT_MIN", 1, 6},
      {"[z-a]", "U_REGEX_INVALID_RANGE", 1, 4},
      {"a(", "U_REGEX_MISMATCHED_PAREN", 1, 2},
      {"😀(", "U_REGEX_MISMATCHED_PAREN", 1, 2},
      {"a\n😀(", "U_REGEX_MISMATCHED_PAREN", 2, 2},
  };

  for (const auto &c : cases) {
    const auto error = compile_error(c.pattern);
    EXPECT_EQ(error.category, RegexErrorCategory::Syntax) << c.pattern;
    EXPECT_STREQ(status_name(error.status), c.status) << c.pattern;
    EXPECT_TRUE(error.has_position()) << c.pattern;
    EXPECT_EQ(error.line, c.line) << c.pattern;
    EXPECT_EQ(error.offset, c.offset) << c.pattern;
  }
}

TEST(AuditRegex, NamedCharacterData) {
  auto re = compile_ok("^\\N{LATIN SMALL LETTER A}+$");
  ASSERT_NE(re, nullptr);
  EXPECT_EQ(find(*re, "aaa"), RegexMatchResult::Match);
  EXPECT_EQ(find(*re, "aab"), RegexMatchResult::NoMatch);
}

TEST(AuditRegex, TimeoutAndRecovery) {
  auto re = compile_ok("(a+)+$");
  ASSERT_NE(re, nullptr);

  // 49 bytes, the trailing quote prevents a successful match
  const std::string pathological = "SELECT '" + std::string(40, 'a') + "'";
  ASSERT_EQ(pathological.size(), 49U);

  RegexError error;
  EXPECT_EQ(re->find(pathological, error), RegexMatchResult::Error);
  EXPECT_EQ(error.category, RegexErrorCategory::Timeout);
  EXPECT_STREQ(status_name(error.status), "U_REGEX_TIME_OUT");
  EXPECT_STREQ(category_name(error.category), "timeout");

  // The same compiled pattern keeps working with a new matcher
  EXPECT_EQ(re->find("SELECT 1 AS aaaa", error), RegexMatchResult::Match);
  EXPECT_EQ(error.category, RegexErrorCategory::None);
}

TEST(AuditRegex, StackLimitAndRecovery) {
  auto re = compile_ok("^(a|b)*$");
  ASSERT_NE(re, nullptr);
  const std::string subject(100000, 'a');

  RegexError error;
  EXPECT_EQ(re->find(subject, RegexLimits{0, 4096}, error),
            RegexMatchResult::Error);
  EXPECT_EQ(error.category, RegexErrorCategory::Stack);
  EXPECT_STREQ(status_name(error.status), "U_REGEX_STACK_OVERFLOW");
  EXPECT_STREQ(category_name(error.category), "stack");

  // Production limits handle the same subject
  EXPECT_EQ(re->find(subject, error), RegexMatchResult::Match);
  EXPECT_EQ(error.category, RegexErrorCategory::None);
  EXPECT_EQ(re->find("ab", RegexLimits{0, 4096}, error),
            RegexMatchResult::Match);
}

TEST(AuditRegex, SharedPatternConcurrentMatchers) {
  auto re = compile_ok("^(new_orders|orders|history)[0-9]+$");
  ASSERT_NE(re, nullptr);

  constexpr int kThreads = 8;
  constexpr int kIterations = 2000;
  std::atomic<int> failures{0};
  std::vector<std::thread> threads;

  for (int t = 0; t < kThreads; ++t) {
    threads.emplace_back([&re, &failures, t]() {
      for (int i = 0; i < kIterations; ++i) {
        // Each thread uses its own distinct subjects
        const std::string matching = "orders" + std::to_string(t * 100000 + i);
        const std::string missing = "customer" + std::to_string(i);
        RegexError error;
        if (re->find(matching, error) != RegexMatchResult::Match ||
            re->find(missing, error) != RegexMatchResult::NoMatch) {
          failures.fetch_add(1);
        }
      }
    });
  }

  for (auto &thread : threads) thread.join();
  EXPECT_EQ(failures.load(), 0);
}

/*
 * Fails exactly one allocation, the n-th one made during its lifetime.
 */
class AllocationFailure {
 public:
  AllocationFailure(alloc_fault::Source source, long n) noexcept
      : m_countdown{source == alloc_fault::Source::Cxx
                        ? alloc_fault::cxx_countdown
                        : alloc_fault::icu_countdown} {
    alloc_fault::fired = false;
    m_countdown = n;
  }
  ~AllocationFailure() { m_countdown = -1; }

  AllocationFailure(const AllocationFailure &) = delete;
  AllocationFailure &operator=(const AllocationFailure &) = delete;

  [[nodiscard]] bool fired() const noexcept { return alloc_fault::fired; }

 private:
  std::atomic<long> &m_countdown;
};

constexpr auto kAllocationPattern = "^(new_orders|orders|history)[0-9]+$"sv;

/*
 * Failures of the wrapper's own allocations during compilation.
 */
TEST(AuditRegex, CompileAllocationFailures) {
  ASSERT_TRUE(alloc_fault::icu_memory_functions_set);

  // Warm up ICU one-time initialization, ICU would cache a failure injected
  // there for the whole process.
  ASSERT_NE(compile_ok(kAllocationPattern), nullptr);
  const long baseline = alloc_fault::icu_outstanding.load();
  int injected = 0;

  for (long n = 0; n < 100000; ++n) {
    RegexError error;
    std::unique_ptr<CompiledRegex> compiled;
    bool fired = false;
    {
      AllocationFailure failure{alloc_fault::Source::Cxx, n};
      compiled = CompiledRegex::compile(kAllocationPattern, error);
      fired = failure.fired();
    }

    if (!fired) {
      // All allocations of a compilation have been exercised
      ASSERT_NE(compiled, nullptr);
      EXPECT_EQ(find(*compiled, "orders1"), RegexMatchResult::Match);
      break;
    }

    ++injected;
    EXPECT_EQ(compiled, nullptr) << "allocation " << n;
    EXPECT_EQ(error.category, RegexErrorCategory::Allocation)
        << "allocation " << n << ": " << status_name(error.status);
    EXPECT_STREQ(status_name(error.status), "U_MEMORY_ALLOCATION_ERROR");
    EXPECT_FALSE(error.has_position());
    // Nothing allocated before the failure is leaked
    EXPECT_EQ(alloc_fault::icu_outstanding.load(), baseline)
        << "allocation " << n;
  }

  // Implementation, conversion buffer and wrapper object
  EXPECT_EQ(injected, 3);
  EXPECT_EQ(alloc_fault::icu_outstanding.load(), baseline);
}

#ifdef AUDIT_REGEX_TEST_PATCHED_ICU
/*
 * Failures of ICU allocations during compilation. Unpatched ICU (seen with
 * 74.2 and 77.1) crashes when RegexPattern::init() fails to allocate its set
 * vector, and silently compiles a pattern matching something else when a
 * UnicodeSet fails to allocate. The bundled ICU is patched for both, so
 * every injected failure must either fail the compilation or be absorbed
 * without changing what the pattern matches.
 */
TEST(AuditRegex, CompileIcuAllocationFailures) {
  ASSERT_TRUE(alloc_fault::icu_memory_functions_set);

  // Character sets, set operations, case closure and initial characters
  const std::string_view patterns[] = {kAllocationPattern, "(?i)\\p{Lu}+\\d"sv,
                                       "[a-z&&[^q]]+[0-9]"sv, "(?i)\\u00DF"sv,
                                       "[\\s\\d\\w]+@\\W"sv};
  const std::string_view subjects[] = {
      "orders1"sv, "new_orders22"sv, "xorders1"sv, "history"sv,
      "ab1"sv,     "AB1"sv,          "q1"sv,       "SS"sv,
      "ss"sv,      "a1 b@!"sv,       ""sv};

  const auto results = [&subjects](const CompiledRegex &re) {
    std::string out;
    for (const auto subject : subjects) {
      RegexError error;
      const auto result = re.find(subject, error);
      out += result == RegexMatchResult::Match     ? 'M'
             : result == RegexMatchResult::NoMatch ? '-'
                                                   : 'E';
    }
    return out;
  };

  for (const auto pattern : patterns) {
    // Also warms up ICU one-time initialization and caches
    auto reference = compile_ok(pattern);
    ASSERT_NE(reference, nullptr);
    const auto expected = results(*reference);
    reference.reset();

    const long baseline = alloc_fault::icu_outstanding.load();
    int injected = 0;

    for (long n = 0; n < 100000; ++n) {
      RegexError error;
      std::unique_ptr<CompiledRegex> compiled;
      bool fired = false;
      {
        AllocationFailure failure{alloc_fault::Source::Icu, n};
        compiled = CompiledRegex::compile(pattern, error);
        fired = failure.fired();
      }

      if (!fired) {
        ASSERT_NE(compiled, nullptr) << pattern;
        EXPECT_EQ(results(*compiled), expected) << pattern;
        break;
      }

      ++injected;
      if (compiled == nullptr) {
        EXPECT_EQ(error.category, RegexErrorCategory::Allocation)
            << pattern << ", allocation " << n << ": "
            << status_name(error.status);
        EXPECT_FALSE(error.has_position());
      } else {
        EXPECT_EQ(results(*compiled), expected)
            << pattern << ", allocation " << n;
        compiled.reset();
      }
      EXPECT_EQ(alloc_fault::icu_outstanding.load(), baseline)
          << pattern << ", allocation " << n;
    }

    EXPECT_GT(injected, 0) << pattern;
  }
}
#endif

/*
 * Failures of ICU allocations made by each evaluation (matcher and its
 * state), the UText is stack allocated. ICU reports a failure to grow the
 * backtracking stack as U_REGEX_STACK_OVERFLOW.
 *
 * ICU itself (seen with 74.2 and 77.1) leaks one allocation when cloning
 * the subject fails in RegexMatcher::reset(): utext_setup() clears
 * UTEXT_OPEN of the matcher's heap allocated UText, so utext_close() in the
 * matcher destructor does not free it. This is tolerated for a single
 * injected allocation only, anything else would be leaked by the wrapper.
 * The test's own accounting checks this, so LeakSanitizer ignores the
 * allocations of evaluations with an injected failure.
 *
 * Which allocations ICU makes, and how it reports their failures, depends
 * on the ICU version, so only properties holding for any of them are
 * checked: every injected failure is an error of an expected category, it
 * does not affect the shared pattern, and nothing outlives an evaluation
 * apart from the known leak.
 */
TEST(AuditRegex, MatchAllocationFailures) {
  auto re = compile_ok(kAllocationPattern);
  ASSERT_NE(re, nullptr);
  EXPECT_EQ(find(*re, "orders1"), RegexMatchResult::Match);

  // The first sweep may populate ICU caches lazily, the second one must not
  // allocate anything which outlives an evaluation.
  for (int sweep = 0; sweep < 2; ++sweep) {
    long outstanding = alloc_fault::icu_outstanding.load();
    int injected = 0;
    int leaking = 0;

    for (long n = 0; n < 100000; ++n) {
      RegexError error;
      RegexMatchResult result;
      bool fired = false;
      {
#ifdef AUDIT_REGEX_TEST_LSAN
        __lsan::ScopedDisabler lsan_disabler;
#endif
        AllocationFailure failure{alloc_fault::Source::Icu, n};
        result = re->find("orders1", error);
        fired = failure.fired();
      }

      if (!fired) {
        EXPECT_EQ(result, RegexMatchResult::Match);
        break;
      }

      ++injected;
      EXPECT_EQ(result, RegexMatchResult::Error) << "allocation " << n;
      EXPECT_TRUE(error.category == RegexErrorCategory::Allocation ||
                  error.category == RegexErrorCategory::Stack)
          << "allocation " << n << ": " << status_name(error.status);
      // The shared pattern is not affected
      EXPECT_EQ(find(*re, "orders1"), RegexMatchResult::Match);

      if (sweep > 0) {
        const long leaked = alloc_fault::icu_outstanding.load() - outstanding;
        EXPECT_GE(leaked, 0) << "allocation " << n;
        EXPECT_LE(leaked, 1) << "allocation " << n;
        if (leaked != 0) ++leaking;
      }
      outstanding = alloc_fault::icu_outstanding.load();
    }

    EXPECT_GT(injected, 0);
    EXPECT_LE(leaking, 1);
  }
}

TEST(AuditRegex, FailureClassification) {
  // ICU returned no pattern without reporting a failure
  auto error = detail::compile_failure(U_ZERO_ERROR, 0, -1);
  EXPECT_EQ(error.category, RegexErrorCategory::Allocation);
  EXPECT_STREQ(status_name(error.status), "U_MEMORY_ALLOCATION_ERROR");
  EXPECT_FALSE(error.has_position());

  // Allocation failures never get a position
  error = detail::compile_failure(U_MEMORY_ALLOCATION_ERROR, 1, 4);
  EXPECT_EQ(error.category, RegexErrorCategory::Allocation);
  EXPECT_FALSE(error.has_position());

  // Data loading failures are not syntax errors and get a position only if
  // ICU supplied one
  error = detail::compile_failure(U_FILE_ACCESS_ERROR, 0, -1);
  EXPECT_EQ(error.category, RegexErrorCategory::Engine);
  EXPECT_STREQ(status_name(error.status), "U_FILE_ACCESS_ERROR");
  EXPECT_FALSE(error.has_position());
  error = detail::compile_failure(U_MISSING_RESOURCE_ERROR, 1, 3);
  EXPECT_EQ(error.category, RegexErrorCategory::Engine);
  EXPECT_TRUE(error.has_position());
  EXPECT_EQ(error.line, 1);
  EXPECT_EQ(error.offset, 3);

  // Engine limits within the regex error range are not syntax errors
  EXPECT_EQ(detail::compile_failure(U_REGEX_INTERNAL_ERROR, 0, -1).category,
            RegexErrorCategory::Engine);
  EXPECT_EQ(detail::compile_failure(U_REGEX_PATTERN_TOO_BIG, 0, -1).category,
            RegexErrorCategory::Engine);

  error = detail::compile_failure(U_REGEX_MISMATCHED_PAREN, 1, 8);
  EXPECT_EQ(error.category, RegexErrorCategory::Syntax);
  EXPECT_EQ(error.line, 1);
  EXPECT_EQ(error.offset, 8);

  // Matcher or UText creation returned no object without reporting a failure
  error = detail::match_failure(U_ZERO_ERROR);
  EXPECT_EQ(error.category, RegexErrorCategory::Allocation);
  EXPECT_STREQ(status_name(error.status), "U_MEMORY_ALLOCATION_ERROR");
  EXPECT_FALSE(error.has_position());

  EXPECT_EQ(detail::match_failure(U_MEMORY_ALLOCATION_ERROR).category,
            RegexErrorCategory::Allocation);
  EXPECT_EQ(detail::match_failure(U_REGEX_TIME_OUT).category,
            RegexErrorCategory::Timeout);
  EXPECT_EQ(detail::match_failure(U_REGEX_STACK_OVERFLOW).category,
            RegexErrorCategory::Stack);
  error = detail::match_failure(U_INTERNAL_PROGRAM_ERROR);
  EXPECT_EQ(error.category, RegexErrorCategory::Engine);
  EXPECT_STREQ(status_name(error.status), "U_INTERNAL_PROGRAM_ERROR");
}

TEST(AuditRegex, ErrorNames) {
  EXPECT_STREQ(category_name(RegexErrorCategory::Allocation), "allocation");
  EXPECT_STREQ(category_name(RegexErrorCategory::Engine), "engine");
  const auto allocation = make_allocation_error();
  EXPECT_EQ(allocation.category, RegexErrorCategory::Allocation);
  EXPECT_STREQ(status_name(allocation.status), "U_MEMORY_ALLOCATION_ERROR");
  EXPECT_FALSE(allocation.has_position());
}

}  // namespace
}  // namespace audit_log_filter::regex
