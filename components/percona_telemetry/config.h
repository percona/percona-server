#ifndef PERCONA_TELEMETRY_CONFIG_H
#define PERCONA_TELEMETRY_CONFIG_H

#include <chrono>
#include <memory>
#include <string>

#include <mysql/components/component_implementation.h>
#include <mysql/components/services/component_sys_var_service.h>

class Config {
 public:
  Config(SERVICE_TYPE(component_sys_variable_register) & var_register_service,
         SERVICE_TYPE(component_sys_variable_unregister) &
             var_unregister_service);
  ~Config() = default;

  Config(const Config &rhs) = delete;
  Config(Config &&rhs) = delete;
  Config &operator=(const Config &rhs) = delete;
  Config &operator=(Config &&rhs) = delete;

  bool init();
  bool deinit();

  const std::string &telemetry_storage_dir_path() const noexcept;
  std::chrono::seconds scrape_interval() const noexcept;
  std::chrono::seconds grace_interval() const noexcept;
  std::chrono::seconds history_keep_interval() const noexcept;
  std::chrono::seconds unconditional_history_cleanup_interval() const noexcept;

 private:
  SERVICE_TYPE(component_sys_variable_register) & var_register_service_;
  SERVICE_TYPE(component_sys_variable_unregister) & var_unregister_service_;

  const char *telemetry_root_dir_value_;
  uint scrape_interval_value_;
  uint grace_interval_value_;
  uint history_keep_interval_value_;
};

#endif /* PERCONA_TELEMETRY_CONFIG_H */
