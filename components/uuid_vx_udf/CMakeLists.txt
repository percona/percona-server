# Copyright (c) 2024 Percona LLC and/or its affiliates. All rights reserved.

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; version 2 of
# the License.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

option(WITH_UUID_VX_UDF "Build UUID_VX UDF plugin" ON)

if(WITH_UUID_VX_UDF)
  message(STATUS "Building UUID_VX UDF component")
else()
  message(STATUS "Not building UUID_VX UDF component")
  return()
endif()

# We are not interesting in profiling tests.
DISABLE_MISSING_PROFILE_WARNING()

MYSQL_ADD_COMPONENT(uuid_vx_udf
  uuid_vx_udf_component.cc
  MODULE_ONLY
)

target_include_directories(component_uuid_vx_udf SYSTEM PRIVATE ${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR})

if(NOT APPLE)
  target_link_libraries(component_uuid_vx_udf PRIVATE atomic)
endif()
