/* Copyright (c) 2022 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

CREATE TABLE IF NOT EXISTS mysql.audit_log_filter(
  filter_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
  name VARCHAR(255) COLLATE utf8_bin NOT NULL,
  filter JSON NOT NULL,
  PRIMARY KEY (`filter_id`),
  UNIQUE KEY `filter_name` (`name`)
) Engine=InnoDB CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_as_ci;

CREATE TABLE IF NOT EXISTS mysql.audit_log_user(
  username VARCHAR(32) COLLATE utf8_bin NOT NULL,
  userhost VARCHAR(255) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL,
  filtername VARCHAR(255) COLLATE utf8_bin NOT NULL,
  PRIMARY KEY (username, userhost),
  FOREIGN KEY `filter_name` (filtername) REFERENCES mysql.audit_log_filter(name)
) Engine=InnoDB CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_as_ci;

INSTALL PLUGIN audit_log_filter SONAME 'audit_log_filter.so';
