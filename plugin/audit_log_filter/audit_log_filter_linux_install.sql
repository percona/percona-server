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

SELECT @sys_var_database := variable_value FROM performance_schema.global_variables WHERE variable_name = 'audit_log_filter_database';
SET @db_name = IFNULL(IFNULL(@sys_var_database, DATABASE()), 'mysql');

SET @create_filter = CONCAT(
  'CREATE TABLE IF NOT EXISTS ', @db_name, '.audit_log_filter (',
    'filter_id INT UNSIGNED NOT NULL AUTO_INCREMENT,',
    'name VARCHAR(255) COLLATE utf8_bin NOT NULL,',
    'filter JSON NOT NULL,',
    'PRIMARY KEY (`filter_id`),',
    'UNIQUE KEY `filter_name` (`name`)',
  ') Engine = InnoDB CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_as_ci'
);

SET @create_user = CONCAT(
  'CREATE TABLE IF NOT EXISTS ', @db_name, '.audit_log_user(',
    'username VARCHAR(32) COLLATE utf8_bin NOT NULL,',
    'userhost VARCHAR(255) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL,',
    'filtername VARCHAR(255) COLLATE utf8_bin NOT NULL,',
    'PRIMARY KEY (username, userhost), FOREIGN KEY `filter_name` (filtername) REFERENCES ', @db_name, '.audit_log_filter(name)'
  ') Engine = InnoDB CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_as_ci'
);

PREPARE stmt from @create_filter;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

PREPARE stmt from @create_user;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

DROP PROCEDURE IF EXISTS mysql.install_audit_log;

DELIMITER //
CREATE PROCEDURE mysql.install_audit_log()
BEGIN
  DECLARE status VARCHAR(20);

  SELECT PLUGIN_STATUS INTO status FROM INFORMATION_SCHEMA.PLUGINS WHERE PLUGIN_NAME = 'audit_log_filter';

  IF ISNULL(status) THEN
    INSTALL PLUGIN audit_log_filter SONAME 'audit_log_filter.so';
  END IF;
END//
DELIMITER ;

CALL mysql.install_audit_log();

DROP PROCEDURE mysql.install_audit_log;
