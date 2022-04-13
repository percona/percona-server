/* Copyright (c) 2022 Percona LLC and/or its affiliates. All rights reserved.

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

#include "plugin/audit_log_filter/log_writer_strategy.h"

#include "log_writer_strategy/async.h"
#include "log_writer_strategy/perf.h"
#include "log_writer_strategy/semisync.h"
#include "log_writer_strategy/sync.h"

namespace audit_log_filter {

template <AuditLogStrategyType StrategyType>
std::unique_ptr<FileWriterStrategyBase> create_helper() {
  return std::make_unique<FileWriterStrategy<StrategyType>>();
}

std::unique_ptr<FileWriterStrategyBase> get_log_writer_strategy(
    AuditLogStrategyType strategy_type) {
  using CreateFunc = std::unique_ptr<FileWriterStrategyBase> (*)();
  static const CreateFunc
      funcs[static_cast<int>(AuditLogStrategyType::StrategiesCount)] = {
          create_helper<AuditLogStrategyType::Asynchronous>,
          create_helper<AuditLogStrategyType::Performance>,
          create_helper<AuditLogStrategyType::Semisynchronous>,
          create_helper<AuditLogStrategyType::Synchronous>};
  return (*funcs[static_cast<int>(strategy_type)])();
}

}  // namespace audit_log_filter
