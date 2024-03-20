#include <cassert>

#include "common.h"
#include "config.h"

namespace {
constexpr char VAR_TELEMETRY_ROOT_DIR[] = "telemetry_root_dir";
constexpr char VAR_SCRAPE_INTERVAL[] = "scrape_interval";
constexpr char VAR_GRACE_INTERVAL[] = "grace_interval";
constexpr char VAR_HISTORY_KEEP_INTERVAL[] = "history_keep_interval";
constexpr char TELEMETRY_ROOT_DIR_DEFAULT[] = "/usr/local/percona/telemetry/ps";

constexpr uint SCRAPE_INTERVAL_DEFAULT = 60 * 60 * 24 * 1;  // 1 day
constexpr uint SCRAPE_INTERVAL_MIN = 10;
constexpr uint SCRAPE_INTERVAL_MAX = 60 * 60 * 24 * 7;     // 1 week
constexpr uint GRACE_INTERVAL_DEFAULT = 60 * 60 * 24 * 1;  // 1 day
constexpr uint GRACE_INTERVAL_MIN = 20;
constexpr uint GRACE_INTERVAL_MAX = 60 * 60 * 24 * 2;  // 2 days

constexpr uint HISTORY_KEEP_INTERVAL_DEFAULT = 60 * 60 * 24 * 7;  // 7 days
constexpr uint HISTORY_KEEP_INTERVAL_MIN = 60;
constexpr uint HISTORY_KEEP_INTERVAL_MAX = HISTORY_KEEP_INTERVAL_DEFAULT;
}  // namespace

Config::Config(SERVICE_TYPE(component_sys_variable_register) &
                   var_register_service,
               SERVICE_TYPE(component_sys_variable_unregister) &
                   var_unregister_service)
    : var_register_service_(var_register_service),
      var_unregister_service_(var_unregister_service),
      telemetry_root_dir_value_(nullptr) {}

bool Config::init() {
  STR_CHECK_ARG(str) telemetry_root_dir_arg;
  telemetry_root_dir_arg.def_val =
      const_cast<char *>(TELEMETRY_ROOT_DIR_DEFAULT);
  if (var_register_service_.register_variable(
          CURRENT_COMPONENT_NAME_STR, VAR_TELEMETRY_ROOT_DIR,
          PLUGIN_VAR_STR | PLUGIN_VAR_MEMALLOC | PLUGIN_VAR_READONLY,
          "Root path of the telemetry data for all mysqld servers", nullptr,
          nullptr, &telemetry_root_dir_arg, &telemetry_root_dir_value_)) {
    return true;
  }

  INTEGRAL_CHECK_ARG(uint)
  scrape_interval_arg, grace_interval_arg, history_keep_interval_arg;
  scrape_interval_arg.def_val = SCRAPE_INTERVAL_DEFAULT;
  scrape_interval_arg.min_val = SCRAPE_INTERVAL_MIN;
  scrape_interval_arg.max_val = SCRAPE_INTERVAL_MAX;
  scrape_interval_arg.blk_sz = 0;
  if (var_register_service_.register_variable(
          CURRENT_COMPONENT_NAME_STR, VAR_SCRAPE_INTERVAL,
          PLUGIN_VAR_UNSIGNED | PLUGIN_VAR_INT | PLUGIN_VAR_READONLY,
          "Telemetry scrape interval", nullptr, nullptr, &scrape_interval_arg,
          &scrape_interval_value_)) {
    return true;
  }

  grace_interval_arg.def_val = GRACE_INTERVAL_DEFAULT;
  grace_interval_arg.min_val = GRACE_INTERVAL_MIN;
  grace_interval_arg.max_val = GRACE_INTERVAL_MAX;
  grace_interval_arg.blk_sz = 0;
  if (var_register_service_.register_variable(
          CURRENT_COMPONENT_NAME_STR, VAR_GRACE_INTERVAL,
          PLUGIN_VAR_UNSIGNED | PLUGIN_VAR_INT | PLUGIN_VAR_READONLY,
          "Telemetry grace interval", nullptr, nullptr, &grace_interval_arg,
          &grace_interval_value_)) {
    return true;
  }

  history_keep_interval_arg.def_val = HISTORY_KEEP_INTERVAL_DEFAULT;
  history_keep_interval_arg.min_val = HISTORY_KEEP_INTERVAL_MIN;
  history_keep_interval_arg.max_val = HISTORY_KEEP_INTERVAL_MAX;
  history_keep_interval_arg.blk_sz = 0;
  if (var_register_service_.register_variable(
          CURRENT_COMPONENT_NAME_STR, VAR_HISTORY_KEEP_INTERVAL,
          PLUGIN_VAR_UNSIGNED | PLUGIN_VAR_INT | PLUGIN_VAR_READONLY,
          "Telemetry history keep interval", nullptr, nullptr,
          &history_keep_interval_arg, &history_keep_interval_value_)) {
    return true;
  }

  return false;
}

bool Config::deinit() {
  bool res = false;
  if (var_unregister_service_.unregister_variable(CURRENT_COMPONENT_NAME_STR,
                                                  VAR_TELEMETRY_ROOT_DIR)) {
    res = true;
  }
  if (var_unregister_service_.unregister_variable(CURRENT_COMPONENT_NAME_STR,
                                                  VAR_SCRAPE_INTERVAL)) {
    res = true;
  }
  if (var_unregister_service_.unregister_variable(CURRENT_COMPONENT_NAME_STR,
                                                  VAR_GRACE_INTERVAL)) {
    res = true;
  }
  if (var_unregister_service_.unregister_variable(CURRENT_COMPONENT_NAME_STR,
                                                  VAR_HISTORY_KEEP_INTERVAL)) {
    res = true;
  }

  return res;
}

const std::string &Config::telemetry_storage_dir_path() const noexcept {
  assert(telemetry_root_dir_value_);
  // it is read-only value
  static std::string telemetry_root_dir_value_str(telemetry_root_dir_value_);
  return telemetry_root_dir_value_str;
}

std::chrono::seconds Config::scrape_interval() const noexcept {
  return std::chrono::seconds(scrape_interval_value_);
}

std::chrono::seconds Config::grace_interval() const noexcept {
  return std::chrono::seconds(grace_interval_value_);
}

std::chrono::seconds Config::history_keep_interval() const noexcept {
  return std::chrono::seconds(history_keep_interval_value_);
}

std::chrono::seconds Config::unconditional_history_cleanup_interval()
    const noexcept {
  return std::chrono::seconds(HISTORY_KEEP_INTERVAL_MAX);
}
