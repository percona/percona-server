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

#ifndef AUDIT_LOG_FILTER_LOG_WRITER_STRATEGY_H_INCLUDED
#define AUDIT_LOG_FILTER_LOG_WRITER_STRATEGY_H_INCLUDED

#include "log_writer_strategy/base.h"

#include <memory>

namespace audit_log_filter {

using namespace log_writer_strategy;

/**
 * @brief Get an instance of log file writer strategy of specified type.
 *
 * @param [in] strategy_type File writer strategy type,
 *                           @see log_writer_strategy::AuditLogStrategyType
 * @return An instance of log file writer strategy
 */
std::unique_ptr<FileWriterStrategyBase> get_log_writer_strategy(
    AuditLogStrategyType strategy_type);

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_LOG_WRITER_STRATEGY_H_INCLUDED
