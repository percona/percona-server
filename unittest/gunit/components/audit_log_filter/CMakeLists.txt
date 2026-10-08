# Copyright (c) 2026 Percona LLC and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; version 2 of
# the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

SET(AUDIT_LOG_FILTER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/components/audit_log_filter)

MYSQL_ADD_EXECUTABLE(audit_regex-t
  audit_regex-t.cc
  ${AUDIT_LOG_FILTER_SOURCE_DIR}/audit_regex.cc
  ADD_TEST audit_regex
  LINK_LIBRARIES ext::icu gunit_small
)

# Bundled ICU loads data (e.g. character names) from files, mysqld finds them
# using icudtXXl.lnk in the build directory. System ICU has built-in data.
# Only the bundled ICU is patched to survive allocation failures during
# pattern compilation.
IF(WITH_ICU STREQUAL "bundled")
  TARGET_COMPILE_DEFINITIONS(audit_regex-t PRIVATE
    AUDIT_REGEX_TEST_ICU_DATA_DIR="${CMAKE_SOURCE_DIR}/extra/icu"
    AUDIT_REGEX_TEST_PATCHED_ICU)
ENDIF()

# Regex condition with the production reporting policy, the unit test
# provides its own failure sink instead of server services.
MYSQL_ADD_EXECUTABLE(audit_field_regex-t
  audit_field_regex-t.cc
  ${AUDIT_LOG_FILTER_SOURCE_DIR}/audit_regex.cc
  ${AUDIT_LOG_FILTER_SOURCE_DIR}/audit_record.cc
  ${AUDIT_LOG_FILTER_SOURCE_DIR}/event_field_condition/and.cc
  ${AUDIT_LOG_FILTER_SOURCE_DIR}/event_field_condition/bool.cc
  ${AUDIT_LOG_FILTER_SOURCE_DIR}/event_field_condition/field_regex.cc
  ${AUDIT_LOG_FILTER_SOURCE_DIR}/event_field_condition/not.cc
  ${AUDIT_LOG_FILTER_SOURCE_DIR}/event_field_condition/or.cc
  ADD_TEST audit_field_regex
  LINK_LIBRARIES ext::icu gunit_small
)
TARGET_COMPILE_DEFINITIONS(audit_field_regex-t PRIVATE
  AUDIT_LOG_FILTER_REGEX_TEST_SINK)
