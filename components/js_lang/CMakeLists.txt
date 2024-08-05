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
ELSE()
  MESSAGE(STATUS "Not building JS stored routine language component")
  RETURN()
ENDIF()

# Prepare for using Google V8 as ext::v8 interface library.
INCLUDE(v8)
MYSQL_CHECK_V8()

DISABLE_MISSING_PROFILE_WARNING()

MYSQL_ADD_COMPONENT(js_lang
  component.cc
  js_lang_core.cc
  js_lang_param.cc
  LINK_LIBRARIES ext::v8
  MODULE_ONLY
)
