# Copyright (c) 2022 Percona LLC and/or its affiliates. All rights reserved.

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

if(WITH_ENCRYPTION_UDF)
  message(STATUS "Building Encryption UDF component")
else()
  message(STATUS "Not building Encryption UDF component")
  return()
endif()

# We are not interesting in profiling tests.
DISABLE_MISSING_PROFILE_WARNING()

MYSQL_ADD_COMPONENT(encryption_udf
  encryption_udf_component.cc
  server_helpers.h
  server_helpers.cc
  LINK_LIBRARIES OpenSSLPP::OpenSSLPP ext::zlib
  MODULE_ONLY
)

# By default gcc / clang toolchain, when creating a shared library,
# marks all symbols as 'exported' - __attribute__((visibility("default"))).
# This includes not only the symbols declared explicitly in the library
# source code but also those linked from the dependent static libraries
# as well as symbols instantiated from the standard library templated code.
# This is not a desired behavior for UDF shared objects, MySQL plugins /
# components that rely on dynamic symbol lookups ('dlsym()' calls).
# The exported symbols namespace may be extremely polluted in this case,
# which may lead to slower symbols lookups as well as raising security
# concerns of being able to load symbols not intended originally to be
# loadable.
# The solution is to switch to __attribute__((visibility("hidden"))) by
# default and mark 'exported' only those sybbols that are marked with
# __attribute__((visibility("default"))) explicitly.
# Here, for gcc for instance, CXX_VISIBILITY_PRESET CMake target property
# will be translated into '-fvisibility=hidden' compiler option.
# VISIBILITY_INLINES_HIDDEN as an addition to the previous one will also
# add '-fvisibility-inlines-hidden' (helps a lot with STL code).
set_target_properties(component_encryption_udf PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN YES
)

target_include_directories(component_encryption_udf SYSTEM PRIVATE ${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR})
