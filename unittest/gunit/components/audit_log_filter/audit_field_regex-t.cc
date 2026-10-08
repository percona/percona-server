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

#include "components/audit_log_filter/audit_record.h"
#include "components/audit_log_filter/audit_regex.h"
#include "components/audit_log_filter/event_field_condition/and.h"
#include "components/audit_log_filter/event_field_condition/bool.h"
#include "components/audit_log_filter/event_field_condition/field_regex.h"
#include "components/audit_log_filter/event_field_condition/not.h"
#include "components/audit_log_filter/event_field_condition/or.h"

#include <mysql/components/services/defs/event_tracking_general_defs.h>
#include <mysql/components/services/defs/event_tracking_table_access_defs.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace audit_log_filter::event_field_condition {

/*
 * Test implementation of the failure sink, records what production code
 * would count and log.
 */
namespace {

struct RecordedWarning {
  std::string filter_name;
  std::string pattern_preview;
  std::string field_name;
  std::string category;
  std::string status;
};

std::atomic<uint64_t> g_error_count{0};
std::mutex g_warnings_mutex;
std::vector<RecordedWarning> g_warnings;

void reset_sink() {
  g_error_count.store(0);
  std::lock_guard<std::mutex> lock{g_warnings_mutex};
  g_warnings.clear();
}

std::vector<RecordedWarning> recorded_warnings() {
  std::lock_guard<std::mutex> lock{g_warnings_mutex};
  return g_warnings;
}

}  // namespace

namespace regex_failure_sink {

void count_error() noexcept { g_error_count.fetch_add(1); }

void warn(const RegexFailureReport &report) noexcept {
  std::lock_guard<std::mutex> lock{g_warnings_mutex};
  g_warnings.push_back({report.filter_name, report.pattern_preview,
                        report.field_name, report.category, report.status});
}

}  // namespace regex_failure_sink

namespace {

using namespace std::chrono_literals;
using namespace std::string_literals;
using namespace std::string_view_literals;

using regex::RegexError;
using regex::RegexLimits;
using regex::RegexMatchResult;

const std::string kPathological = "SELECT '" + std::string(40, 'a') + "'";

std::shared_ptr<EventFieldConditionRegex> make_condition(
    const std::string &field_name, std::string_view pattern,
    const std::string &filter_name = "rx_filter",
    RegexLimits limits = regex::kDefaultRegexLimits) {
  RegexError error;
  auto compiled = regex::CompiledRegex::compile(pattern, error);
  EXPECT_NE(compiled, nullptr) << regex::status_name(error.status);
  if (compiled == nullptr) return nullptr;
  return std::make_shared<EventFieldConditionRegex>(
      field_name, std::move(compiled),
      RegexConditionDiagnostics{make_diagnostic_text(filter_name),
                                make_diagnostic_text(field_name),
                                make_diagnostic_text(pattern)},
      limits);
}

class AuditFieldRegexTest : public ::testing::Test {
 protected:
  void SetUp() override { reset_sink(); }
};

/*
 * Diagnostic text helper
 */
TEST(AuditDiagnosticText, Escaping) {
  EXPECT_EQ(make_diagnostic_text(""), "");
  EXPECT_EQ(make_diagnostic_text("table_name.str"), "table_name.str");
  EXPECT_EQ(make_diagnostic_text("regex\0"sv), "regex\\u0000");
  EXPECT_EQ(make_diagnostic_text("a\\b"), "a\\\\b");
  EXPECT_EQ(make_diagnostic_text("it's"), "it\\u0027s");
  EXPECT_EQ(make_diagnostic_text("a\nb\tc\x1F\x7F"),
            "a\\u000Ab\\u0009c\\u001F\\u007F");
  EXPECT_EQ(make_diagnostic_text("zamówienia 😀"), "zamówienia 😀");
  // Bytes which are not valid UTF-8 are shown in hexadecimal
  EXPECT_EQ(make_diagnostic_text("caf\xE9"), "caf\\xE9");
  EXPECT_EQ(make_diagnostic_text("\xED\xA0\x80"), "\\xED\\xA0\\x80");
  EXPECT_EQ(make_diagnostic_text("\xC0\xAF"), "\\xC0\\xAF");
}

TEST(AuditDiagnosticText, Bounds) {
  const std::string exact(kMaxDiagnosticTextBytes, 'a');
  EXPECT_EQ(make_diagnostic_text(exact), exact);

  const std::string longer(kMaxDiagnosticTextBytes + 1, 'a');
  const auto truncated = make_diagnostic_text(longer);
  EXPECT_EQ(truncated.size(), kMaxDiagnosticTextBytes);
  EXPECT_EQ(truncated, std::string(kMaxDiagnosticTextBytes - 3, 'a') + "...");

  // Escapes are never split: 92 plain bytes leave no room for a 6 byte escape
  // within the 93 bytes available before "..."
  const auto escaped =
      make_diagnostic_text(std::string(92, 'a') + '\0' + std::string(10, 'b'));
  EXPECT_EQ(escaped, std::string(92, 'a') + "...");

  // Multibyte code points are never split
  std::string multibyte;
  for (int i = 0; i < 40; ++i) multibyte += "ó";  // 80 bytes
  multibyte += std::string(12, 'x') + "😀" + "tail";
  const auto cut = make_diagnostic_text(multibyte);
  EXPECT_LE(cut.size(), kMaxDiagnosticTextBytes);
  EXPECT_EQ(cut.substr(cut.size() - 3), "...");
  EXPECT_EQ(cut.substr(0, cut.size() - 3),
            multibyte.substr(0, 92));  // emoji at 92..95 does not fit 93

  // Output is bounded even for inputs made only of escapes
  const auto all_escapes = make_diagnostic_text(std::string(1000, '\0'));
  EXPECT_LE(all_escapes.size(), kMaxDiagnosticTextBytes);
  EXPECT_EQ(all_escapes, [] {
    std::string s;
    for (int i = 0; i < 15; ++i) s += "\\u0000";
    return s + "...";
  }());
}

/*
 * Warning limiter
 */
TEST(AuditRegexWarningLimiter, Interval) {
  RegexWarningLimiter limiter;
  EXPECT_TRUE(limiter.try_acquire(1000s));
  EXPECT_FALSE(limiter.try_acquire(1000s));
  EXPECT_FALSE(limiter.try_acquire(1059s));
  EXPECT_FALSE(limiter.try_acquire(1059s + 999ms));
  EXPECT_TRUE(limiter.try_acquire(1060s));
  EXPECT_FALSE(limiter.try_acquire(1061s));
  EXPECT_TRUE(limiter.try_acquire(2000s));
}

TEST(AuditRegexWarningLimiter, FirstWarningAtTimeZero) {
  RegexWarningLimiter limiter;
  EXPECT_TRUE(limiter.try_acquire(0s));
  EXPECT_FALSE(limiter.try_acquire(1s));
}

TEST(AuditRegexWarningLimiter, SubsecondPrecision) {
  RegexWarningLimiter limiter;
  EXPECT_TRUE(limiter.try_acquire(100s + 990ms));
  // Whole-second truncation would allow this one (160 - 100 = 60)
  EXPECT_FALSE(limiter.try_acquire(160s + 10ms));
  EXPECT_TRUE(limiter.try_acquire(160s + 990ms));
}

TEST(AuditRegexWarningLimiter, IndependentInstances) {
  RegexWarningLimiter first;
  RegexWarningLimiter second;
  EXPECT_TRUE(first.try_acquire(10s));
  EXPECT_TRUE(second.try_acquire(10s));
  EXPECT_FALSE(first.try_acquire(11s));
  EXPECT_FALSE(second.try_acquire(11s));
}

TEST(AuditRegexWarningLimiter, Contention) {
  for (int round = 0; round < 20; ++round) {
    RegexWarningLimiter limiter;
    constexpr int kThreads = 16;
    std::atomic<int> acquired{0};
    std::atomic<bool> start{false};
    std::vector<std::thread> threads;

    for (int t = 0; t < kThreads; ++t) {
      threads.emplace_back([&]() {
        while (!start.load()) std::this_thread::yield();
        if (limiter.try_acquire(5s)) acquired.fetch_add(1);
      });
    }

    start.store(true);
    for (auto &thread : threads) thread.join();
    EXPECT_EQ(acquired.load(), 1);
  }
}

/*
 * Regex condition
 */
TEST_F(AuditFieldRegexTest, MatchAndMissingFields) {
  auto cond =
      make_condition("table_name.str", "^(new_orders|orders|history)[0-9]+$");
  ASSERT_NE(cond, nullptr);

  EXPECT_TRUE(cond->check_applies({{"table_name.str", "orders1"s}}));
  EXPECT_TRUE(cond->check_applies({{"table_name.str", "history2"s}}));
  EXPECT_FALSE(cond->check_applies({{"table_name.str", "customer1"s}}));
  EXPECT_FALSE(cond->check_applies({{"table_name.str", "orders_archive"s}}));
  // Missing field
  EXPECT_FALSE(cond->check_applies({{"table_database.str", "orders1"s}}));
  EXPECT_FALSE(cond->check_applies({}));
  // Defensive non-string runtime value
  EXPECT_FALSE(cond->check_applies({{"table_name.str", uint64_t{1}}}));
  EXPECT_FALSE(cond->check_applies({{"table_name.str", int64_t{-1}}}));

  EXPECT_EQ(g_error_count.load(), 0U);
  EXPECT_TRUE(recorded_warnings().empty());
}

TEST_F(AuditFieldRegexTest, EmptyValuePredicates) {
  for (const auto pattern : {"^$"sv, "\\A\\z"sv}) {
    auto cond = make_condition("general_query.str", pattern);
    ASSERT_NE(cond, nullptr);
    EXPECT_TRUE(cond->check_applies({{"general_query.str", ""s}}));
    EXPECT_FALSE(cond->check_applies({{"general_query.str", "orders1"s}}));
    // Missing field is not an empty string
    EXPECT_FALSE(cond->check_applies({}));
  }
  EXPECT_EQ(g_error_count.load(), 0U);
}

TEST_F(AuditFieldRegexTest, RuntimeErrorCountedAndRateLimited) {
  auto cond = make_condition("general_query.str", "(a+)+$", "rx_timeout");
  ASSERT_NE(cond, nullptr);
  const AuditRecordFieldsList bad{{"general_query.str", kPathological}};
  const AuditRecordFieldsList good{{"general_query.str", "SELECT 1 AS aaaa"s}};

  RegexError error;
  ASSERT_EQ(cond->evaluate(bad, error), RegexMatchResult::Error);
  EXPECT_EQ(error.category, regex::RegexErrorCategory::Timeout);

  // Evaluating does not report anything by itself
  EXPECT_EQ(g_error_count.load(), 0U);

  EXPECT_FALSE(cond->check_applies(bad));
  EXPECT_FALSE(cond->check_applies(bad));
  EXPECT_FALSE(cond->check_applies(bad));
  EXPECT_EQ(g_error_count.load(), 3U);

  auto warnings = recorded_warnings();
  ASSERT_EQ(warnings.size(), 1U);
  EXPECT_EQ(warnings[0].filter_name, "rx_timeout");
  EXPECT_EQ(warnings[0].pattern_preview, "(a+)+$");
  EXPECT_EQ(warnings[0].field_name, "general_query.str");
  EXPECT_EQ(warnings[0].category, "timeout");
  EXPECT_EQ(warnings[0].status, "U_REGEX_TIME_OUT");

  // Recovery, successful matches do not count errors
  EXPECT_TRUE(cond->check_applies(good));
  EXPECT_EQ(g_error_count.load(), 3U);
}

TEST_F(AuditFieldRegexTest, ReportPolicyWithExplicitTime) {
  auto cond = make_condition("general_query.str", "x");
  ASSERT_NE(cond, nullptr);
  const auto error = regex::make_allocation_error();

  cond->report_failure(error, 100s + 990ms);
  cond->report_failure(error, 160s + 10ms);
  EXPECT_EQ(g_error_count.load(), 2U);
  EXPECT_EQ(recorded_warnings().size(), 1U);

  cond->report_failure(error, 160s + 990ms);
  EXPECT_EQ(g_error_count.load(), 3U);
  auto warnings = recorded_warnings();
  ASSERT_EQ(warnings.size(), 2U);
  EXPECT_EQ(warnings[1].category, "allocation");
  EXPECT_EQ(warnings[1].status, "U_MEMORY_ALLOCATION_ERROR");
}

TEST_F(AuditFieldRegexTest, IndependentConditionWarnings) {
  auto first = make_condition("general_query.str", "(a+)+$", "rx_one");
  auto second = make_condition("general_query.str", "(a|a+)+$", "rx_two");
  ASSERT_NE(first, nullptr);
  ASSERT_NE(second, nullptr);
  const AuditRecordFieldsList bad{{"general_query.str", kPathological}};

  for (int i = 0; i < 3; ++i) {
    EXPECT_FALSE(first->check_applies(bad));
    EXPECT_FALSE(second->check_applies(bad));
  }

  EXPECT_EQ(g_error_count.load(), 6U);
  const auto warnings = recorded_warnings();
  ASSERT_EQ(warnings.size(), 2U);
  EXPECT_EQ(warnings[0].filter_name, "rx_one");
  EXPECT_EQ(warnings[0].pattern_preview, "(a+)+$");
  EXPECT_EQ(warnings[1].filter_name, "rx_two");
  EXPECT_EQ(warnings[1].pattern_preview, "(a|a+)+$");
}

TEST_F(AuditFieldRegexTest, ConcurrentFailuresWarnOnce) {
  auto cond = make_condition("general_query.str", "x");
  ASSERT_NE(cond, nullptr);
  const auto error = regex::make_allocation_error();
  constexpr int kThreads = 8;
  std::atomic<bool> start{false};
  std::vector<std::thread> threads;

  for (int t = 0; t < kThreads; ++t) {
    threads.emplace_back([&]() {
      while (!start.load()) std::this_thread::yield();
      cond->report_failure(error, 42s);
    });
  }

  start.store(true);
  for (auto &thread : threads) thread.join();

  EXPECT_EQ(g_error_count.load(), static_cast<uint64_t>(kThreads));
  EXPECT_EQ(recorded_warnings().size(), 1U);
}

TEST_F(AuditFieldRegexTest, StackErrorAndPreviewEscaping) {
  const std::string pattern =
      "^(a|b)*$\\'"s + '\0' + "|" + std::string(100, 'c');
  auto cond = make_condition("general_query.str", pattern, "rx'stack",
                             RegexLimits{0, 4096});
  ASSERT_NE(cond, nullptr);

  EXPECT_FALSE(
      cond->check_applies({{"general_query.str", std::string(100000, 'a')}}));
  EXPECT_EQ(g_error_count.load(), 1U);

  const auto warnings = recorded_warnings();
  ASSERT_EQ(warnings.size(), 1U);
  EXPECT_EQ(warnings[0].category, "stack");
  EXPECT_EQ(warnings[0].status, "U_REGEX_STACK_OVERFLOW");
  EXPECT_EQ(warnings[0].filter_name, "rx\\u0027stack");
  EXPECT_EQ(warnings[0].pattern_preview,
            "^(a|b)*$\\\\\\u0027\\u0000|" + std::string(93 - 23, 'c') + "...");
  EXPECT_EQ(warnings[0].pattern_preview.size(), kMaxDiagnosticTextBytes);
}

TEST_F(AuditFieldRegexTest, BooleanComposition) {
  auto failing = make_condition("general_query.str", "(a+)+$");
  ASSERT_NE(failing, nullptr);
  const AuditRecordFieldsList bad{{"general_query.str", kPathological},
                                  {"general_command.str", "Query"s}};

  // Short-circuited regex leaves are never evaluated
  EventFieldConditionAnd and_false{
      {std::make_shared<EventFieldConditionBool>(false), failing}};
  EventFieldConditionOr or_true{
      {std::make_shared<EventFieldConditionBool>(true), failing}};
  EXPECT_FALSE(and_false.check_applies(bad));
  EXPECT_TRUE(or_true.check_applies(bad));
  EXPECT_EQ(g_error_count.load(), 0U);

  // A failed leaf is false, its negation is true
  EventFieldConditionNot negated{failing};
  EXPECT_TRUE(negated.check_applies(bad));
  EXPECT_EQ(g_error_count.load(), 1U);

  EventFieldConditionAnd and_true{
      {std::make_shared<EventFieldConditionBool>(true), failing}};
  EXPECT_FALSE(and_true.check_applies(bad));
  EXPECT_EQ(g_error_count.load(), 2U);
}

/*
 * Real field maps produced for general and table access events
 */
TEST_F(AuditFieldRegexTest, GeneralFieldMap) {
  mysql_event_tracking_general_data event{};
  AuditRecordGeneral record{};
  record.event = &event;

  auto empty = make_condition("general_query.str", "^$");
  auto nul = make_condition("general_query.str", "nul_probe_a\0b"sv);
  auto marker = make_condition("general_query.str", "rx_marker");
  auto utf8_literal = make_condition("general_query.str", "café");
  ASSERT_NE(empty, nullptr);
  ASSERT_NE(nul, nullptr);
  ASSERT_NE(marker, nullptr);
  ASSERT_NE(utf8_literal, nullptr);

  // Unavailable capture is a present empty string with zero length
  record.extended_info.query = std::nullopt;
  auto fields = get_audit_record_fields(record);
  ASSERT_EQ(fields.count("general_query.str"), 1U);
  EXPECT_EQ(std::get<uint64_t>(fields.at("general_query.length")), 0U);
  EXPECT_TRUE(empty->check_applies(fields));

  // Present empty capture
  record.extended_info.query = std::string{};
  EXPECT_TRUE(empty->check_applies(get_audit_record_fields(record)));

  // Embedded NUL is kept in the subject
  record.extended_info.query = "SELECT 'nul_probe_a\0b' AS p"s;
  fields = get_audit_record_fields(record);
  EXPECT_EQ(std::get<uint64_t>(fields.at("general_query.length")), 27U);
  EXPECT_TRUE(nul->check_applies(fields));
  EXPECT_FALSE(empty->check_applies(fields));
  record.extended_info.query = "SELECT 'nul_probe_ab' AS p"s;
  EXPECT_FALSE(nul->check_applies(get_audit_record_fields(record)));

  // Non-UTF-8 raw query bytes, output conversion does not change subjects
  record.extended_info.query = "SELECT 'caf\xE9' /* rx_marker */"s;
  record.extended_info.query_charset = "latin1";
  record.extended_info.query_output =
      QueryOutput{"SELECT 'café' /* rx_marker */", ""};
  fields = get_audit_record_fields(record);
  EXPECT_TRUE(marker->check_applies(fields));
  EXPECT_FALSE(utf8_literal->check_applies(fields));

  record.extended_info.query = "SELECT 'café' /* rx_marker */"s;
  record.extended_info.query_charset = "utf8mb4";
  fields = get_audit_record_fields(record);
  EXPECT_TRUE(marker->check_applies(fields));
  EXPECT_TRUE(utf8_literal->check_applies(fields));

  EXPECT_EQ(g_error_count.load(), 0U);
}

TEST_F(AuditFieldRegexTest, TableAccessFieldMap) {
  mysql_event_tracking_table_access_data event{};
  event.table_database = {"tpcc", 4};
  event.table_name = {"orders1", 7};
  AuditRecordTableAccess record{};
  record.event = &event;

  auto table =
      make_condition("table_name.str", "^(new_orders|orders|history)[0-9]+$");
  auto query = make_condition("query.str", "^$");
  ASSERT_NE(table, nullptr);
  ASSERT_NE(query, nullptr);

  auto fields = get_audit_record_fields(record);
  EXPECT_TRUE(table->check_applies(fields));
  // Unavailable query capture is a present empty string
  EXPECT_TRUE(query->check_applies(fields));

  event.table_name = {"customer1", 9};
  EXPECT_FALSE(table->check_applies(get_audit_record_fields(record)));
  EXPECT_EQ(g_error_count.load(), 0U);
}

/*
 * Ownership: the condition is released by its publisher while evaluations
 * in other threads still hold it.
 */
TEST_F(AuditFieldRegexTest, OwnershipAcrossRelease) {
  auto publisher = make_condition("table_name.str", "^orders[0-9]+$");
  ASSERT_NE(publisher, nullptr);
  std::weak_ptr<EventFieldConditionRegex> observer = publisher;

  constexpr int kThreads = 4;
  std::mutex mutex;
  std::condition_variable cv;
  bool released = false;
  int ready = 0;
  std::atomic<int> failures{0};
  std::vector<std::thread> threads;

  for (int t = 0; t < kThreads; ++t) {
    std::shared_ptr<EventFieldConditionRegex> session_copy = publisher;
    threads.emplace_back([&, t, held = std::move(session_copy)]() mutable {
      {
        std::unique_lock<std::mutex> lock{mutex};
        ++ready;
        cv.notify_all();
        cv.wait(lock, [&] { return released; });
      }
      for (int i = 0; i < 500; ++i) {
        const std::string name = "orders" + std::to_string(t * 1000 + i);
        if (!held->check_applies({{"table_name.str", name}}) ||
            held->check_applies({{"table_name.str", "x" + name}})) {
          failures.fetch_add(1);
        }
      }
      held.reset();
    });
  }

  {
    std::unique_lock<std::mutex> lock{mutex};
    cv.wait(lock, [&] { return ready == kThreads; });
    // Publishing owner drops its reference, sessions keep theirs
    publisher.reset();
    EXPECT_FALSE(observer.expired());
    released = true;
    cv.notify_all();
  }

  for (auto &thread : threads) thread.join();
  EXPECT_TRUE(observer.expired());
  EXPECT_EQ(failures.load(), 0);
  EXPECT_EQ(g_error_count.load(), 0U);
}

}  // namespace
}  // namespace audit_log_filter::event_field_condition
