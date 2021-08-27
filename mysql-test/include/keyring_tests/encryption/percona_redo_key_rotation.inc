let $MYSQLD_BASEDIR= `select @@basedir`;
--mkdir $MYSQL_TMP_DIR/datadir1

let $MYSQLD_DATADIR1 = $MYSQL_TMP_DIR/datadir1/data;
--source include/shutdown_mysqld.inc

--let BOOTSTRAP_SQL=$MYSQL_TMP_DIR/boot.sql
--let BOOTSTRAP_LOG=$MYSQL_TMP_DIR/bootstrap.log

--echo # create bootstrap file
write_file $BOOTSTRAP_SQL;
CREATE DATABASE test;
EOF

--let BOOTSTRAP_CMD = $MYSQLD --no-defaults --initialize-insecure --basedir=$MYSQLD_BASEDIR --datadir=$MYSQLD_DATADIR1 --init-file=$BOOTSTRAP_SQL $PLUGIN_DIR_OPT --innodb_redo_log_encrypt=KEYRING_KEY > $BOOTSTRAP_LOG 2>&1

--exec $BOOTSTRAP_CMD

--remove_file $BOOTSTRAP_LOG
--remove_file $MYSQLD_DATADIR1/ib_logfile0
--remove_file $MYSQLD_DATADIR1/ib_logfile1

--let $test_error_log=$MYSQL_TMP_DIR/error_log.err

# Check that percona_redo-UUID:1 key is used after log files removal and server restart.

--let $restart_parameters = "restart: $PLUGIN_DIR_OPT --loose-skip-log-bin --skip-log-slave-updates --datadir=$MYSQLD_DATADIR1 --log-error=$test_error_log --innodb_redo_log_encrypt=KEYRING_KEY"
--replace_result $PLUGIN_DIR_OPT PLUGIN_DIR_OPT $MYSQLD_DATADIR1 MYSQLD_DATADIR1 $MYSQL_TMP_DIR MYSQL_TMP_DIR
--source include/start_mysqld.inc

--let $server_uuid= query_get_value(SELECT @@SERVER_UUID, @@SERVER_UUID, 1)

--source include/shutdown_mysqld.inc

# Check that there is no percona_redo keys without UUID

--let $percona_redo_key_name_v0=percona_redo:0

--let SEARCH_PATTERN=$percona_redo_key_name_v0
--let ABORT_ON=FOUND
--let SEARCH_FILE=$KEYRING_FILE_PATH
--source include/search_pattern_in_file.inc

--let $percona_redo_key_name_v1=percona_redo:1

--let SEARCH_PATTERN=$percona_redo_key_name_v1
--let ABORT_ON=FOUND
--source include/search_pattern_in_file.inc

# Check that percona_redo-UUID key exists only in version 1

--let $percona_key_name_v1=percona_redo-$server_uuid:1

--let SEARCH_PATTERN=$percona_key_name_v1
--let ABORT_ON=NOT_FOUND
--source include/search_pattern_in_file.inc

--let $percona_key_name_v2=percona_redo-$server_uuid:2

--let SEARCH_PATTERN=$percona_key_name_v2
--let ABORT_ON=FOUND
--source include/search_pattern_in_file.inc

# Rotate percona_redo-UUID key to version 2.

--let $restart_parameters = "restart: $PLUGIN_DIR_OPT --loose-skip-log-bin --skip-log-slave-updates --datadir=$MYSQLD_DATADIR1 --log-error=$test_error_log --innodb_redo_log_encrypt=KEYRING_KEY"
--replace_result $PLUGIN_DIR_OPT PLUGIN_DIR_OPT $MYSQLD_DATADIR1 MYSQLD_DATADIR1 $MYSQL_TMP_DIR MYSQL_TMP_DIR
--source include/start_mysqld.inc

ALTER INSTANCE ROTATE REDO SYSTEM KEY;

--source include/shutdown_mysqld.inc

# Check that percona_redo-UUID:2 key exists

--let SEARCH_PATTERN=$percona_key_name_v2
--let ABORT_ON=NOT_FOUND
--source include/search_pattern_in_file.inc

--remove_file $MYSQLD_DATADIR1/ib_logfile0
--remove_file $MYSQLD_DATADIR1/ib_logfile1

--let $restart_parameters = "restart: $PLUGIN_DIR_OPT --loose-skip-log-bin --skip-log-slave-updates --datadir=$MYSQLD_DATADIR1 --log-error=$test_error_log --innodb_redo_log_encrypt=KEYRING_KEY"
--replace_result $PLUGIN_DIR_OPT PLUGIN_DIR_OPT $MYSQLD_DATADIR1 MYSQLD_DATADIR1 $MYSQL_TMP_DIR MYSQL_TMP_DIR
--source include/start_mysqld.inc

--source include/shutdown_mysqld.inc

--echo # Restart the server with default options.
--let $restart_parameters=
--source include/start_mysqld.inc

--remove_file $test_error_log
--remove_file $BOOTSTRAP_SQL
--force-rmdir $MYSQL_TMP_DIR/datadir1
