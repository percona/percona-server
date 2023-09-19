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

#ifndef AUDIT_LOG_FILTER_COMPONENT_REGISTRY_SERVICE_H_INCLUDED
#define AUDIT_LOG_FILTER_COMPONENT_REGISTRY_SERVICE_H_INCLUDED

#include <mysql/service_plugin_registry.h>

#include <functional>
#include <memory>

namespace audit_log_filter {

using comp_registry_srv_t = SERVICE_TYPE(registry);
using comp_registry_srv_deleter_t = std::function<void(comp_registry_srv_t *)>;
using comp_registry_srv_container_t =
    std::unique_ptr<comp_registry_srv_t, comp_registry_srv_deleter_t>;

comp_registry_srv_container_t get_component_registry_service();

}  // namespace audit_log_filter

#endif  // AUDIT_LOG_FILTER_COMPONENT_REGISTRY_SERVICE_H_INCLUDED
