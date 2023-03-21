RETURN()  # Disable TokuDB

# Copyright (c) 2017, Percona and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

# by default TokuDB is attempted to build, if environment is unsupported,
# TokuDB is gently skipped
IF (NOT DEFINED WITH_TOKUDB)
  IF (WITHOUT_TOKUDB OR WITHOUT_TOKUDB_STORAGE_ENGINE)
    SET (WITH_TOKUDB 0)
  ELSE ()
    SET (WITH_TOKUDB 1)
  ENDIF ()
ENDIF ()

IF (NOT WITH_TOKUDB)
  MESSAGE (STATUS "Not building with TokuDB storage engine, you must specify -DWITH_TOKUDB=1|ON|FORCE to cmake in order to build with TokuDB.")
  RETURN ()
ELSE ()
  MESSAGE (STATUS "Building with TokuDB storage engine = " ${WITH_TOKUDB})
  IF (WITH_TOKUDB STREQUAL "FORCE")
    SET (TOKUDB_STATUS_MODE FATAL_ERROR)
  ELSE ()
    SET (TOKUDB_STATUS_MODE WARNING)
  ENDIF ()
ENDIF ()

# check platform support, no 32 bit
IF (NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  MESSAGE (${TOKUDB_STATUS_MODE} "x86_64 is only platform supported. ${CMAKE_SYSTEM_PROCESSOR} found. Not building TokuDB")
  RETURN ()
ENDIF ()

CHECK_CXX_SOURCE_COMPILES (
"
struct a {int b; int c; };
struct a d = { .b=1, .c=2 };
int main() { return 0; }
" TOKUDB_OK)

IF (NOT TOKUDB_OK)
  MESSAGE (${TOKUDB_STATUS_MODE} "Test compile failed. Not building TokuDB")
  RETURN ()
ENDIF ()

SET(BUILD_TESTING OFF CACHE BOOL "")
SET(USE_VALGRIND OFF CACHE BOOL "")
SET(TOKU_DEBUG_PARANOID OFF CACHE BOOL "")

MY_CHECK_CXX_COMPILER_FLAG("-Wcast-qual" HAVE_CAST_EQUAL)
IF(HAVE_CAST_EQUAL)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-cast-qual")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-cast-qual")
ENDIF()

# Enable TokuDB's TOKUDB_DEBUG in debug builds
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DTOKUDB_DEBUG")

IF(NOT DEFINED TOKUDB_VERSION)
    IF(DEFINED ENV{TOKUDB_VERSION})
        SET(TOKUDB_VERSION $ENV{TOKUDB_VERSION})
    ENDIF()
ENDIF()
IF(DEFINED TOKUDB_VERSION)
    ADD_DEFINITIONS("-DTOKUDB_VERSION=${TOKUDB_VERSION}")
    IF (${TOKUDB_VERSION} MATCHES "^tokudb-([0-9]+)\\.([0-9]+)\\.([0-9]+.*)")
        ADD_DEFINITIONS("-DTOKUDB_VERSION_MAJOR=${CMAKE_MATCH_1}")
        ADD_DEFINITIONS("-DTOKUDB_VERSION_MINOR=${CMAKE_MATCH_2}")
        ADD_DEFINITIONS("-DTOKUDB_VERSION_PATCH=${CMAKE_MATCH_3}")
    ENDIF()
ENDIF()

IF(DEFINED TOKUDB_NOPATCH_CONFIG)
    ADD_DEFINITIONS("-DTOKUDB_NOPATCH_CONFIG=${TOKUDB_NOPATCH_CONFIG}")
ENDIF()



# Suppress warnings for all compilers
append_cflags_if_supported(-Wno-missing-field-initializers -Wno-suggest-override)

# "cmake/maintainer.cmake" sets "-Wvla", "-Wmissing-include-dirs" and
# "-Wextra-semi" which causes warnings with PerconaFT
remove_compile_flags(-Wvla -Wmissing-include-dirs -Wextra-semi)

# Suppress warnings for all clang versions
IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  append_cflags_if_supported(-Wno-deprecated-dynamic-exception-spec -Wno-inconsistent-missing-override)
  remove_compile_flags(-Wheader-hygiene -Wconditional-uninitialized -Winconsistent-missing-destructor-override)
ENDIF()

# Suppress warnings for clang-8 or newer
IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
  remove_compile_flags(-Wshadow-field)
ENDIF()


IF (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/PerconaFT/")
    SET(TOKU_FT_DIR_NAME "PerconaFT")
ELSE ()
    MESSAGE(FATAL_ERROR "Could not find PerconaFT sources.")
ENDIF ()

IF (WITH_VALGRIND)
    SET(USE_VALGRIND "ON")
ENDIF ()

ADD_SUBDIRECTORY(${TOKU_FT_DIR_NAME})
INCLUDE_DIRECTORIES(${TOKU_FT_DIR_NAME})
INCLUDE_DIRECTORIES(${TOKU_FT_DIR_NAME}/portability)
INCLUDE_DIRECTORIES(${TOKU_FT_DIR_NAME}/util)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/${TOKU_FT_DIR_NAME})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/${TOKU_FT_DIR_NAME}/buildheader)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/${TOKU_FT_DIR_NAME}/portability)


SET(TOKUDB_PLUGIN_DYNAMIC "ha_tokudb")
SET(TOKUDB_SOURCES
    ha_tokudb.cc
    tokudb_background.cc
    tokudb_information_schema.cc
    tokudb_sysvars.cc
    tokudb_thread.cc
    tokudb_dir_cmd.cc)
MYSQL_ADD_PLUGIN(tokudb ${TOKUDB_SOURCES} STORAGE_ENGINE MODULE_ONLY
    LINK_LIBRARIES tokufractaltree_static tokuportability_static ${ZLIB_LIBRARY} stdc++)
SET(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} -flto -fuse-linker-plugin")
SET(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO} -flto -fuse-linker-plugin")
