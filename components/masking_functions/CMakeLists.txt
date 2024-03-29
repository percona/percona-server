# Copyright (c) 2018, 2019, Francisco Miguel Biete Banon. All rights reserved.
# Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.
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

option(WITH_COMPONENT_MASKING_FUNCTIONS "Build Masking Functions component" ON)

if(WITH_COMPONENT_MASKING_FUNCTIONS)
  message(STATUS "Building Masking Functions component")
else()
  message(STATUS "Not building Masking Functions component")
  return()
endif()

set(DATAMASKING_SOURCES
  src/component.cpp

  src/masking_functions/charset_string.cpp
  src/masking_functions/charset_string_operations.cpp
  src/masking_functions/dictionary_container.cpp
  src/masking_functions/query_builder.cpp
  src/masking_functions/query_cache.cpp
  src/masking_functions/random_string_generators.cpp
  src/masking_functions/registration_routines.cpp
  src/masking_functions/sql_context.cpp
  src/masking_functions/sql_escape_functions.cpp
  src/masking_functions/sys_vars.cpp

  include/masking_functions/charset_string_fwd.hpp
  include/masking_functions/charset_string.hpp
  include/masking_functions/charset_string_operations.hpp
  include/masking_functions/command_service_tuple_fwd.hpp
  include/masking_functions/command_service_tuple.hpp
  include/masking_functions/dictionary_container.hpp
  include/masking_functions/primitive_singleton.hpp
  include/masking_functions/query_builder.hpp
  include/masking_functions/query_cache.hpp
  include/masking_functions/random_string_generators.hpp
  include/masking_functions/registration_routines.hpp
  include/masking_functions/sql_context.hpp
  include/masking_functions/sql_escape_functions.hpp
  include/masking_functions/string_service_tuple_fwd.hpp
  include/masking_functions/string_service_tuple.hpp
  include/masking_functions/sys_vars.hpp
)

### Configuration ###

DISABLE_MISSING_PROFILE_WARNING()

MYSQL_ADD_COMPONENT(masking_functions
  ${DATAMASKING_SOURCES}
  MODULE_ONLY
)

target_compile_definitions(component_masking_functions PRIVATE LOG_COMPONENT_TAG="component_masking_functions")
target_include_directories(component_masking_functions PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(component_masking_functions SYSTEM PRIVATE ${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR})

if(APPLE)
  set_target_properties(component_masking_functions PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
endif()
