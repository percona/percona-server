--source include/have_debug.inc

#
# Test the effect of various signals on the server
#
--let $test_error_log= $MYSQLTEST_VARDIR/tmp/test_threadpool_sig_handler.err
--let $restart_parameters=restart:--log-error=$test_error_log --debug=d,simulate_sighup_print_status
--replace_result $MYSQLTEST_VARDIR MYSQLTEST_VARDIR
--source include/restart_mysqld.inc

SELECT @@GLOBAL.thread_handling;

--let $mysqld_pid_file=`SELECT @@GLOBAL.pid_file`

--echo # Test SIGPIPE, SIGTSTP, SIGINT, which should be ignored
exec kill -PIPE `cat $mysqld_pid_file`;
exec kill -TSTP `cat $mysqld_pid_file`;
exec kill -INT `cat $mysqld_pid_file`;

--echo # Test SIGHUP, which should result in status dump in the error log
exec kill -HUP `cat $mysqld_pid_file`;

SELECT 1;

--echo #
--echo # Bug 805805: attaching to percona-server with gdb disconnects clients
--echo # Bug 1060136: safe_process.cc/safe_process.pl should not kill mysqld on SIGSTOP/SIGCONT
--echo #

exec kill -STOP `cat $mysqld_pid_file`;
exec kill -CONT `cat $mysqld_pid_file`;

SELECT 1;

--echo #
--echo # Bug 1537554: SIGTERM kills server instead of shutdown w/ threadpool
--echo #

--source include/expect_crash.inc
exec kill -TERM `cat $mysqld_pid_file`;
--source include/wait_until_disconnected.inc

--let $restart_parameters=
--source include/start_mysqld.inc

# Test for SIGHUP output
--let SEARCH_FILE=$test_error_log
--let SEARCH_PATTERN=Status information:
--source include/search_pattern.inc

# Test for SIGTERM output
--let SEARCH_PATTERN=mysqld.*: Shutdown complete
--source include/search_pattern.inc

--remove_file $test_error_log
