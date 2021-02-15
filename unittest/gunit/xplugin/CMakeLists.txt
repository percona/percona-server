# Copyright (c) 2015, 2020, Oracle and/or its affiliates.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

IF(NOT WITH_MYSQLX)
  RETURN()
ENDIF()

INCLUDE(CTest)

OPTION(MYSQLX_ADDITIONAL_TESTS_ENABLE "MYSQLX - enable additional tests" OFF)

SET(MYSQLX_PROJECT_DIR "${PROJECT_SOURCE_DIR}/plugin/x")

GET_DIRECTORY_PROPERTY(MYSQLX_CLIENT_INCLUDE_DIR
  DIRECTORY ${MYSQLX_PROJECT_DIR}
  DEFINITION MYSQLX_CLIENT_INCLUDE_DIR
)

GET_DIRECTORY_PROPERTY(MYSQLX_GENERATE_DIR
  DIRECTORY ${MYSQLX_PROJECT_DIR}
  DEFINITION MYSQLX_GENERATE_DIR
)

INCLUDE_DIRECTORIES(
  ${MYSQLX_GENERATE_DIR}
  ${MYSQLX_CLIENT_INCLUDE_DIR}
)

# Protobuf 3 will generate #if PROTOBUF_INLINE_NOT_IN_HEADERS
# which may fail with -Werror=undef
ADD_DEFINITIONS(-DPROTOBUF_INLINE_NOT_IN_HEADERS=0 -DXPLUGIN_DISABLE_LOG)
# Protobuf 3.7.0 will generate #if 3007000 < PROTOBUF_MIN_PROTOC_VERSION
IF(PROTOBUF_VERSION_NUMBER MATCHES "3007000")
  ADD_DEFINITIONS(-DPROTOBUF_MIN_PROTOC_VERSION=2006000)
ENDIF()

ADD_SUBDIRECTORY(xpl)
ADD_SUBDIRECTORY(xcl)

<<<<<<< HEAD
IF(MY_COMPILER_IS_CLANG)
  STRING_APPEND(CMAKE_CXX_FLAGS " -Wno-deprecated")
ENDIF()

# assertion failed in function assert_with_dumps() @ ssa.c:621 when using -xO3
# Segfault in the unit test, so lower optimization level there as well
IF(MY_COMPILER_IS_SUNPRO)
  ADD_COMPILE_FLAGS(
    ${CMAKE_CURRENT_SOURCE_DIR}/xpl/callback_command_delegate_t.cc
    COMPILE_FLAGS "-xO2")
ENDIF()

TARGET_LINK_LIBRARIES(${XPL_UNIT_TESTS}
  ${GCOV_LDFLAGS}
  ${MYSQLX_CLIENT_LIB}
  mysqlx
  perconaserverclient
  gtest
  gmock
)


# --== XClient unit tests (always full version) ==--

GET_DIRECTORY_PROPERTY(MYSQLX_CLIENT_FULL_LIB
  DIRECTORY ${MYSQLX_PROJECT_DIR}
  DEFINITION MYSQLX_CLIENT_FULL_LIB
)

GET_DIRECTORY_PROPERTY(MYSQLX_PROTOCOL_FULL_LIB
  DIRECTORY ${MYSQLX_PROJECT_DIR}
  DEFINITION MYSQLX_PROTOCOL_FULL_LIB
)

SET(XCL_UNIT_TESTS xclient_unit_tests)

SET(XCL_SRC
  test_main.cc
  xcl/auth_chaining_t.cc
  xcl/connection_general_t.cc
  xcl/protocol_auth_t.cc
  xcl/protocol_execute_t.cc
  xcl/protocol_global_error_t.cc
  xcl/protocol_notices_t.cc
  xcl/protocol_send_recv_t.cc
  xcl/query_t.cc
  xcl/session_capability_t.cc
  xcl/session_connect_t.cc
  xcl/session_execute_t.cc
  xcl/session_general_t.cc
  xcl/session_options_t.cc
  xcl/session_negotiation_t.cc
  xcl/sha256_scramble_t.cc
  xcl/ssl_config_t.cc
  xcl/xpriority_list_t.cc
)
# This compiler fails to handle some of the mock functions involved.
IF(MY_COMPILER_IS_SUNPRO)
  LIST(REMOVE_ITEM XCL_SRC
    xcl/auth_chaining_t.cc
    xcl/session_capability_t.cc
    xcl/session_connect_t.cc
    xcl/session_execute_t.cc
    xcl/session_general_t.cc
    xcl/session_options_t.cc
    xcl/session_negotiation_t.cc
    )
ENDIF()

ADD_COMPILE_DEFINITIONS(${XCL_SRC}
  COMPILE_DEFINITIONS USE_MYSQLX_FULL_PROTO
)

MYSQL_ADD_EXECUTABLE(${XCL_UNIT_TESTS}
  ${XCL_SRC}
  ADD_TEST xclient
)
SET_TESTS_PROPERTIES(xclient PROPERTIES TIMEOUT 180)

ADD_DEPENDENCIES(${XCL_UNIT_TESTS}
  ${MYSQLX_CLIENT_FULL_LIB}
  ${MYSQLX_PROTOCOL_FULL_LIB}
  xprotocol_tags
)

TARGET_LINK_LIBRARIES(${XCL_UNIT_TESTS}
  ${GCOV_LDFLAGS}
  ${MYSQLX_CLIENT_FULL_LIB}
  ${MYSQLX_PROTOCOL_FULL_LIB}
  ${PROTOBUF_LIBRARY}
  ${LZ4_LIBRARY}
  perconaserverclient
  gtest
  gmock
)
||||||| ee4455a33b1
IF(MY_COMPILER_IS_CLANG)
  STRING_APPEND(CMAKE_CXX_FLAGS " -Wno-deprecated")
ENDIF()

# assertion failed in function assert_with_dumps() @ ssa.c:621 when using -xO3
# Segfault in the unit test, so lower optimization level there as well
IF(MY_COMPILER_IS_SUNPRO)
  ADD_COMPILE_FLAGS(
    ${CMAKE_CURRENT_SOURCE_DIR}/xpl/callback_command_delegate_t.cc
    COMPILE_FLAGS "-xO2")
ENDIF()

TARGET_LINK_LIBRARIES(${XPL_UNIT_TESTS}
  ${GCOV_LDFLAGS}
  ${MYSQLX_CLIENT_LIB}
  mysqlx
  mysqlclient
  gtest
  gmock
)


# --== XClient unit tests (always full version) ==--

GET_DIRECTORY_PROPERTY(MYSQLX_CLIENT_FULL_LIB
  DIRECTORY ${MYSQLX_PROJECT_DIR}
  DEFINITION MYSQLX_CLIENT_FULL_LIB
)

GET_DIRECTORY_PROPERTY(MYSQLX_PROTOCOL_FULL_LIB
  DIRECTORY ${MYSQLX_PROJECT_DIR}
  DEFINITION MYSQLX_PROTOCOL_FULL_LIB
)

SET(XCL_UNIT_TESTS xclient_unit_tests)

SET(XCL_SRC
  test_main.cc
  xcl/auth_chaining_t.cc
  xcl/connection_general_t.cc
  xcl/protocol_auth_t.cc
  xcl/protocol_execute_t.cc
  xcl/protocol_global_error_t.cc
  xcl/protocol_notices_t.cc
  xcl/protocol_send_recv_t.cc
  xcl/query_t.cc
  xcl/session_capability_t.cc
  xcl/session_connect_t.cc
  xcl/session_execute_t.cc
  xcl/session_general_t.cc
  xcl/session_options_t.cc
  xcl/session_negotiation_t.cc
  xcl/sha256_scramble_t.cc
  xcl/ssl_config_t.cc
  xcl/xpriority_list_t.cc
)
# This compiler fails to handle some of the mock functions involved.
IF(MY_COMPILER_IS_SUNPRO)
  LIST(REMOVE_ITEM XCL_SRC
    xcl/auth_chaining_t.cc
    xcl/session_capability_t.cc
    xcl/session_connect_t.cc
    xcl/session_execute_t.cc
    xcl/session_general_t.cc
    xcl/session_options_t.cc
    xcl/session_negotiation_t.cc
    )
ENDIF()

ADD_COMPILE_DEFINITIONS(${XCL_SRC}
  COMPILE_DEFINITIONS USE_MYSQLX_FULL_PROTO
)

MYSQL_ADD_EXECUTABLE(${XCL_UNIT_TESTS}
  ${XCL_SRC}
  ADD_TEST xclient
)
SET_TESTS_PROPERTIES(xclient PROPERTIES TIMEOUT 180)

ADD_DEPENDENCIES(${XCL_UNIT_TESTS}
  ${MYSQLX_CLIENT_FULL_LIB}
  ${MYSQLX_PROTOCOL_FULL_LIB}
  xprotocol_tags
)

TARGET_LINK_LIBRARIES(${XCL_UNIT_TESTS}
  ${GCOV_LDFLAGS}
  ${MYSQLX_CLIENT_FULL_LIB}
  ${MYSQLX_PROTOCOL_FULL_LIB}
  ${PROTOBUF_LIBRARY}
  ${LZ4_LIBRARY}
  mysqlclient
  gtest
  gmock
)
=======
>>>>>>> mysql-8.0.23
