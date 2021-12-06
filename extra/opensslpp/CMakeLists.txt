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

cmake_minimum_required (VERSION 3.8)

project(
  OpenSSLPP
  VERSION 1.0.0
  DESCRIPTION "C++ wrappers for OpenSSL"
  LANGUAGES CXX
)

if(NOT DEFINED CMAKE_CXX_VISIBILITY_PRESET AND NOT DEFINED CMAKE_VISIBILITY_INLINES_HIDDEN)
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
  set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
endif()

set(PUBLIC_INCLUDE_DIR include)
set(PRIVATE_INCLUDE_DIR include_private)
set(SRC_DIR src)

string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LC)
set(NAMESPACE_SUBDIR ${PROJECT_NAME_LC})

set(PUBLIC_INCLUDE_DIR_EX ${PUBLIC_INCLUDE_DIR}/${NAMESPACE_SUBDIR})
set(PRIVATE_INCLUDE_DIR_EX ${PRIVATE_INCLUDE_DIR}/${NAMESPACE_SUBDIR})
set(SRC_DIR_EX ${SRC_DIR}/${NAMESPACE_SUBDIR})

set(
  PUBLIC_HEADER_LIST
  accessor_fwd.hpp
  big_number.hpp
  big_number_fwd.hpp
  core_error.hpp
  core_error_fwd.hpp
  rsa_key.hpp
  rsa_key_fwd.hpp
  rsa_padding.hpp
  rsa_padding_fwd.hpp
)
list(TRANSFORM PUBLIC_HEADER_LIST PREPEND "${PUBLIC_INCLUDE_DIR_EX}/")
set(
  PRIVATE_HEADER_LIST
  accessor.hpp
  big_number_accessor.hpp
  bio.hpp
  bio_accessor.hpp
  bio_fwd.hpp
  core_allocated_buffer.hpp
  rsa_key_accessor.hpp
  typed_accessor.hpp
)
list(TRANSFORM PRIVATE_HEADER_LIST PREPEND "${PRIVATE_INCLUDE_DIR_EX}/")
set(
  SOURCE_LIST
  big_number.cpp
  bio.cpp
  core_error.cpp
  core_allocated_buffer.cpp
  rsa_key.cpp
)
list(TRANSFORM SOURCE_LIST PREPEND "${SRC_DIR_EX}/")

find_package(OpenSSL REQUIRED)

add_library(${PROJECT_NAME} STATIC ${SOURCE_LIST} ${PUBLIC_HEADER_LIST} ${PRIVATE_HEADER_LIST})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC ${${PROJECT_NAME}_SOURCE_DIR}/include
  PRIVATE ${${PROJECT_NAME}_SOURCE_DIR}/include_private
)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    VERSION ${${PROJECT_NAME}_VERSION}
    SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::Crypto)
