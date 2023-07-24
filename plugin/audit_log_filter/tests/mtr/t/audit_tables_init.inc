# ==== Purpose ====
#
# Create tables needed for the plugin configuration in specified database.
#
# ==== Usage ====
#
# --let $audit_db_name = database name where to create tables
# --source audit_tables_init.inc
#
# Parameters:
#
# $audit_db_name
#   Database name, defaults to 'mysql'

if ($audit_db_name == '') {
--let $audit_db_name = mysql
}

--disable_query_log
--disable_result_log

call mtr.add_suppression("Plugin audit_log_filter reported: 'Failed to get an opened audit_log_filter table");
call mtr.add_suppression("Plugin audit_log_filter reported: 'Failed to load filtering rules");

eval DROP TABLE IF EXISTS $audit_db_name.audit_log_user;
eval DROP TABLE IF EXISTS $audit_db_name.audit_log_filter;

eval CREATE TABLE $audit_db_name.audit_log_filter(
    filter_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
    name VARCHAR(255) NOT NULL,
    filter JSON NOT NULL,
    PRIMARY KEY (`filter_id`),
    UNIQUE KEY `filter_name` (`name`)
) Engine=InnoDB CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_as_ci;

eval CREATE TABLE $audit_db_name.audit_log_user(
    username VARCHAR(32) NOT NULL,
    userhost VARCHAR(255) NOT NULL,
    filtername VARCHAR(255) NOT NULL,
    PRIMARY KEY (username, userhost),
    FOREIGN KEY `filter_name` (filtername) REFERENCES $audit_db_name.audit_log_filter(name)
) Engine=InnoDB CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_as_ci;

SELECT audit_log_filter_flush();

--enable_query_log
--enable_result_log
