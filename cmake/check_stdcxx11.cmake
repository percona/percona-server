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

# cmake include to wrap if compiler supports std cxx11 and alters compiler flags
# On return, HAVE_STDCXX11 will be set

INCLUDE (CheckCCompilerFlag)
INCLUDE (CheckCXXCompilerFlag)

check_cxx_compiler_flag (-std=c++11 HAVE_STDCXX11)

IF (HAVE_STDCXX11)
  STRING (REPLACE "-std=gnu++03" "" COMMON_CXX_FLAGS ${COMMON_CXX_FLAGS})
  STRING (REPLACE "-std=gnu++03" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
  STRING (REPLACE "-std=gnu++03" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
  SET (CMAKE_CXX_FLAGS "-std=c++11 -Wno-deprecated-declarations ${CMAKE_CXX_FLAGS}")
ENDIF ()
