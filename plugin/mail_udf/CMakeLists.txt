# Copyright (c) 2018, 2019, Francisco Miguel Biete Banon. All rights reserved.
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
IF(WITH_MAIL)
  SET(MAILUDF_SOURCES
    src/plugin.cc
    src/udf_registration.cc
    src/udf_mail_send.cc
  )

  ### Configuration ###

  INCLUDE(curl)

  IF(CURL_FOUND)
    # declare the plugin itself
    MYSQL_ADD_PLUGIN(udf_mail
                     ${MAILUDF_SOURCES}
                     LINK_LIBRARIES curl
                     MODULE_ONLY
                     MODULE_OUTPUT_NAME "udf_mail")
  ELSE()
    MESSAGE(FATAL_ERROR "Plugin Mail UDF won't be compiled: curl package is missing")
  ENDIF()
ENDIF(WITH_MAIL)
