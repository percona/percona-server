# Copyright (c) 2016, 2021, Oracle and/or its affiliates.
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
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

ADD_SUBDIRECTORY(src)

IF(WITH_UNIT_TESTS)
<<<<<<< HEAD
  ADD_LIBRARY(metadata_cache_static STATIC ${METADATA_CACHE_SOURCES})
  TARGET_LINK_LIBRARIES(metadata_cache_static
    router_lib
    perconaserverclient
    mysqlxmessages_lite
    ${PROTOBUF_LITE_LIBRARY}
    )
  TARGET_COMPILE_DEFINITIONS(metadata_cache_static
    PRIVATE -Dmetadata_cache_DEFINE_STATIC=1)
  TARGET_INCLUDE_DIRECTORIES(metadata_cache_static PRIVATE ${include_dirs})

  ADD_SUBDIRECTORY(tests/)
||||||| 6846e6b2f72
  ADD_LIBRARY(metadata_cache_static STATIC ${METADATA_CACHE_SOURCES})
  TARGET_LINK_LIBRARIES(metadata_cache_static
    router_lib
    mysqlclient
    mysqlxmessages_lite
    ${PROTOBUF_LITE_LIBRARY}
    )
  TARGET_COMPILE_DEFINITIONS(metadata_cache_static
    PRIVATE -Dmetadata_cache_DEFINE_STATIC=1)
  TARGET_INCLUDE_DIRECTORIES(metadata_cache_static PRIVATE ${include_dirs})

  ADD_SUBDIRECTORY(tests/)
=======
  ADD_SUBDIRECTORY(tests)
>>>>>>> mysql-8.0.29
ENDIF()
