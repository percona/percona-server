# Copyright (c) 2023, 2024 Percona LLC and/or its affiliates. All rights
# reserved.
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

OPTION(WITH_JS_LANG "Build JS stored routine language component" OFF)

IF(WITH_JS_LANG)
  MESSAGE(STATUS "Building JS stored routine language component")
  IF(NOT DEFINED V8_INCLUDE_DIR OR NOT DEFINED V8_LIB_DIR)
    MESSAGE(FATAL_ERROR "No path to V8 headers and libraries. Please set V8_INCLUDE_DIR and V8_LIB_DIR.")
  ENDIF()
ELSE()
  MESSAGE(STATUS "Not building JS stored routine language component")
  RETURN()
ENDIF()

DISABLE_MISSING_PROFILE_WARNING()

MYSQL_ADD_COMPONENT(js_lang
  component.cc
  js_lang_core.cc
  js_lang_param.cc
  LINK_LIBRARIES v8_monolith
  MODULE_ONLY
)

target_include_directories(component_js_lang SYSTEM PRIVATE ${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR} ${V8_INCLUDE_DIR})
target_link_directories(component_js_lang SYSTEM PRIVATE ${V8_LIB_DIR})

# Uncomment below if you are playing with debug build of V8 library.
# TODO: Add check which will determine if we need to use this option automagically?
# target_compile_definitions(component_js_lang PRIVATE V8_ENABLE_CHECKS)
