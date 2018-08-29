--echo #########
--echo # Test for upgrade partitioning with mysql_upgrade
--echo ###

--echo # Unzip the zip file.
--file_exists $DATA_ARCH_PATH
--exec unzip -qo $DATA_ARCH_PATH -d $MYSQL_TMP_DIR

--let $restart_parameters = "restart: --loose-skip-log-bin --skip-log-slave-updates --datadir=$MYSQLD_DATADIR1 $ADDITIONAL_OPTS"
--let $restart_hide_args = 1
--source include/start_mysqld.inc

--echo # Execute mysql_upgrade
--source include/mysql_upgrade_preparation.inc
--exec $MYSQL_UPGRADE --skip-verbose --force 2>&1
--source include/mysql_upgrade_cleanup.inc

--echo # Check test table after upgrade
SHOW CREATE TABLE test.t1;
SELECT * FROM test.t1 ORDER BY a;
--source include/table_files_replace_pattern.inc
--list_files $MYSQLD_DATADIR1/test/

--echo # Stop the server
--source include/shutdown_mysqld.inc

--let $restart_parameters =
--let $restart_hide_args =

--echo # Remove copied files
--remove_file $DATA_ARCH_PATH
--force-rmdir $MYSQLD_DATADIR1
