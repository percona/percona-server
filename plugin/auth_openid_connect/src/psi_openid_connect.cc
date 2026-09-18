/*
(C) 2026 Percona LLC and/or its affiliates

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#include "psi_openid_connect.h"

#include <mysql/psi/mysql_memory.h>
#include <template_utils.h>

namespace Psi_openid_connect {

PSI_memory_key config_memory_key;

static constexpr auto config_memory{"config_memory"};

static PSI_memory_info all_memory[] = {{&config_memory_key, config_memory, 0,
                                        PSI_VOLATILITY_UNKNOWN,
                                        PSI_DOCUMENT_ME}};

void init() {
  static constexpr auto category{"auth_openid_connect"};

  int count = array_elements(all_memory);
  mysql_memory_register(category, all_memory, count);
}

}  // namespace Psi_openid_connect
