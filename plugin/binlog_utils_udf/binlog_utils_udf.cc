#include <array>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <boost/algorithm/find_backward.hpp>

#include <mysql/plugin.h>

#include <mysql/components/services/component_sys_var_service.h>

#include <mysqlpp/udf_wrappers.hpp>

#include <sql/binlog.h>
#include <sql/binlog/tools/iterators.h>
#include <sql/binlog_reader.h>

namespace {

bool binlog_utils_udf_initialized{false};

struct registry_service_releaser {
  void operator()(SERVICE_TYPE(registry) * srv) const noexcept {
    if (srv != nullptr) mysql_plugin_registry_release(srv);
  }
};
using registry_service_ptr =
    std::unique_ptr<SERVICE_TYPE(registry), registry_service_releaser>;

registry_service_ptr reg_srv{nullptr, registry_service_releaser{}};

struct component_sys_variable_register_releaser {
  registry_service_ptr &parent;
  void operator()(SERVICE_TYPE(component_sys_variable_register) *
                  srv) const noexcept {
    if (parent && srv != nullptr)
      parent->release(reinterpret_cast<my_h_service>(
          const_cast<SERVICE_TYPE_NO_CONST(component_sys_variable_register) *>(
              srv)));
  }
};
using component_sys_variable_register_ptr =
    std::unique_ptr<SERVICE_TYPE(component_sys_variable_register),
                    component_sys_variable_register_releaser>;

component_sys_variable_register_ptr sys_var_srv{
    nullptr, component_sys_variable_register_releaser{reg_srv}};

int binlog_utils_udf_init(void *) {
  DBUG_TRACE;
  registry_service_ptr local_reg_srv{mysql_plugin_registry_acquire(),
                                     registry_service_releaser{}};
  if (!local_reg_srv) return 1;
  my_h_service acquired_service{nullptr};
  if (local_reg_srv->acquire("component_sys_variable_register",
                             &acquired_service) != 0)
    return 1;
  if (acquired_service == nullptr) return 1;

  reg_srv = std::move(local_reg_srv);
  sys_var_srv.reset(
      reinterpret_cast<SERVICE_TYPE(component_sys_variable_register) *>(
          acquired_service));
  binlog_utils_udf_initialized = true;
  return 0;
}

int binlog_utils_udf_deinit(void *) {
  DBUG_TRACE;
  sys_var_srv.reset();
  reg_srv.reset();
  binlog_utils_udf_initialized = false;
  return 0;
}

struct st_mysql_daemon binlog_utils_udf_decriptor = {
    MYSQL_DAEMON_INTERFACE_VERSION};

}  // end of anonymous namespace

/*
  Plugin library descriptor
*/

mysql_declare_plugin(binlog_utils_udf){
    MYSQL_DAEMON_PLUGIN,
    &binlog_utils_udf_decriptor,
    "binlog_utils_udf",
    PLUGIN_AUTHOR_ORACLE,
    "Binlog utils UDF plugin",
    PLUGIN_LICENSE_GPL,
    binlog_utils_udf_init,   /* Plugin Init */
    nullptr,                 /* Plugin check uninstall */
    binlog_utils_udf_deinit, /* Plugin Deinit */
    0x0100 /* 1.0 */,
    nullptr, /* status variables                */
    nullptr, /* system variables                */
    nullptr, /* config options                  */
    0,       /* flags                           */
} mysql_declare_plugin_end;

namespace {

//
// Binlog utils shared functions
//
const ext::string_view default_component_name{"mysql_server"};
const ext::string_view gtid_executed_variable_name{"gtid_executed"};

constexpr std::size_t default_static_buffer_size{1024};
using static_buffer_t = std::array<char, default_static_buffer_size + 1>;
using dynamic_buffer_t = std::vector<char>;
using uni_buffer_t = std::pair<static_buffer_t, dynamic_buffer_t>;

ext::string_view extract_sys_var_value(ext::string_view component_name,
                                       ext::string_view variable_name,
                                       uni_buffer_t &ub) {
  DBUG_TRACE;
  void *ptr = ub.first.data();
  std::size_t length = ub.first.size() - 1;

  if (sys_var_srv->get_variable(component_name.data(), variable_name.data(),
                                &ptr, &length) == 0)
    return {static_cast<char *>(ptr), length};

  ub.second.resize(length + 1);
  ptr = ub.second.data();
  if (sys_var_srv->get_variable(component_name.data(), variable_name.data(),
                                &ptr, &length) != 0)
    throw std::runtime_error("Cannot get sys_var value");

  if (ptr == nullptr) throw std::runtime_error("The value of sys_var is null");

  return {static_cast<char *>(ptr), length};
}

using log_event_ptr = std::unique_ptr<Log_event>;
using fn_reflen_buffer = char[FN_REFLEN + 1];

const char *check_and_normalize_binlog_name(ext::string_view binlog_name,
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

log_event_ptr find_first_event(ext::string_view binlog_name) {
  DBUG_TRACE;

  fn_reflen_buffer binlog_name_buffer;
  auto search_file_name =
      check_and_normalize_binlog_name(binlog_name, binlog_name_buffer);

  Binlog_file_reader reader(false /* do not verify checksum */);
  if (reader.open(search_file_name, 0))
    throw std::runtime_error(reader.get_error_str());

  binlog::tools::Iterator it(&reader);
  log_event_ptr ev{it.begin()};

  if (reader.has_fatal_error())
    throw std::runtime_error(reader.get_error_str());
  if (it.has_error()) throw std::runtime_error(it.get_error_message());

  return ev;
}

log_event_ptr find_last_event(ext::string_view binlog_name) {
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

  binlog::tools::Iterator it(&reader);
  log_event_ptr ev{it.begin()};

  while (true) {
    if (reader.has_fatal_error())
      throw std::runtime_error(reader.get_error_str());
    if (it.has_error()) throw std::runtime_error(it.get_error_message());
    if (ev->common_header->log_pos >= end_pos) break;
    log_event_ptr next_ev{it.next()};
    if (next_ev.get() == it.end()) break;
    ev.swap(next_ev);
  }
  return ev;
}

log_event_ptr find_previous_gtids_event(ext::string_view binlog_name) {
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
  binlog::tools::Iterator it(&reader);

  for (log_event_ptr ev{it.begin()}; ev.get() != it.end();) {
    if (reader.has_fatal_error())
      throw std::runtime_error(reader.get_error_str());
    if (it.has_error()) throw std::runtime_error(it.get_error_message());
    if (ev->get_type_code() == binary_log::PREVIOUS_GTIDS_LOG_EVENT) return ev;
    if (ev->common_header->log_pos >= end_pos) break;
    ev.reset(it.next());
  }
  return {};
}

bool extract_previous_gtids(ext::string_view binlog_name, bool is_first,
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
    assert(ev->get_type_code() == binary_log::PREVIOUS_GTIDS_LOG_EVENT);
    auto *casted_ev = static_cast<Previous_gtids_log_event *>(ev.get());
    extracted_gtids.clear();
    casted_ev->add_to_set(&extracted_gtids);
    res = true;
  }
  return res;
}

log_event_ptr find_last_gtid_event(ext::string_view binlog_name) {
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

  log_event_ptr last_gtid_ev;
  binlog::tools::Iterator it(&reader);

  for (log_event_ptr ev{it.begin()}; ev.get() != it.end();) {
    if (reader.has_fatal_error())
      throw std::runtime_error(reader.get_error_str());
    if (it.has_error()) throw std::runtime_error(it.get_error_message());
    auto ev_row = ev.get();
    if (ev_row->get_type_code() == binary_log::GTID_LOG_EVENT)
      last_gtid_ev = std::move(ev);
    if (ev_row->common_header->log_pos >= end_pos) break;
    ev.reset(it.next());
  }
  return last_gtid_ev;
}

bool extract_last_gtid(ext::string_view binlog_name, Sid_map &sid_map,
                       Gtid &extracted_gtid) {
  DBUG_TRACE;

  auto ev = find_last_gtid_event(binlog_name);
  if (!ev) return false;

  assert(ev->get_type_code() == binary_log::GTID_LOG_EVENT);
  auto *casted_ev = static_cast<Gtid_log_event *>(ev.get());
  rpl_sidno sidno = casted_ev->get_sidno(&sid_map);
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

    if (!binlog_utils_udf_initialized)
      throw std::invalid_argument(
          "This function requires binlog_utils_udf plugin which is not "
          "installed.");

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
  Sid_map sid_map{nullptr};
  Gtid gtid;
  if (gtid.parse(&sid_map, gtid_text.c_str()) != RETURN_STATUS_OK)
    throw std::invalid_argument("Invalid GTID specified");

  Gtid_set covering_gtids{&sid_map};

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
    Gtid_set extracted_gtids{&sid_map};
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

    if (!binlog_utils_udf_initialized)
      throw std::invalid_argument(
          "This function requires binlog_utils_udf plugin which is not "
          "installed.");

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

  Sid_map sid_map{nullptr};
  Gtid extracted_gtid;
  if (!extract_last_gtid(ctx.get_arg<STRING_RESULT>(0), sid_map,
                         extracted_gtid))
    return {};

  char buf[Gtid::MAX_TEXT_LENGTH + 1];
  auto length =
      static_cast<std::size_t>(extracted_gtid.to_string(&sid_map, buf));

  return mysqlpp::udf_result_t<STRING_RESULT>{boost::in_place_init, buf,
                                              length};
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

    if (!binlog_utils_udf_initialized)
      throw std::invalid_argument(
          "This function requires binlog_utils_udf plugin which is not "
          "installed.");

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
  Sid_map sid_map{nullptr};
  Gtid_set extracted_gtids{&sid_map};
  extract_previous_gtids(get_short_binlog_name(*fnd), fnd == bg,
                         extracted_gtids);

  Gtid_set covering_gtids{&sid_map};
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

  return mysqlpp::udf_result_t<STRING_RESULT>{boost::in_place_init,
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

    if (!binlog_utils_udf_initialized)
      throw std::invalid_argument(
          "This function requires binlog_utils_udf plugin which is not "
          "installed.");

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
  Sid_map sid_map{nullptr};
  Gtid_set gtid_set{&sid_map};
  auto gtid_set_parse_result = gtid_set.add_gtid_text(gtid_set_text.c_str());
  if (gtid_set_parse_result != RETURN_STATUS_OK)
    throw std::runtime_error("Cannot parse GTID set");

  Gtid_set covering_gtids{&sid_map};

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
    Gtid_set extracted_gtids{&sid_map};
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

    if (!binlog_utils_udf_initialized)
      throw std::invalid_argument(
          "This function requires binlog_utils_udf plugin which is not "
          "installed.");

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
  if (!ev) return {};
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

    if (!binlog_utils_udf_initialized)
      throw std::invalid_argument(
          "This function requires binlog_utils_udf plugin which is not "
          "installed.");

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
  if (!ev) return {};
  return ev->common_header->when.tv_sec * 1000000LL +
         ev->common_header->when.tv_usec;
}

}  // end of anonymous namespace

DECLARE_STRING_UDF(get_binlog_by_gtid_impl, get_binlog_by_gtid)

DECLARE_STRING_UDF(get_last_gtid_from_binlog_impl, get_last_gtid_from_binlog)

DECLARE_STRING_UDF(get_gtid_set_by_binlog_impl, get_gtid_set_by_binlog)

DECLARE_STRING_UDF(get_binlog_by_gtid_set_impl, get_binlog_by_gtid_set)

DECLARE_INT_UDF(get_first_record_timestamp_by_binlog_impl,
                get_first_record_timestamp_by_binlog)

DECLARE_INT_UDF(get_last_record_timestamp_by_binlog_impl,
                get_last_record_timestamp_by_binlog)
