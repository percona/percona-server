--echo #########
--echo # Test for "ALTER TABLE ... UPGRADE PARTITIONING"
--echo ###

--echo # Unzip the zip file.
--file_exists $DATA_ARCH_PATH
--exec unzip -qo $DATA_ARCH_PATH -d $MYSQL_TMP_DIR

--echo # Create a bootstrap file in temp location
--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR
--exec echo $MYSQL_TMP_DIR/bootstrap.log

--let $restart_parameters = "restart: --loose-skip-log-bin --skip-log-slave-updates --datadir=$MYSQLD_DATADIR1 $ADDITIONAL_OPTS"
--let $restart_hide_args = 1
--source include/start_mysqld.inc

--echo # Check test table before upgrade
SHOW CREATE TABLE test.t1;
SELECT * FROM test.t1 ORDER BY a;
--source include/table_files_replace_pattern.inc
--list_files $MYSQLD_DATADIR1/test/

--echo # Upgrade
ALTER TABLE test.t1 UPGRADE PARTITIONING;

--echo # Check test table after upgrade
SHOW CREATE TABLE test.t1;
SELECT * FROM test.t1 ORDER BY a;
--source include/table_files_replace_pattern.inc
--list_files $MYSQLD_DATADIR1/test/

--echo # Stop the server
--source include/shutdown_mysqld.inc

--let $restart_parameters =
--let $restart_hide_args =

--echo # Remove data directory
--exec rm -rf $MYSQLD_DATADIR1
