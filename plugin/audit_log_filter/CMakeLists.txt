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

MYSQL_ADD_PLUGIN(audit_log_filter
                 audit_log_filter.cc
                 audit_log_reader.cc
                 audit_log_json_handler.cc
                 audit_record.cc
                 audit_filter.cc
                 audit_rule.cc
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
                 log_writer/file_buffer.cc
                 log_writer/file_handle.cc
                 log_writer_strategy.cc
                 log_writer_strategy/async.cc
                 log_writer_strategy/perf.cc
                 log_writer_strategy/semisync.cc
                 log_writer_strategy/sync.cc
                 event_field_condition/field.cc
                 event_field_condition/and.cc
                 event_field_condition/or.cc
                 event_field_condition/not.cc
                 event_field_condition/variable.cc
                 event_field_condition/function.cc
                 event_field_condition/bool.cc
                 event_field_action/log.cc
                 event_field_action/block.cc
                 event_field_action/replace_field.cc
                 event_field_action/replace_filter.cc
                 event_filter_function.cc
                 event_filter_function/base.cc
                 event_filter_function/string_find.cc
                 event_filter_function/query_digest.cc
                 MODULE_ONLY MODULE_OUTPUT_NAME "audit_log_filter")

IF(UNIX)
  IF(INSTALL_MYSQLTESTDIR)
    INSTALL(DIRECTORY tests/mtr/ DESTINATION ${INSTALL_MYSQLTESTDIR}/suite/audit_log_filter COMPONENT Test)
  ENDIF()
ENDIF()
