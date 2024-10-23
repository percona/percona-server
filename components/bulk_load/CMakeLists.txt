# Copyright (c) 2024, Percona and/or its affiliates.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.

# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation. The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License, version 2.0, for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

ADD_DEFINITIONS(-DLOG_COMPONENT_TAG="bulk_load")

MYSQL_ADD_COMPONENT(bulk_load
  bulk_load_component.cc
  bulk_loader.cc
  bulk_loader/bulk_loader_abstract.cc
  bulk_loader/bulk_loader_local.cc
  bulk_loader/bulk_loader_s3.cc
  data_stream/stream_local/data_stream_local.cc
  data_stream/stream_s3/data_stream_s3.cc
  stream_parser/parser_iterator.cc
  stream_parser/parser_row.cc
  stream_parser/stream_parser.cc
  LINK_LIBRARIES extra::rapidjson
  MODULE_ONLY
)

target_include_directories(
  component_bulk_load
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)
