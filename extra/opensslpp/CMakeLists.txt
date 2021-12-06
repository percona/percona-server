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

# list(TRANSFORM ... PREPEND ...) emulation for older CMake
function(list_transform_prepend var prefix)
    set(temp "")
    foreach(f ${${var}})
        list(APPEND temp "${prefix}${f}")
    endforeach()
    set(${var} "${temp}" PARENT_SCOPE)
endfunction()

set(
  PUBLIC_HEADER_LIST
  accessor_fwd.hpp
  big_number.hpp
  big_number_fwd.hpp
  core_error.hpp
  core_error_fwd.hpp
  dh_compute_operations.hpp
  dh_key.hpp
  dh_key_fwd.hpp
  dh_padding.hpp
  dh_padding_fwd.hpp
  digest_context.hpp
  digest_context_fwd.hpp
  digest_operations.hpp
  dsa_key.hpp
  dsa_key_fwd.hpp
  dsa_sign_verify_operations.hpp
  key_generation_cancellation_callback_fwd.hpp
  operation_cancelled_error.hpp
  operation_cancelled_error_fwd.hpp
  rsa_encrypt_decrypt_operations.hpp
  rsa_key.hpp
  rsa_key_fwd.hpp
  rsa_padding.hpp
  rsa_padding_fwd.hpp
  rsa_sign_verify_operations.hpp
)
list_transform_prepend(PUBLIC_HEADER_LIST "${PUBLIC_INCLUDE_DIR_EX}/")
set(
  PRIVATE_HEADER_LIST
  accessor.hpp
  big_number_accessor.hpp
  bio.hpp
  bio_accessor.hpp
  bio_fwd.hpp
  dh_key_accessor.hpp
  digest_context_accessor.hpp
  dsa_key_accessor.hpp
  key_generation_cancellation_context.hpp
  key_generation_cancellation_context_accessor.hpp
  key_generation_cancellation_context_fwd.hpp
  rsa_key_accessor.hpp
  rsa_padding_conversions.hpp
  typed_accessor.hpp
)
list_transform_prepend(PRIVATE_HEADER_LIST "${PRIVATE_INCLUDE_DIR_EX}/")
set(
  SOURCE_LIST
  big_number.cpp
  bio.cpp
  core_error.cpp
  dh_compute_operations.cpp
  dh_key.cpp
  digest_context.cpp
  digest_operations.cpp
  dsa_key.cpp
  dsa_sign_verify_operations.cpp
  key_generation_cancellation_context.cpp
  rsa_encrypt_decrypt_operations.cpp
  rsa_key.cpp
  rsa_sign_verify_operations.cpp
)
list_transform_prepend(SOURCE_LIST "${SRC_DIR_EX}/")

add_library(${PROJECT_NAME} STATIC ${SOURCE_LIST} ${PUBLIC_HEADER_LIST} ${PRIVATE_HEADER_LIST})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC ${${PROJECT_NAME}_SOURCE_DIR}/include
  PRIVATE ${${PROJECT_NAME}_SOURCE_DIR}/include_private
)

target_compile_definitions(${PROJECT_NAME} PRIVATE OPENSSL_SUPPRESS_DEPRECATED)

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
