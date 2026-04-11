# Copyright (c) 2026, Percona LLC and/or its affiliates.
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

IF(WITH_PERCONA_AUTHENTICATION_APPKEY)
  ADD_DEFINITIONS(-DLOG_COMPONENT_TAG="authentication_appkey")

  MYSQL_ADD_PLUGIN(authentication_appkey
    authentication_appkey.cc
    san_parser.cc
    LINK_LIBRARIES OpenSSL::SSL OpenSSL::Crypto
    MODULE_ONLY
    MODULE_OUTPUT_NAME "authentication_appkey")

  IF(UNIX)
    IF(INSTALL_MYSQLTESTDIR)
      INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/mysql-test/suite/auth_appkey/
              DESTINATION ${INSTALL_MYSQLTESTDIR}/suite/auth_appkey
              COMPONENT Test)
    ENDIF(INSTALL_MYSQLTESTDIR)
  ENDIF(UNIX)
ENDIF(WITH_PERCONA_AUTHENTICATION_APPKEY)
