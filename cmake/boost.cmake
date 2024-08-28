# Copyright (c) 2014, 2024, Oracle and/or its affiliates.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is designed to work with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have either included with
# the program or referenced in the documentation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

SET(BOOST_PACKAGE_NAME "boost_1_84_0")

# Always use the bundled version.
SET(BOOST_SOURCE_DIR ${CMAKE_SOURCE_DIR}/extra/boost)

# Contains all header files we need.
# (All the directories that contain at least one needed file).
SET(BOOST_INCLUDE_DIR ${BOOST_SOURCE_DIR}/${BOOST_PACKAGE_NAME})

# We have a limited set of patches/bugfixes here:
SET(BOOST_PATCHES_DIR
  "${CMAKE_SOURCE_DIR}/include/${BOOST_PACKAGE_NAME}/patches")

# Bundled boost excludes unnecessary sources. Some of them are required by Percona Server specific code.
# Each time bundled boost gets updated we must update those Percona Server specific sources as well.
# Actual list of Percona Server specific boost sources is the following:
#   boost/dynamic_bitset.hpp
#   boost/dynamic_bitset_fwd.hpp
#   boost/dynamic_bitset/*
#   boost/io_fwd.hpp
#   boost/io/*
#   boost/random/*
#   boost/tti/*
#   boost/uuid/*
SET(EXPECTED_BOOST_VERSION 108400)
IF(EXISTS "${BOOST_INCLUDE_DIR}/boost/version.hpp")
  FILE(STRINGS "${BOOST_INCLUDE_DIR}/boost/version.hpp" BOOST_VERSION_HPP_CONTENTS REGEX "#define BOOST_VERSION ")
  IF("${BOOST_VERSION_HPP_CONTENTS}" MATCHES "#define BOOST_VERSION ([0-9]+)")
    IF(NOT ${CMAKE_MATCH_1} EQUAL ${EXPECTED_BOOST_VERSION})
      MESSAGE(FATAL_ERROR "Actual BOOST_VERSION doesn't match EXPECTED_BOOST_VERSION. \
                           Make sure boost headers required by Percona Server are updated \
                           and update EXPECTED_BOOST_VERSION.")
    ENDIF()
  ELSE()
    MESSAGE(FATAL_ERROR "Cannot find BOOST_VERSION in version.hpp")
  ENDIF()
ELSE()
  MESSAGE(FATAL_ERROR "Cannot find boost version.hpp")
ENDIF()

ADD_LIBRARY(boost INTERFACE)
ADD_LIBRARY(extra::boost ALIAS boost)

TARGET_INCLUDE_DIRECTORIES(boost SYSTEM BEFORE INTERFACE
  ${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR})

IF(NOT WIN32)
  # See boost/container_hash/hash.hpp
  # We pretend that the compiler is pre-c++98, in order to hide the
  # usage of std::unary_function<..> (which was removed in C++17)
  # For windows: see boost/config/stdlib/dinkumware.hpp
  TARGET_COMPILE_DEFINITIONS(boost INTERFACE BOOST_NO_CXX98_FUNCTION_BASE)
ENDIF()

MESSAGE(STATUS "BOOST_PATCHES_DIR ${BOOST_PATCHES_DIR}")
MESSAGE(STATUS "BOOST_INCLUDE_DIR ${BOOST_INCLUDE_DIR}")

IF(NOT WIN32)
  FILE(GLOB_RECURSE BOOST_PATCHES_LIST
    RELATIVE ${BOOST_PATCHES_DIR}
    ${BOOST_PATCHES_DIR}/*.hpp
    )

  SET(DIFF_COMMAND_LIST "#! /bin/bash")
  FOREACH(PATCHED_FILE ${BOOST_PATCHES_LIST})
    SET(ORIGINAL_FILE_PATH "${BOOST_INCLUDE_DIR}/${PATCHED_FILE}")
    SET(PATCHED_FILE_PATH "${BOOST_PATCHES_DIR}/${PATCHED_FILE}")
    LIST(APPEND DIFF_COMMAND_LIST
      "diff -u ${ORIGINAL_FILE_PATH} ${PATCHED_FILE_PATH}")
  ENDFOREACH()
  # Add true, to get zero exit status.
  LIST(APPEND DIFF_COMMAND_LIST "true")

  STRING(REPLACE ";" "\n" DIFF_COMMAND_LINES "${DIFF_COMMAND_LIST}")

  FILE(GENERATE
    OUTPUT ${CMAKE_BINARY_DIR}/boost_patch_diffs
    CONTENT "${DIFF_COMMAND_LINES}"
    )

  ADD_CUSTOM_TARGET(show_boost_patches
    COMMAND bash ${CMAKE_BINARY_DIR}/boost_patch_diffs
    DEPENDS ${CMAKE_BINARY_DIR}/boost_patch_diffs
    )
ENDIF()
