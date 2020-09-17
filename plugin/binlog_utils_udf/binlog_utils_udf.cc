#include <array>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <mysql/plugin.h>

#include <mysql/components/services/component_sys_var_service.h>

#include <mysqlpp/udf_wrappers.hpp>

#include <sql/binlog.h>
#include <sql/binlog/tools/iterators.h>
#include <sql/binlog_reader.h>

static bool binlog_utils_udf_initialized{false};

struct registry_service_releaser {
  void operator()(SERVICE_TYPE(registry) * srv) const noexcept {
    if (srv != nullptr) mysql_plugin_registry_release(srv);
  }
};
using registry_service_ptr =
    std::unique_ptr<SERVICE_TYPE(registry), registry_service_releaser>;

static registry_service_ptr reg_srv{nullptr, registry_service_releaser{}};

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

static component_sys_variable_register_ptr sys_var_srv{
    nullptr, component_sys_variable_register_releaser{reg_srv}};

static int binlog_utils_udf_init(void *) {
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

static int binlog_utils_udf_deinit(void *) {
  DBUG_TRACE;
  sys_var_srv.reset();
  reg_srv.reset();
  binlog_utils_udf_initialized = false;
  return 0;
}

struct st_mysql_daemon binlog_utils_udf_decriptor = {
    MYSQL_DAEMON_INTERFACE_VERSION};

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

class get_binlog_by_gtid_capsule {
 public:
  get_binlog_by_gtid_capsule(mysqlpp::udf_context &ctx) {
    DBUG_TRACE;

    if (!binlog_utils_udf_initialized)
      throw std::invalid_argument(
          "This function requires binlog_utils_udf plugin which is not "
          "installed.");

    if (ctx.get_number_of_args() != 1)
      throw std::invalid_argument(
          "GET_BINLOG_BY_GTID() requires exactly one argument");
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);
  }
  ~get_binlog_by_gtid_capsule() { DBUG_TRACE; }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &args);

 private:
  using log_event_ptr = std::unique_ptr<Log_event>;
  log_event_ptr find_previous_gtids_event(ext::string_view binlog_name);
};

mysqlpp::udf_result_t<STRING_RESULT> get_binlog_by_gtid_capsule::calculate(
    const mysqlpp::udf_context &ctx) {
  DBUG_TRACE;

  auto gtid_text = static_cast<std::string>(ctx.get_arg<STRING_RESULT>(0));
  Sid_map sid_map{nullptr};
  Gtid gtid;
  if (gtid.parse(&sid_map, gtid_text.c_str()) != RETURN_STATUS_OK)
    throw std::invalid_argument("Invalid GTID specified");

  Gtid_set covering_gtids{&sid_map};

  {
    constexpr std::size_t initial_buffer_size{1024};
    using static_buffer_t = std::array<char, initial_buffer_size + 1>;
    static_buffer_t static_buffer{};

    using dynamic_buffer_t = std::vector<char>;
    dynamic_buffer_t dynamic_buffer{};
    void *ptr = static_buffer.data();
    std::size_t length = initial_buffer_size;

    if (sys_var_srv->get_variable("mysql_server", "gtid_executed", &ptr,
                                  &length)) {
      dynamic_buffer.resize(length + 1);
      ptr = dynamic_buffer.data();
      if (sys_var_srv->get_variable("mysql_server", "gtid_executed", &ptr,
                                    &length))
        throw std::runtime_error("Cannot get 'gtid_executed'");
    }

    auto gtid_set_parse_result =
        covering_gtids.add_gtid_text(static_cast<char *>(ptr));
    if (gtid_set_parse_result != RETURN_STATUS_OK)
      throw std::runtime_error("Cannot parse 'gtid_executed'");
  }

  auto log_index = mysql_bin_log.get_log_index(true /* need_lock_index */);
  if (log_index.first != LOG_INFO_EOF)
    throw std::runtime_error("Cannot read binary log index'");
  if (log_index.second.empty())
    throw std::runtime_error("Binary log index is empty'");
  auto it = log_index.second.crbegin();
  auto en = log_index.second.crend();
  bool found{false};
  do {
    auto ev = find_previous_gtids_event(*it);
    Gtid_set extracted_gtids{&sid_map};
    if (!ev) {
      if (*it != log_index.second.front())
        throw std::runtime_error(
            "Encountered binary log without PREVIOUS_GTIDS_LOG_EVENT in the "
            "middle of log index'");
    } else {
      assert(ev->get_type_code() == binary_log::PREVIOUS_GTIDS_LOG_EVENT);
      auto *casted_ev = static_cast<Previous_gtids_log_event *>(ev.get());
      casted_ev->add_to_set(&extracted_gtids);
    }
    found = covering_gtids.contains_gtid(gtid) &&
            !extracted_gtids.contains_gtid(gtid);
    if (!found) {
      covering_gtids.clear();
      covering_gtids.add_gtid_set(&extracted_gtids);
      ++it;
    }
  } while (!found && it != en);
  if (!found) return {};

  return {*it};
}

get_binlog_by_gtid_capsule::log_event_ptr
get_binlog_by_gtid_capsule::find_previous_gtids_event(
    ext::string_view binlog_name) {
  DBUG_TRACE;

  std::string casted_binlog_name = static_cast<std::string>(binlog_name);

  char search_file_name[FN_REFLEN + 1];
  mysql_bin_log.make_log_name(search_file_name, casted_binlog_name.c_str());

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

DECLARE_STRING_UDF(get_binlog_by_gtid_capsule, get_binlog_by_gtid)

class get_last_gtid_from_binlog_capsule {
 public:
  get_last_gtid_from_binlog_capsule(mysqlpp::udf_context &ctx) {
    DBUG_TRACE;

    if (!binlog_utils_udf_initialized)
      throw std::invalid_argument(
          "This function requires binlog_utils_udf plugin which is not "
          "installed.");

    if (ctx.get_number_of_args() != 1)
      throw std::invalid_argument(
          "GET_LAST_GTID_FROM_BINLOG() requires exactly one argument");
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);
  }
  ~get_last_gtid_from_binlog_capsule() { DBUG_TRACE; }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &args);

 private:
  using log_event_ptr = std::shared_ptr<Log_event>;
  log_event_ptr find_last_gtid_event(ext::string_view binlog_name);
};

mysqlpp::udf_result_t<STRING_RESULT>
get_last_gtid_from_binlog_capsule::calculate(const mysqlpp::udf_context &ctx) {
  DBUG_TRACE;

  Sid_map sid_map{nullptr};

  auto ev = find_last_gtid_event(ctx.get_arg<STRING_RESULT>(0));
  if (!ev) return {};

  assert(ev->get_type_code() == binary_log::GTID_LOG_EVENT);
  auto *casted_ev = static_cast<Gtid_log_event *>(ev.get());
  rpl_sidno sidno = casted_ev->get_sidno(&sid_map);
  if (sidno < 0) throw std::runtime_error("Invalid GTID event encountered");
  Gtid gtid;
  gtid.set(sidno, casted_ev->get_gno());

  char buf[Gtid::MAX_TEXT_LENGTH + 1];
  auto length = static_cast<std::size_t>(gtid.to_string(&sid_map, buf));

  return mysqlpp::udf_result_t<STRING_RESULT>{boost::in_place_init, buf,
                                              length};
}

get_last_gtid_from_binlog_capsule::log_event_ptr
get_last_gtid_from_binlog_capsule::find_last_gtid_event(
    ext::string_view binlog_name) {
  DBUG_TRACE;

  std::string casted_binlog_name = static_cast<std::string>(binlog_name);

  char search_file_name[FN_REFLEN + 1];
  mysql_bin_log.make_log_name(search_file_name, casted_binlog_name.c_str());

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
  log_event_ptr last_gtid_ev;
  binlog::tools::Iterator it(&reader);

  for (log_event_ptr ev{it.begin()}; ev.get() != it.end();) {
    if (reader.has_fatal_error())
      throw std::runtime_error(reader.get_error_str());
    if (it.has_error()) throw std::runtime_error(it.get_error_message());
    if (ev->get_type_code() == binary_log::GTID_LOG_EVENT) last_gtid_ev = ev;
    if (ev->common_header->log_pos >= end_pos) break;
    ev.reset(it.next());
  }
  return last_gtid_ev;
}

DECLARE_STRING_UDF(get_last_gtid_from_binlog_capsule, get_last_gtid_from_binlog)
