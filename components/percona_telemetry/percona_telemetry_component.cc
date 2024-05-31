/* Copyright (c) 2024 Percona LLC and/or its affiliates. All rights reserved.

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
