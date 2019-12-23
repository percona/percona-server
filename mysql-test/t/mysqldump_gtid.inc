--source include/have_log_bin.inc
--source include/have_innodb.inc
--source include/not_embedded.inc
--source include/have_debug_sync.inc
--source include/count_sessions.inc

# start-sql-file/finish-sql-file options of mysqldump available only in debug build
--source include/have_debug.inc

# Handling of background process
--source include/not_windows.inc


--echo #################################################
--echo # Testing mysqldump with options: $MYSQLDUMP_OPTS
--echo #################################################

--let $sleep_sec = 5
--let $start_sql_file = $MYSQL_TEST_DIR/std_data/mysqldump_gtid_start.sql
--let $finish_sql_file = $MYSQL_TEST_DIR/std_data/mysqldump_gtid_finish.sql

--connect(con1, localhost, root)

--connection default
CREATE TABLE t1 (a INT) ENGINE = InnoDB;
INSERT INTO t1 VALUES (0);

# Capture current gtid
--let $gtid_start = `SELECT @@GLOBAL.GTID_EXECUTED`

# Start mysqldump in background collecting its PID
--let $command = $MYSQL_DUMP
--let $command_opt = $MYSQLDUMP_OPTS test t1 --start-sql-file=$start_sql_file --finish-sql-file=$finish_sql_file
--let $output_file = $MYSQLTEST_VARDIR/tmp/dump.sql
--let $pid_file = $MYSQLTEST_VARDIR/tmp/mysqldump.pid
--source include/start_proc_in_background.inc

# Wait for mysqldump to reach the point just before getting gtid_executed
SET DEBUG_SYNC = 'now WAIT_FOR gtid_executed.started';

# Now mysqldump started its job, let's try to insert something
# Following insert should be blocked. Pause execution for a while to be sure it is blocked
# before we let mysqldump to continue
--send INSERT INTO t1 VALUES (1)
--real_sleep $sleep_sec

# Now let's go to another connection and let mysqldump to continue
--connection con1
SET DEBUG_SYNC = 'now SIGNAL gtid_executed.continue';

# Back to default connection. Get result of insert as it should be unblocked when gtid_executed is queried
# and wait for mysqldump to finish
--connection default
--reap
SET DEBUG_SYNC = 'now WAIT_FOR mysqldump.finished';

# Now gtid_executed changed, but mysqldump should have grab the one that was present before 2nd insert.
# mysqldump signaled its finish, however process may not exited yet and dump file probably is not flushed yet.
# $pid_file is already initalized
--source include/wait_proc_to_finish.inc

--let $gtid_finish = `SELECT @@GLOBAL.GTID_EXECUTED`

# We expect this gtid to be present
--let $grep_output = boolean
--let $grep_file = $output_file
--let $grep_pattern = $gtid_start
--source include/grep_pattern.inc

# We expect this gtid not to be present
--let $grep_pattern = $gtid_finish
--let $include_filename = grep_pattern.inc
--source include/grep_pattern.inc

# cleanup
--disconnect con1
DROP TABLE t1;
SET DEBUG_SYNC = 'RESET';
--remove_file $output_file
--remove_file $pid_file

--source include/wait_until_count_sessions.inc

