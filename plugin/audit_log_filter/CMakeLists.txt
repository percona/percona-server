# Copyright (c) 2022 Percona LLC and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

OPTION(WITH_PERCONA_AUDIT_LOG_FILTER "Build with Percona Audit Log Filter plugin" ON)

IF(WITH_PERCONA_AUDIT_LOG_FILTER)
  INCLUDE_DIRECTORIES(SYSTEM ${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS})

  SET(AUDIT_LOG_FILTER_SOURCES
      audit_log_filter.cc
      audit_log_reader.cc
      audit_encryption.cc
      audit_record.cc
      audit_filter.cc
      audit_keyring.cc
      audit_rule.cc
      audit_rule_parser.cc
      audit_rule_registry.cc
      audit_psi_info.cc
      audit_udf.cc
      component_registry_service.cc
      sys_vars.cc
      audit_table/base.cc
      audit_table/audit_log_filter.cc
      audit_table/audit_log_user.cc
      log_record_formatter.cc
      log_record_formatter/base.cc
      log_record_formatter/new.cc
      log_record_formatter/old.cc
      log_record_formatter/json.cc
      log_record_formatter/csv.cc
      log_writer.cc
      log_writer/base.cc
      log_writer/file.cc
      log_writer/syslog.cc
      log_writer/file_handle.cc
      log_writer/file_writer.cc
      log_writer/file_writer_decorator_base.cc
      log_writer/file_writer_buffering.cc
      log_writer/file_writer_compressing.cc
      log_writer/file_writer_encrypting.cc
      json_reader/audit_json_read_stream.cc
      json_reader/audit_json_handler.cc
      json_reader/file_reader.cc
      json_reader/file_reader_decorator_base.cc
      json_reader/file_reader_decompressing.cc
      json_reader/file_reader_decrypting.cc
      event_field_condition/field.cc
      event_field_condition/and.cc
      event_field_condition/or.cc
      event_field_condition/not.cc
      event_field_condition/variable.cc
      event_field_condition/function.cc
      event_field_condition/bool.cc
      event_field_action/base.cc
      event_field_action/log.cc
      event_field_action/block.cc
      event_field_action/replace_field.cc
      event_field_action/replace_filter.cc
      event_field_action/print_query_attrs.cc
      event_field_action/print_service_comp.cc
      event_filter_function.cc
      event_filter_function/base.cc
      event_filter_function/string_find.cc
      event_filter_function/query_digest.cc
  )

  MYSQL_ADD_PLUGIN(audit_log_filter ${AUDIT_LOG_FILTER_SOURCES}
                   LINK_LIBRARIES ${ZLIB_LIBRARY} ${SSL_LIBRARIES}
                   MODULE_ONLY MODULE_OUTPUT_NAME "audit_log_filter")

  IF(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1)
    TARGET_LINK_LIBRARIES(audit_log_filter stdc++fs)
  ENDIF()

  IF(UNIX)
    IF(INSTALL_MYSQLTESTDIR)
      INSTALL(DIRECTORY tests/mtr/ DESTINATION ${INSTALL_MYSQLTESTDIR}/suite/audit_log_filter COMPONENT Test)
    ENDIF()

    IF(INSTALL_MYSQLSHAREDIR)
      INSTALL(FILES "audit_log_filter_linux_install.sql"
              DESTINATION ${INSTALL_MYSQLSHAREDIR}
              COMPONENT Server
              PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
    ENDIF()
  ENDIF()
ENDIF()
