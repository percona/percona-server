#Copyright (c) 2026 Percona LLC and/or its affiliates. All rights reserved.
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

IF(WITH_AUTHENTICATION_OPENID_CONNECT)

    SET(AUTH_OPENID_CONNECT_SOURCES
            src/plugin_openid_connect.cc
            src/config.cc
            src/jwk.cc
            src/jwks.cc
            src/udf.cc
            src/id_token.cc
    )
    ### Configuration ###
    ADD_DEFINITIONS(-DLOG_COMPONENT_TAG="authentication_openid_connect")
    SET(JWT_CPP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extra/jwt-cpp/include)
    MESSAGE(STATUS "JWT CPP include dir is " ${JWT_CPP_INCLUDE_DIR})

    #INCLUDE_DIRECTORIES(SYSTEM ${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR})

    #DISABLE_MISSING_PROFILE_WARNING()

    MYSQL_ADD_PLUGIN(authentication_openid_connect
            ${AUTH_OPENID_CONNECT_SOURCES}
            LINK_LIBRARIES OpenSSL::SSL OpenSSL::Crypto curl
            SYSTEM_INCLUDE_DIRECTORIES "${JWT_CPP_INCLUDE_DIR}"
            MODULE_ONLY MODULE_OUTPUT_NAME "authentication_openid_connect"
    )

ENDIF(WITH_AUTHENTICATION_OPENID_CONNECT)