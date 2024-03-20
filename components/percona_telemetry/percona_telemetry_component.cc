#include "percona_telemetry_component.h"
#include "config.h"
#include "data_provider.h"
#include "logger.h"
#include "storage.h"
#include "worker.h"

PerconaTelemetryComponent::PerconaTelemetryComponent(
    std::unique_ptr<Services> services)
    : services_(std::move(services)) {}

bool PerconaTelemetryComponent::start() {
  logger_ = std::make_unique<Logger>(*(services_->log_builtins_service),
                                     *(services_->log_builtins_string),
                                     INFORMATION_LEVEL);

  config_ = std::make_unique<Config>(*(services_->var_register_service),
                                     *(services_->var_unregister_service));
  if (config_->init()) {
    return true;
  }

  storage_ = std::make_unique<Storage>(*config_, *logger_);

  data_provider_ = std::make_unique<DataProvider>(
      *(services_->command_factory_service),
      *(services_->command_options_service),
      *(services_->command_query_service),
      *(services_->command_query_result_service),
      *(services_->command_field_info_service),
      *(services_->command_error_info_service),
      *(services_->command_thread_service), *logger_);

  worker_ =
      std::make_unique<Worker>(*config_, *storage_, *data_provider_, *logger_);

  return worker_->start();
}

bool PerconaTelemetryComponent::stop() {
  if (worker_->stop()) {
    logger_->warning("Unable to stop PerconaTelemetryComponent.");
    return true;
  }

  worker_.reset();
  data_provider_.reset();
  storage_.reset();
  config_->deinit();
  config_.reset();

  return false;
}
