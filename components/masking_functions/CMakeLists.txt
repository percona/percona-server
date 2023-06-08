# Copyright (c) 2018, 2019, Francisco Miguel Biete Banon. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

SET(DATAMASKING_SOURCES
  src/component.cc
  src/udf/udf_registration.cc
  src/udf/udf_utils.cc
  src/udf/udf_utils_string.cc
  src/udf/udf_utils_sql.cc
)

### Configuration ###

DISABLE_MISSING_PROFILE_WARNING()

if(NOT DEFINED CMAKE_CXX_VISIBILITY_PRESET AND NOT DEFINED CMAKE_VISIBILITY_INLINES_HIDDEN)
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
  set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
endif()

MYSQL_ADD_COMPONENT(masking_functions
  ${DATAMASKING_SOURCES}
  MODULE_ONLY
)

target_include_directories(component_masking_functions SYSTEM PRIVATE ${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR})
