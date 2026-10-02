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

#include "components/audit_log_filter/audit_log_reader.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "mysql/service_mysql_alloc.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace audit_log_filter {
namespace {

using json_reader::AuditJsonHandler;

std::unique_ptr<AuditJsonHandler> handler(AuditLogReaderContext &context) {
  constexpr size_t buffer_size = 256;
  auto buffer = std::unique_ptr<char, std::function<void(char *)>>(
      static_cast<char *>(my_malloc(PSI_NOT_INSTRUMENTED, buffer_size, MYF(0))),
      [](char *p) { my_free(p); });
  context.batch_reader_args = std::make_unique<AuditLogReaderArgs>();
  context.batch_reader_args->command =
      AuditLogReaderArgs::Command::ReadFromTimestamp;
  return std::make_unique<AuditJsonHandler>(&context, std::move(buffer),
                                            buffer_size);
}

std::string input_json(const std::vector<std::string> &queries) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  writer.StartArray();
  unsigned id = 0;
  for (const auto &query : queries) {
    writer.StartObject();
    writer.Key("timestamp");
    writer.String("2026-09-25 12:00:00");
    writer.Key("id");
    writer.Uint(id++);
    writer.Key("query");
    writer.String(query.data(), query.size());
    writer.EndObject();
  }
  writer.EndArray();
  return {buffer.GetString(), buffer.GetSize()};
}

void round_trip(const std::vector<std::string> &queries, unsigned limit = 0) {
  AuditLogReaderContext context;
  auto output = handler(context);
  context.batch_reader_args->max_array_length = limit;
  const auto input = input_json(queries);
  rapidjson::StringStream stream(input.c_str());
  rapidjson::Reader reader;
  reader.IterativeParseInit();
  std::vector<std::string> actual;
  size_t batches = 0;
  while (!reader.IterativeParseComplete()) {
    ASSERT_LT(batches++, queries.size() + 2);
    context.is_batch_end = false;
    output->iterative_parse_init();
    while (!reader.IterativeParseComplete() && !context.is_batch_end) {
      ASSERT_TRUE(reader.IterativeParseNext<rapidjson::kParseDefaultFlags>(
          stream, *output));
    }
    const bool finished = reader.IterativeParseComplete();
    output->iterative_parse_close(finished);
    rapidjson::Document batch;
    batch.Parse<rapidjson::kParseValidateEncodingFlag>(
        output->get_result_buffer_ptr());
    ASSERT_FALSE(batch.HasParseError());
    ASSERT_TRUE(batch.IsArray());
    ASSERT_FALSE(batch.Empty());
    size_t events = 0;
    for (const auto &event : batch.GetArray()) {
      if (event.IsNull()) {
        EXPECT_TRUE(finished);
        continue;
      }
      EXPECT_EQ(actual.size(), event["id"].GetUint());
      const auto &query = event["query"];
      actual.emplace_back(query.GetString(), query.GetStringLength());
      ++events;
    }
    if (limit != 0) {
      EXPECT_LE(events, limit);
    }
    // Even after growing, the configured batch limit is retained: none of
    // these records can share a 256-byte batch with an oversized record.
    EXPECT_LE(events, 1U);
  }
  EXPECT_EQ(queries, actual);
}

TEST(AuditJsonHandler, OversizedFirstRecord) {
  round_trip({std::string(4096, 'a'), "tail"});
}

TEST(AuditJsonHandler, PendingRecordsAndRepeatedGrowth) {
  round_trip({"head", std::string(4096, 'a'), "middle", std::string(16384, 'b'),
              "tail"});
}

TEST(AuditJsonHandler, EscapingAndArrayLimit) {
  std::string query;
  for (int i = 0; i < 512; ++i) query.append("\xc3\xa9\"\\\0", 5);
  round_trip({"head", query, "tail"}, 1);
}

#ifndef NDEBUG
TEST(AuditJsonHandler, GrowthFailureThrowsInsteadOfOverflowing) {
  AuditLogReaderContext context;
  auto output = handler(context);
  const auto input = input_json({std::string(4096, 'a')});
  rapidjson::StringStream stream(input.c_str());
  rapidjson::Reader reader;
  output->iterative_parse_init();
  DBUG_SET("+d,audit_log_filter_reader_grow_bad_alloc");
  EXPECT_THROW(reader.Parse(stream, *output), std::bad_alloc);
  DBUG_SET("-d,audit_log_filter_reader_grow_bad_alloc");
}
#endif

}  // namespace
}  // namespace audit_log_filter
