/* Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include <array>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

#include <boost/algorithm/find_backward.hpp>
#include <boost/preprocessor/stringize.hpp>

#include <mysql/components/services/component_sys_var_service.h>
#include <mysql/components/services/mysql_runtime_error_service.h>

#include <mysqlpp/udf_registration.hpp>
#include <mysqlpp/udf_wrappers.hpp>

#include <sql/binlog.h>
#include <sql/binlog/decompressing_event_object_istream.h>
#include <sql/binlog_reader.h>

// defined as a macro because needed both raw and stringized
#define CURRENT_COMPONENT_NAME binlog_utils_udf
#define CURRENT_COMPONENT_NAME_STR BOOST_PP_STRINGIZE(CURRENT_COMPONENT_NAME)

REQUIRES_SERVICE_PLACEHOLDER(udf_registration);
REQUIRES_SERVICE_PLACEHOLDER(component_sys_variable_register);

namespace {

//
// Binlog utils shared functions
//
const std::string_view default_component_name{"mysql_server"};
const std::string_view gtid_executed_variable_name{"gtid_executed"};

constexpr std::size_t default_static_buffer_size{1024};
using static_buffer_t = std::array<char, default_static_buffer_size + 1>;
using dynamic_buffer_t = std::vector<char>;
using uni_buffer_t = std::pair<static_buffer_t, dynamic_buffer_t>;
using Return_status = mysql::utils::Return_status;

std::string_view extract_sys_var_value(std::string_view component_name,
                                       std::string_view variable_name,
                                       uni_buffer_t &ub) {
  DBUG_TRACE;
  void *ptr = ub.first.data();
  std::size_t length = ub.first.size() - 1;

  if (mysql_service_component_sys_variable_register->get_variable(
          component_name.data(), variable_name.data(), &ptr, &length) == 0)
    return {static_cast<char *>(ptr), length};

  ub.second.resize(length + 1);
  ptr = ub.second.data();
  if (mysql_service_component_sys_variable_register->get_variable(
          component_name.data(), variable_name.data(), &ptr, &length) != 0)
    throw std::runtime_error("Cannot get sys_var value");

  if (ptr == nullptr) throw std::runtime_error("The value of sys_var is null");

  return {static_cast<char *>(ptr), length};
}

using log_event_ptr = std::shared_ptr<Log_event>;
using fn_reflen_buffer = char[FN_REFLEN + 1];

const char *check_and_normalize_binlog_name(std::string_view binlog_name,
                                            fn_reflen_buffer &buffer) {
  if (binlog_name.empty())
    throw std::runtime_error("expecting non-empty binlog name");

  if (std::find_if(binlog_name.begin(), binlog_name.end(),
                   &is_directory_separator) != binlog_name.end())
    throw std::runtime_error("binlog name must not contain path separators");

  std::size_t log_dir_length = dirname_length(mysql_bin_log.get_log_fname());
  if (log_dir_length + binlog_name.size() + 1 > sizeof buffer)
    throw std::runtime_error("binlog name is too long");

  auto it = std::copy_n(mysql_bin_log.get_log_fname(), log_dir_length, buffer);
  it = std::copy_n(binlog_name.data(), binlog_name.size(), it);
  *it = '\0';

  return buffer;
}

const char *get_short_binlog_name(const std::string &binlog_name) noexcept {
  return binlog_name.c_str() + dirname_length(binlog_name.c_str());
}

log_event_ptr find_first_event(std::string_view binlog_name) {
  DBUG_TRACE;

  fn_reflen_buffer binlog_name_buffer;
  auto search_file_name =
      check_and_normalize_binlog_name(binlog_name, binlog_name_buffer);

  Binlog_file_reader reader(false /* do not verify checksum */);
  if (reader.open(search_file_name, 0))
    throw std::runtime_error(reader.get_error_str());

  binlog::Decompressing_event_object_istream istream{reader};
  log_event_ptr ev;
  istream >> ev;

  if (reader.has_fatal_error())
    throw std::runtime_error(reader.get_error_str());
  if (istream.has_error()) throw std::runtime_error(istream.get_error_str());

  return ev;
}

log_event_ptr find_last_event(std::string_view binlog_name) {
  DBUG_TRACE;

  fn_reflen_buffer binlog_name_buffer;
  auto search_file_name =
      check_and_normalize_binlog_name(binlog_name, binlog_name_buffer);

  Binlog_file_reader reader(false /* do not verify checksum */);
  if (reader.open(search_file_name, 0))
    throw std::runtime_error(reader.get_error_str());

  // Here 'is_active()' is called after 'get_binlog_end_pos()' deliberately
  // to properly handle the situation when rotation happens between these
  // two calls
  my_off_t end_pos = mysql_bin_log.get_binlog_end_pos();
  if (!mysql_bin_log.is_active(search_file_name))
    end_pos = std::numeric_limits<my_off_t>::max();

  binlog::Decompressing_event_object_istream istream{reader};
  log_event_ptr ev;
  istream >> ev;
  if (istream.has_error()) throw std::runtime_error(istream.get_error_str());

  while (true) {
    if (reader.has_fatal_error())
      throw std::runtime_error(reader.get_error_str());
    if (ev->common_header->log_pos >= end_pos) break;
    log_event_ptr next_ev;
    if (!(istream >> next_ev)) break;
    ev.swap(next_ev);
  }
  if (istream.has_error()) throw std::runtime_error(istream.get_error_str());
  return ev;
}

log_event_ptr find_previous_gtids_event(std::string_view binlog_name) {
  DBUG_TRACE;

  fn_reflen_buffer binlog_name_buffer;
  auto search_file_name =
      check_and_normalize_binlog_name(binlog_name, binlog_name_buffer);

  Binlog_file_reader reader(false /* do not verify checksum */);
  if (reader.open(search_file_name, 0))
    throw std::runtime_error(reader.get_error_str());

  // Here 'is_active()' is called after 'get_binlog_end_pos()' deliberately
  // to properly handle the situation when rotation happens between these
  // two calls
  my_off_t end_pos = mysql_bin_log.get_binlog_end_pos();
  if (!mysql_bin_log.is_active(search_file_name))
    end_pos = std::numeric_limits<my_off_t>::max();

  log_event_ptr ev;
  binlog::Decompressing_event_object_istream istream{reader};

  while (istream >> ev) {
    if (reader.has_fatal_error())
      throw std::runtime_error(reader.get_error_str());
    if (ev->get_type_code() == mysql::binlog::event::PREVIOUS_GTIDS_LOG_EVENT)
      return ev;
    if (ev->common_header->log_pos >= end_pos) break;
  }
  if (istream.has_error()) throw std::runtime_error(istream.get_error_str());
  return {};
}

bool extract_previous_gtids(std::string_view binlog_name, bool is_first,
                            Gtid_set &extracted_gtids) {
  DBUG_TRACE;

  bool res = false;
  auto ev = find_previous_gtids_event(binlog_name);

  if (!ev) {
    if (!is_first)
      throw std::runtime_error(
          "Encountered binary log without PREVIOUS_GTIDS_LOG_EVENT in the "
          "middle of log index");
    extracted_gtids.clear();
  } else {
    assert(ev->get_type_code() ==
           mysql::binlog::event::PREVIOUS_GTIDS_LOG_EVENT);
    auto *casted_ev = static_cast<Previous_gtids_log_event *>(ev.get());
    extracted_gtids.clear();
    casted_ev->add_to_set(&extracted_gtids);
    res = true;
  }
  return res;
}

log_event_ptr find_last_gtid_event(std::string_view binlog_name) {
  DBUG_TRACE;

  fn_reflen_buffer binlog_name_buffer;
  auto search_file_name =
      check_and_normalize_binlog_name(binlog_name, binlog_name_buffer);

  Binlog_file_reader reader(false /* do not verify checksum */);
  if (reader.open(search_file_name, 0))
    throw std::runtime_error(reader.get_error_str());

  // Here 'is_active()' is called after 'get_binlog_end_pos()' deliberately
  // to properly handle the situation when rotation happens between these
  // two calls
  my_off_t end_pos = mysql_bin_log.get_binlog_end_pos();
  if (!mysql_bin_log.is_active(search_file_name))
    end_pos = std::numeric_limits<my_off_t>::max();

  log_event_ptr ev, last_gtid_ev;
  binlog::Decompressing_event_object_istream istream{reader};

  while (istream >> ev) {
    if (reader.has_fatal_error())
      throw std::runtime_error(reader.get_error_str());
    auto ev_row = ev.get();
    if (ev_row->get_type_code() == mysql::binlog::event::GTID_LOG_EVENT ||
        ev_row->get_type_code() == mysql::binlog::event::GTID_TAGGED_LOG_EVENT)
      last_gtid_ev = std::move(ev);
    if (ev_row->common_header->log_pos >= end_pos) break;
  }
  if (istream.has_error()) throw std::runtime_error(istream.get_error_str());
  return last_gtid_ev;
}

bool extract_last_gtid(std::string_view binlog_name, Tsid_map &tsid_map,
                       Gtid &extracted_gtid) {
  DBUG_TRACE;

  auto ev = find_last_gtid_event(binlog_name);
  if (!ev) return false;

  assert(ev->get_type_code() == mysql::binlog::event::GTID_LOG_EVENT ||
         ev->get_type_code() == mysql::binlog::event::GTID_TAGGED_LOG_EVENT);
  auto *casted_ev = static_cast<Gtid_log_event *>(ev.get());
  rpl_sidno sidno = casted_ev->get_sidno(&tsid_map);
  if (sidno < 0) throw std::runtime_error("Invalid GTID event encountered");
  extracted_gtid.set(sidno, casted_ev->get_gno());
  return true;
}

//
// GET_BINLOG_BY_GTID()
// This MySQL function accepts a GTID and returns the name of the binlog file
// that contains this GTID.
//
class get_binlog_by_gtid_impl {
 public:
  get_binlog_by_gtid_impl(mysqlpp::udf_context &ctx) {
    DBUG_TRACE;

    if (ctx.get_number_of_args() != 1)
      throw std::invalid_argument("Function requires exactly one argument");
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);
  }
  ~get_binlog_by_gtid_impl() { DBUG_TRACE; }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &args);
};

mysqlpp::udf_result_t<STRING_RESULT> get_binlog_by_gtid_impl::calculate(
    const mysqlpp::udf_context &ctx) {
  DBUG_TRACE;

  auto gtid_text = static_cast<std::string>(ctx.get_arg<STRING_RESULT>(0));
  Tsid_map tsid_map{nullptr};
  Gtid gtid;
  if (gtid.parse(&tsid_map, gtid_text.c_str()) != Return_status::ok)
    throw std::invalid_argument("Invalid GTID specified");

  Gtid_set covering_gtids{&tsid_map};

  {
    uni_buffer_t ub{};
    auto gtid_executed_sv = extract_sys_var_value(
        default_component_name, gtid_executed_variable_name, ub);

    auto gtid_set_parse_result =
        covering_gtids.add_gtid_text(gtid_executed_sv.data());
    if (gtid_set_parse_result != RETURN_STATUS_OK)
      throw std::runtime_error("Cannot parse 'gtid_executed'");
  }

  auto log_index = mysql_bin_log.get_log_index(true /* need_lock_index */);
  if (log_index.first != LOG_INFO_EOF)
    throw std::runtime_error("Cannot read binary log index'");
  if (log_index.second.empty())
    throw std::runtime_error("Binary log index is empty'");
  auto rit = std::crbegin(log_index.second);
  auto ren = std::crend(log_index.second);
  auto bg = std::cbegin(log_index.second);
  bool found{false};
  do {
    Gtid_set extracted_gtids{&tsid_map};
    extract_previous_gtids(get_short_binlog_name(*rit), rit.base() == bg,
                           extracted_gtids);
    found = covering_gtids.contains_gtid(gtid) &&
            !extracted_gtids.contains_gtid(gtid);
    if (!found) {
      covering_gtids.clear();
      covering_gtids.add_gtid_set(&extracted_gtids);
      ++rit;
    }
  } while (!found && rit != ren);
  if (!found) return {};
  return {std::string{get_short_binlog_name(*rit)}};
}

//
// GET_LAST_GTID_FROM_BINLOG()
// This MySQL function accepts a binlog file name and returns the last GTID
// found in this binlog
//
class get_last_gtid_from_binlog_impl {
 public:
  get_last_gtid_from_binlog_impl(mysqlpp::udf_context &ctx) {
    DBUG_TRACE;

    if (ctx.get_number_of_args() != 1)
      throw std::invalid_argument("Function requires exactly one argument");
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);
  }
  ~get_last_gtid_from_binlog_impl() { DBUG_TRACE; }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &args);
};

mysqlpp::udf_result_t<STRING_RESULT> get_last_gtid_from_binlog_impl::calculate(
    const mysqlpp::udf_context &ctx) {
  DBUG_TRACE;

  Tsid_map tsid_map{nullptr};
  Gtid extracted_gtid;
  if (!extract_last_gtid(ctx.get_arg<STRING_RESULT>(0), tsid_map,
                         extracted_gtid))
    return {};

  char buf[Gtid::MAX_TEXT_LENGTH + 1];
  auto length =
      static_cast<std::size_t>(extracted_gtid.to_string(&tsid_map, buf));

  return mysqlpp::udf_result_t<STRING_RESULT>{std::in_place, buf, length};
}

//
// GET_GTID_SET_BY_BINLOG()
// This MySQL function accepts a binlog file name and returns all GTIDs that
// are stored inside this binlog
//
class get_gtid_set_by_binlog_impl {
 public:
  get_gtid_set_by_binlog_impl(mysqlpp::udf_context &ctx) {
    DBUG_TRACE;

    if (ctx.get_number_of_args() != 1)
      throw std::invalid_argument("Function requires exactly one argument");
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);
  }
  ~get_gtid_set_by_binlog_impl() { DBUG_TRACE; }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &args);
};

mysqlpp::udf_result_t<STRING_RESULT> get_gtid_set_by_binlog_impl::calculate(
    const mysqlpp::udf_context &ctx) {
  DBUG_TRACE;

  auto log_index = mysql_bin_log.get_log_index(true /* need_lock_index */);
  if (log_index.first != LOG_INFO_EOF)
    throw std::runtime_error("Cannot read binary log index");
  if (log_index.second.empty())
    throw std::runtime_error("Binary log index is empty");

  // trying to find the specified binlog name in the index
  auto binlog_name_sv = ctx.get_arg<STRING_RESULT>(0);
  auto bg = std::cbegin(log_index.second);
  auto en = std::cend(log_index.second);
  fn_reflen_buffer binlog_name_buffer;
  auto normalized_binlog_name =
      check_and_normalize_binlog_name(binlog_name_sv, binlog_name_buffer);
  auto fnd = boost::algorithm::find_backward(bg, en, normalized_binlog_name);
  if (fnd == en) throw std::runtime_error("Binary log does not exist");

  // if found, reading previous GTIDs from it
  Tsid_map tsid_map{nullptr};
  Gtid_set extracted_gtids{&tsid_map};
  extract_previous_gtids(get_short_binlog_name(*fnd), fnd == bg,
                         extracted_gtids);

  Gtid_set covering_gtids{&tsid_map};
  --en;
  if (fnd == en) {
    // if the found binlog is the last in the list (the active one),
    // extract covering GTIDs from the 'gtid_executed' system variable
    // via sys_var plugin service
    uni_buffer_t ub{};
    auto gtid_executed_sv = extract_sys_var_value(
        default_component_name, gtid_executed_variable_name, ub);

    auto gtid_set_parse_result =
        covering_gtids.add_gtid_text(gtid_executed_sv.data());
    if (gtid_set_parse_result != RETURN_STATUS_OK)
      throw std::runtime_error("Cannot parse 'gtid_executed'");
  } else {
    // if the found binlog is not the last in the list (not the active one),
    // extract covering GTIDs from the next binlog

    ++fnd;
    extract_previous_gtids(get_short_binlog_name(*fnd), fnd == bg,
                           covering_gtids);
  }
  covering_gtids.remove_gtid_set(&extracted_gtids);
  dynamic_buffer_t result_buffer(covering_gtids.get_string_length() + 1);
  auto length = covering_gtids.to_string(result_buffer.data());

  return mysqlpp::udf_result_t<STRING_RESULT>{std::in_place,
                                              result_buffer.data(), length};
}

//
// GET_BINLOG_BY_GTID_SET()
// This MySQL function accepts a GTID set and returns the name of the oldest
// binlog file that contains at least one of those GTIDs
//
class get_binlog_by_gtid_set_impl {
 public:
  get_binlog_by_gtid_set_impl(mysqlpp::udf_context &ctx) {
    DBUG_TRACE;

    if (ctx.get_number_of_args() != 1)
      throw std::invalid_argument("Function requires exactly one argument");
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);
  }
  ~get_binlog_by_gtid_set_impl() { DBUG_TRACE; }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &args);
};

mysqlpp::udf_result_t<STRING_RESULT> get_binlog_by_gtid_set_impl::calculate(
    const mysqlpp::udf_context &ctx) {
  DBUG_TRACE;

  auto gtid_set_text = static_cast<std::string>(ctx.get_arg<STRING_RESULT>(0));
  Tsid_map tsid_map{nullptr};
  Gtid_set gtid_set{&tsid_map};
  auto gtid_set_parse_result = gtid_set.add_gtid_text(gtid_set_text.c_str());
  if (gtid_set_parse_result != RETURN_STATUS_OK)
    throw std::runtime_error("Cannot parse GTID set");

  Gtid_set covering_gtids{&tsid_map};

  {
    uni_buffer_t ub{};
    auto gtid_executed_sv = extract_sys_var_value(
        default_component_name, gtid_executed_variable_name, ub);

    gtid_set_parse_result =
        covering_gtids.add_gtid_text(gtid_executed_sv.data());
    if (gtid_set_parse_result != RETURN_STATUS_OK)
      throw std::runtime_error("Cannot parse 'gtid_executed'");
  }

  auto log_index = mysql_bin_log.get_log_index(true /* need_lock_index */);
  if (log_index.first != LOG_INFO_EOF)
    throw std::runtime_error("Cannot read binary log index'");
  if (log_index.second.empty())
    throw std::runtime_error("Binary log index is empty'");
  auto rit = std::crbegin(log_index.second);
  auto ren = std::crend(log_index.second);
  auto bg = std::cbegin(log_index.second);

  bool encountered_nonempty_intersection{false};
  bool found{false};
  do {
    Gtid_set extracted_gtids{&tsid_map};
    extract_previous_gtids(get_short_binlog_name(*rit), rit.base() == bg,
                           extracted_gtids);
    covering_gtids.remove_gtid_set(&extracted_gtids);
    bool current_nonempty_intersection =
        covering_gtids.is_intersection_nonempty(&gtid_set);
    encountered_nonempty_intersection =
        encountered_nonempty_intersection || current_nonempty_intersection;
    found = encountered_nonempty_intersection && !current_nonempty_intersection;
    if (!found) {
      covering_gtids.clear();
      covering_gtids.add_gtid_set(&extracted_gtids);
      ++rit;
    }
  } while (!found && rit != ren);

  if (!encountered_nonempty_intersection) return {};

  --rit;
  return {std::string{get_short_binlog_name(*rit)}};
}

//
// get_first_record_timestamp_by_binlog()
// This MySQL function accepts a binlog file name and returns timestamp
// of the first record (number of microseconds since 1-Jan-1970)
//
class get_first_record_timestamp_by_binlog_impl {
 public:
  get_first_record_timestamp_by_binlog_impl(mysqlpp::udf_context &ctx) {
    DBUG_TRACE;

    if (ctx.get_number_of_args() != 1)
      throw std::invalid_argument("Function requires exactly one argument");
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);
  }
  ~get_first_record_timestamp_by_binlog_impl() { DBUG_TRACE; }

  mysqlpp::udf_result_t<INT_RESULT> calculate(const mysqlpp::udf_context &args);
};

mysqlpp::udf_result_t<INT_RESULT>
get_first_record_timestamp_by_binlog_impl::calculate(
    const mysqlpp::udf_context &ctx) {
  DBUG_TRACE;

  // trying to find the specified binlog name in the index
  auto binlog_name_sv = ctx.get_arg<STRING_RESULT>(0);

  auto ev = find_first_event(binlog_name_sv);
  if (!ev) return {std::nullopt};
  return ev->common_header->when.tv_sec * 1000000LL +
         ev->common_header->when.tv_usec;
}

//
// get_last_record_timestamp_by_binlog()
// This MySQL function accepts a binlog file name and returns timestamp
// of the last record (number of microseconds since 1-Jan-1970)
//
class get_last_record_timestamp_by_binlog_impl {
 public:
  get_last_record_timestamp_by_binlog_impl(mysqlpp::udf_context &ctx) {
    DBUG_TRACE;

    if (ctx.get_number_of_args() != 1)
      throw std::invalid_argument("Function requires exactly one argument");
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);
  }
  ~get_last_record_timestamp_by_binlog_impl() { DBUG_TRACE; }

  mysqlpp::udf_result_t<INT_RESULT> calculate(const mysqlpp::udf_context &args);
};

mysqlpp::udf_result_t<INT_RESULT>
get_last_record_timestamp_by_binlog_impl::calculate(
    const mysqlpp::udf_context &ctx) {
  DBUG_TRACE;

  // trying to find the specified binlog name in the index
  auto binlog_name_sv = ctx.get_arg<STRING_RESULT>(0);

  auto ev = find_last_event(binlog_name_sv);
  if (!ev) return {std::nullopt};
  return ev->common_header->when.tv_sec * 1000000LL +
         ev->common_header->when.tv_usec;
}

}  // end of anonymous namespace

DECLARE_STRING_UDF_AUTO(get_binlog_by_gtid)
DECLARE_STRING_UDF_AUTO(get_last_gtid_from_binlog)
DECLARE_STRING_UDF_AUTO(get_gtid_set_by_binlog)
DECLARE_STRING_UDF_AUTO(get_binlog_by_gtid_set)
DECLARE_INT_UDF_AUTO(get_first_record_timestamp_by_binlog)
DECLARE_INT_UDF_AUTO(get_last_record_timestamp_by_binlog)

static const std::array known_udfs{
    DECLARE_UDF_INFO(get_binlog_by_gtid, STRING_RESULT),
    DECLARE_UDF_INFO(get_last_gtid_from_binlog, STRING_RESULT),
    DECLARE_UDF_INFO(get_gtid_set_by_binlog, STRING_RESULT),
    DECLARE_UDF_INFO(get_binlog_by_gtid_set, STRING_RESULT),
    DECLARE_UDF_INFO(get_first_record_timestamp_by_binlog, INT_RESULT),
    DECLARE_UDF_INFO(get_last_record_timestamp_by_binlog, INT_RESULT)};

#undef DECLARE_UDF_INFO

static void binlog_utils_my_error(int error_id, myf flags, ...) {
  va_list args;
  va_start(args, flags);
  mysql_service_mysql_runtime_error->emit(error_id, flags, args);
  va_end(args);
}

using udf_bitset_type =
    std::bitset<std::tuple_size<decltype(known_udfs)>::value>;
static udf_bitset_type registered_udfs;

static mysql_service_status_t component_init() {
  // here we use a custom error reporting function
  // 'binlog_utils_my_error()' based on the
  // 'mysql_service_mysql_runtime_error' service instead of the standard
  // 'my_error()' from 'mysys' to get rid of the 'mysys' dependency for this
  // component
  mysqlpp::udf_error_reporter::instance() = &binlog_utils_my_error;

  mysqlpp::register_udfs(mysql_service_udf_registration, known_udfs,
                         registered_udfs);
  return registered_udfs.all() ? 0 : 1;
}

static mysql_service_status_t component_deinit() {
  mysqlpp::unregister_udfs(mysql_service_udf_registration, known_udfs,
                           registered_udfs);
  return registered_udfs.none() ? 0 : 1;
}

// clang-format off
BEGIN_COMPONENT_PROVIDES(CURRENT_COMPONENT_NAME)
END_COMPONENT_PROVIDES();

BEGIN_COMPONENT_REQUIRES(CURRENT_COMPONENT_NAME)
  REQUIRES_SERVICE(udf_registration),
  REQUIRES_SERVICE(component_sys_variable_register),
END_COMPONENT_REQUIRES();

BEGIN_COMPONENT_METADATA(CURRENT_COMPONENT_NAME)
  METADATA("mysql.author", "Percona Corporation"),
  METADATA("mysql.license", "GPL"),
END_COMPONENT_METADATA();

DECLARE_COMPONENT(CURRENT_COMPONENT_NAME, CURRENT_COMPONENT_NAME_STR)
  component_init,
  component_deinit,
END_DECLARE_COMPONENT();
// clang-format on

DECLARE_LIBRARY_COMPONENTS &COMPONENT_REF(CURRENT_COMPONENT_NAME)
    END_DECLARE_LIBRARY_COMPONENTS
