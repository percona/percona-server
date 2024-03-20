#include <mysql/components/component_implementation.h>
#include <mysql/components/my_service.h>
#include <mysql/components/services/component_sys_var_service.h>
#include <mysql/components/services/log_builtins.h>
#include <mysql/components/services/mysql_command_consumer.h>
#include <mysql/components/services/mysql_command_services.h>
#include <mysql/components/services/mysql_current_thread_reader.h>
#include <mysql/components/services/security_context.h>

#include "common.h"
#include "percona_telemetry_component.h"

namespace {
REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_thd_security_context,
                                thd_security_context_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_thread, command_thread_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_factory, command_factory_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_options, command_options_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_query, command_query_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_query_result,
                                command_query_result_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_field_info,
                                command_field_info_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(mysql_command_error_info,
                                command_error_info_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(registry, registry_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(log_builtins, log_builtins_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(log_builtins_string, log_builtins_string_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(component_sys_variable_register,
                                component_sys_variable_register_srv);
REQUIRES_SERVICE_PLACEHOLDER_AS(component_sys_variable_unregister,
                                component_sys_variable_unregister_srv);

std::unique_ptr<PerconaTelemetryComponent> percona_telemetry_component;

mysql_service_status_t component_init() {
  auto services = std::make_unique<PerconaTelemetryComponent::Services>();
  services->thread_security_context_service = thd_security_context_srv;
  services->command_thread_service = command_thread_srv;
  services->command_factory_service = command_factory_srv;
  services->command_options_service = command_options_srv;
  services->command_query_service = command_query_srv;
  services->command_query_result_service = command_query_result_srv;
  services->command_field_info_service = command_field_info_srv;
  services->command_error_info_service = command_error_info_srv;
  services->log_builtins_service = log_builtins_srv;
  services->log_builtins_string = log_builtins_string_srv;
  services->var_register_service = component_sys_variable_register_srv;
  services->var_unregister_service = component_sys_variable_unregister_srv;

  percona_telemetry_component =
      std::make_unique<PerconaTelemetryComponent>(std::move(services));

  if (percona_telemetry_component->start()) {
    return 1;
  }

  return 0;
}

mysql_service_status_t component_deinit() {
  if (percona_telemetry_component->stop()) {
    return 1;
  }

  percona_telemetry_component.reset();
  return 0;
}

BEGIN_COMPONENT_PROVIDES(CURRENT_COMPONENT_NAME)
END_COMPONENT_PROVIDES();

BEGIN_COMPONENT_REQUIRES(CURRENT_COMPONENT_NAME)
REQUIRES_SERVICE_AS(mysql_thd_security_context, thd_security_context_srv),
    REQUIRES_SERVICE_AS(mysql_command_thread, command_thread_srv),
    REQUIRES_SERVICE_AS(mysql_command_factory, command_factory_srv),
    REQUIRES_SERVICE_AS(mysql_command_options, command_options_srv),
    REQUIRES_SERVICE_AS(mysql_command_query, command_query_srv),
    REQUIRES_SERVICE_AS(mysql_command_query_result, command_query_result_srv),
    REQUIRES_SERVICE_AS(mysql_command_field_info, command_field_info_srv),
    REQUIRES_SERVICE_AS(mysql_command_error_info, command_error_info_srv),
    REQUIRES_SERVICE_AS(registry, registry_srv),
    REQUIRES_SERVICE_AS(log_builtins, log_builtins_srv),
    REQUIRES_SERVICE_AS(log_builtins_string, log_builtins_string_srv),
    REQUIRES_SERVICE_AS(component_sys_variable_register,
                        component_sys_variable_register_srv),
    REQUIRES_SERVICE_AS(component_sys_variable_unregister,
                        component_sys_variable_unregister_srv),
    END_COMPONENT_REQUIRES();

BEGIN_COMPONENT_METADATA(CURRENT_COMPONENT_NAME)
METADATA("mysql.author", "Percona Corporation"),
    METADATA("mysql.license", "GPL"), METADATA("mysql.version", "1"),
    END_COMPONENT_METADATA();

DECLARE_COMPONENT(CURRENT_COMPONENT_NAME, CURRENT_COMPONENT_NAME_STR)
component_init, component_deinit END_DECLARE_COMPONENT();

}  // namespace

DECLARE_LIBRARY_COMPONENTS &COMPONENT_REF(CURRENT_COMPONENT_NAME)
    END_DECLARE_LIBRARY_COMPONENTS
