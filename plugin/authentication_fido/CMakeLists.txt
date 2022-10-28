#Copyright (c) 2022 Percona LLC and/or its affiliates. All rights reserved.
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

IF(NOT FIDO_FOUND)
  MESSAGE(WARNING "Skipping the FIDO server authentication plugin")
  RETURN()
ENDIF()

SET(AUTH_FIDO_SOURCES
  src/plugin_fido.cc
)
### Configuration ###
ADD_DEFINITIONS(-DLOG_COMPONENT_TAG="authentication_fido")

INCLUDE_DIRECTORIES(SYSTEM ${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR})

MY_INCLUDE_SYSTEM_DIRECTORIES(FIDO)

DISABLE_MISSING_PROFILE_WARNING()

MYSQL_ADD_PLUGIN(auth_fido ${AUTH_FIDO_SOURCES}
  LINK_LIBRARIES
  ${FIDO_LIBRARY}
  ${OPENSSL_LIBRARIES}
  MODULE_ONLY MODULE_OUTPUT_NAME "authentication_fido"
  )
