--let $error_log_file = $MYSQLTEST_VARDIR/tmp/my_restart.err
--let $do_not_echo_parameters = 1
--let $restart_parameters = "restart:--log-error=$error_log_file --utility-user-dynamic-privileges='$utility_user_dynamic_privileges'"

--source include/start_mysqld.inc
--connection default
--source include/shutdown_mysqld.inc

--let ABORT_ON = NOT_FOUND
--let SEARCH_FILE = $error_log_file
--let SEARCH_PATTERN = $expected_log_message
--let PRINT_SEARCH_RESULT = 1
--source include/search_pattern_in_file.inc
--remove_file $error_log_file

--let $error_log_file =
--let $restart_parameters =
--let $do_not_echo_parameters = 

