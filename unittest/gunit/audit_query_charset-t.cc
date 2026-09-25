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

#include "components/audit_log_filter/charset_conversion.h"
#include "components/audit_log_filter/query_output.h"
#include "my_sys.h"
#include "mysql/components/services/defs/event_tracking_parse_defs.h"
#include "mysql/components/services/defs/event_tracking_query_defs.h"
#include "mysql/strings/m_ctype.h"

#include <stdexcept>

namespace audit_log_filter {
namespace {
const CHARSET_INFO *charset(const char *name) {
  return get_charset_by_csname(name, MY_CS_PRIMARY, MYF(0));
}

void check(const char *cs, std::string_view input, std::string_view expected) {
  const auto *source = charset(cs);
  ASSERT_NE(nullptr, source) << cs;
  const auto result = convert_query_to_utf8mb4(input, source);
  EXPECT_EQ(expected, result) << cs;
  EXPECT_EQ(expected.size(), result.size());
  // Independently check the encoder's output with the server's validator.
  int error = 0;
  EXPECT_EQ(result.size(),
            my_charset_utf8mb4_bin.cset->well_formed_len(
                &my_charset_utf8mb4_bin, result.data(),
                result.data() + result.size(), result.size(), &error));
  EXPECT_EQ(0, error);
}

TEST(AuditQueryCharset, EncodingsAndCharacterBoundaries) {
  check("latin1", "caf\xe9", "caf\xc3\xa9");
  check("cp1250", "\xb9\x9c", "\xc4\x85\xc5\x9b");
  check("sjis", "A\x83\x5c\\B", "A\xe3\x82\xbd\\B");
  check("gbk", "\x81\x5c", "\xe4\xb9\x97");
  check("big5", "\xa4\x5c", "\xe4\xb9\x88");
  check("swe7", "[\\]", "\xc3\x84\xc3\x96\xc3\x85");
  check("utf8mb4", "\xc3\xa9\xf0\x9f\x98\x80", "\xc3\xa9\xf0\x9f\x98\x80");
  check("utf8mb3", "\xc3\xa9", "\xc3\xa9");
  check("latin1", std::string_view("a\0\xe9", 3),
        std::string_view("a\0\xc3\xa9", 4));
  check("utf8mb4", "ASCII ' \" \\ < > &", "ASCII ' \" \\ < > &");
  check("latin1", std::string(10000, '\xe9'), [] {
    std::string expected;
    for (int i = 0; i < 10000; ++i) expected += "\xc3\xa9";
    return expected;
  }());
}

TEST(AuditQueryCharset, MalformedAndBinary) {
  check("utf8mb4", "\xe9", "?");
  check("utf8mb4", "\xf0\x9f", "??");
  check("utf8mb4", "\xc0\xaf", "??");
  check("utf8mb4", "\xed\xa0\x80", "???");
  check("utf8mb4", "\xf4\x90\x80\x80", "????");
  check("utf8mb4", "\xf0\x9f'",
        "?"
        "?'");
  check("utf8mb4",
        "a\xe9"
        "b",
        "a?b");
  check("utf8mb4", "SELECT 'caf\xe9'", "SELECT 'caf?'");
  check("utf8mb4", "SELECT '\xff'", "SELECT '?'");
  check("utf8mb3",
        "x\xe2"
        "A",
        "x?A");
  check("utf8mb4", "\xf0\xc3\xa9", "?\xc3\xa9");
  check("binary", "'\xe9'", "'?'");
  check("gb18030", "\x81\x30'", "?0'");
  check("ucs2", std::string_view("\xd8\0", 2), "?");
  check("utf32", std::string_view("\0\0\xd8\0", 4), "?");
  check("utf32", std::string_view("\0\x11\0\0", 4), "?");
  check("cp1250", "\x81", "?");
  check("sjis", "\x81\xad", "?");
  check("sjis", "\x83", "?");
  check("binary", "\xc3\xbcser", "\xc3\xbcser");
  check("binary", "\xe9", "?");
  EXPECT_EQ("", convert_query_to_utf8mb4({}, nullptr));
  EXPECT_THROW(convert_query_to_utf8mb4("x", nullptr), std::invalid_argument);
}

TEST(AuditQueryCharset, RejectImpossibleDecoderResults) {
  CHARSET_INFO source = my_charset_utf8mb4_bin;
  MY_CHARSET_HANDLER handler = *source.cset;
  source.cset = &handler;
  handler.mb_wc = [](const CHARSET_INFO *, my_wc_t *, const uint8_t *,
                     const uint8_t *) { return 8; };
  EXPECT_THROW(convert_query_to_utf8mb4("\x80", &source), std::runtime_error);
  handler.mb_wc = [](const CHARSET_INFO *, my_wc_t *, const uint8_t *,
                     const uint8_t *) { return -8; };
  EXPECT_THROW(convert_query_to_utf8mb4("\x80", &source), std::runtime_error);
  handler.mb_wc = [](const CHARSET_INFO *, my_wc_t *, const uint8_t *,
                     const uint8_t *) { return MY_CS_TOOSMALL2; };
  EXPECT_EQ("?xx", convert_query_to_utf8mb4("\x80xx", &source));
  handler.mb_wc = [](const CHARSET_INFO *, my_wc_t *wc, const uint8_t *,
                     const uint8_t *) {
    *wc = 0xd800;
    return 1;
  };
  EXPECT_EQ("?", convert_query_to_utf8mb4("\x80", &source));
}

TEST(AuditQueryCharset, PrepareParseReplacementAndPreserveRaw) {
  mysql_cstring_with_length rewritten{"SELECT '\xb9\x9c'", 11};
  mysql_event_tracking_parse_data event{};
  event.query = {"SELECT '\xb9'", 10};
  event.rewritten_query = &rewritten;
  AuditRecordParse parse{};
  parse.event = &event;
  parse.extended_info.query_charset = "cp1250";
  AuditRecordVariant record = parse;
  EXPECT_FALSE(query_output_is_ready(record));
  prepare_query_output(record);
  EXPECT_TRUE(query_output_is_ready(record));
  const auto &output =
      std::get<AuditRecordParse>(record).extended_info.query_output;
  ASSERT_TRUE(output.has_value());
  EXPECT_EQ("SELECT '\xc4\x85'", output->query);
  EXPECT_EQ("SELECT '\xc4\x85\xc5\x9b'", output->rewritten_query);
  EXPECT_EQ(std::string_view("SELECT '\xb9'"),
            std::string_view(event.query.str, event.query.length));
}

TEST(AuditQueryCharset, DigestSelectionAndCaptureFailure) {
  AuditRecordGeneral general{};
  general.extended_info.digest = "SELECT `\xc3\xa9`";
  // The selected digest does not require capture or a source charset.
  AuditRecordVariant record = general;
  prepare_query_output(record);
  auto &extra = std::get<AuditRecordGeneral>(record).extended_info;
  ASSERT_TRUE(extra.query_output.has_value());
  EXPECT_EQ(general.extended_info.digest, extra.query_output->query);
  extra.digest.clear();
  EXPECT_THROW(prepare_query_output(record), std::runtime_error);
  EXPECT_FALSE(extra.query_output.has_value());
  extra.query.emplace();
  EXPECT_NO_THROW(prepare_query_output(record));
  EXPECT_TRUE(extra.query_output->query.empty());
  extra.query = "\xe9";
  extra.query_charset = "no_such_charset";
  EXPECT_THROW(prepare_query_output(record), std::invalid_argument);
  EXPECT_FALSE(query_output_is_ready(record));
  EXPECT_FALSE(extra.query_output.has_value());
  extra.query_charset = "latin1";
  prepare_query_output(record);
  EXPECT_EQ("\xc3\xa9", extra.query_output->query);
  EXPECT_EQ("\xe9", extra.query);
}

TEST(AuditQueryCharset, InvalidBufferAndAtomicPreparation) {
  mysql_event_tracking_parse_data event{};
  event.query = {"SELECT 1", 8};
  mysql_cstring_with_length rewritten{nullptr, 2};
  event.rewritten_query = &rewritten;
  AuditRecordParse parse{};
  parse.event = &event;
  parse.extended_info.query_charset = "utf8mb4";
  AuditRecordVariant record = parse;
  EXPECT_THROW(prepare_query_output(record), std::invalid_argument);
  EXPECT_FALSE(std::get<AuditRecordParse>(record).extended_info.query_output);
  event.rewritten_query = nullptr;
  prepare_query_output(record);
  EXPECT_EQ(
      "SELECT 1",
      std::get<AuditRecordParse>(record).extended_info.query_output->query);
  event.query = {nullptr, 1};
  EXPECT_THROW(prepare_query_output(record), std::invalid_argument);
  EXPECT_FALSE(std::get<AuditRecordParse>(record).extended_info.query_output);
}

TEST(AuditQueryCharset, QueryUsesCapturedCharsetAndSanitizesDigest) {
  mysql_event_tracking_query_data event{};
  event.query = {"\xe9", 1};
  AuditRecordQuery query{};
  query.event = &event;
  query.extended_info.query_charset = "latin1";
  AuditRecordVariant record = query;
  prepare_query_output(record);
  auto &extra = std::get<AuditRecordQuery>(record).extended_info;
  EXPECT_EQ("\xc3\xa9", extra.query_output->query);
  extra.digest = "`\xe9`";
  prepare_query_output(record);
  EXPECT_EQ("`?`", extra.query_output->query);
}

TEST(AuditQueryCharset, ReadinessChecksOnlyQueryBearingRecords) {
  EXPECT_TRUE(query_output_is_ready(AuditRecordConnection{}));
  EXPECT_TRUE(query_output_is_ready(AuditRecordAudit{}));
  for (auto record : {AuditRecordVariant{AuditRecordGeneral{}},
                      AuditRecordVariant{AuditRecordTableAccess{}},
                      AuditRecordVariant{AuditRecordQuery{}},
                      AuditRecordVariant{AuditRecordParse{}}}) {
    EXPECT_FALSE(query_output_is_ready(record));
    std::visit([](auto &rec) { rec.extended_info.query_output.emplace(); },
               record);
    EXPECT_TRUE(query_output_is_ready(record));
  }
}
}  // namespace
}  // namespace audit_log_filter
