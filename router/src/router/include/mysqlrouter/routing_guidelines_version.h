/*
  Copyright (c) 2024, Oracle and/or its affiliates.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is also distributed with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have included with MySQL.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _MYSQLROUTER_ROUTING_GUIDELINES_VERSION_INCLUDED_
#define _MYSQLROUTER_ROUTING_GUIDELINES_VERSION_INCLUDED_

#include "mysqlrouter/router_utils_export.h"
#include "mysqlrouter/version_base.h"

#include <array>
#include <string>

namespace mysqlrouter {

struct RoutingGuidelinesVersion : public VersionBase {};

constexpr RoutingGuidelinesVersion kBaseRoutingGuidelines{1, 0};

// New versions supported should be put at the end.
[[maybe_unused]] constexpr std::array kSupportedRoutingGuidelinesVersions{
    kBaseRoutingGuidelines,  // Initial routing guidelines version
};

std::string ROUTER_UTILS_EXPORT
to_string(const RoutingGuidelinesVersion &version);

RoutingGuidelinesVersion ROUTER_UTILS_EXPORT
get_routing_guidelines_supported_version();

RoutingGuidelinesVersion ROUTER_UTILS_EXPORT
routing_guidelines_version_from_string(const std::string &version_string);

bool ROUTER_UTILS_EXPORT routing_guidelines_version_is_compatible(
    const mysqlrouter::RoutingGuidelinesVersion &required,
    const mysqlrouter::RoutingGuidelinesVersion &available);

}  // namespace mysqlrouter

#endif
