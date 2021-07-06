# Start the server with the provided value for --replica-enable-event.
# This may result with startup abort, or proper startup with no errors.
# Validate the expected log accordingly.
#
# Usage:
#
# --let $wild_pattern = db%.%event%
# --let $expect_abort = no  <-- We expect that server shall start properly
# --let $reason             <-- Message to be searched in the error log for validation
# --source rpl_enable_event_cmd_line_validation.inc
#
# === References ===
#
# PS-6002: Create/alter event state to keep enabled on replicas

--echo
--echo Starting the replica server with --replica-enable-event='$wild_pattern'

--let $error_log= $MYSQL_TMP_DIR/test_error_log.err
--let $start_params = --replica-enable-event=$wild_pattern

if ($expect_abort == 'yes')
{
  --let $rpl_server_parameters= --log-error=$error_log $start_params
  --let $mysqld= $MYSQLD --defaults-file=$MYSQLTEST_VARDIR/my.cnf --defaults-group-suffix=.2

  # Server abort is expected
  --error 1
  --exec $mysqld $rpl_server_parameters
  --echo Server Startup aborted as expected.
}

if ($expect_abort != 'yes')
{
  --let $rpl_server_number = 2
  --let $rpl_omit_print_server_parameters= 1
  --let $rpl_server_parameters= --log-error=$error_log $start_params
  --source include/rpl_start_server.inc
}

# Validation
if ($expect_abort == 'yes')
{
  # Restart the server with defaut options for validation
  --let $rpl_server_number = 2
  --let $rpl_omit_print_server_parameters= 1
  --let $rpl_server_parameters=
  --source include/rpl_start_server.inc

  # Assert that the server logs the message "Could not add event rule .* to --replica-enable-event"
  --let $assert_file= $error_log
  --let $assert_text= Found the message: Could not add event rule '$wild_pattern' to --replica-enable-event
  --let $assert_select= \[ERROR\] .* Could not add event rule '$wild_pattern' to --replica-enable-event
  --let $assert_count= 1
  --source include/assert_grep.inc

  # Additional validation for the exact reason for the server abort.
  --let $assert_file= $error_log
  --let $assert_text= Found the reason: $reason
  --let $assert_select= $reason
  --let $assert_count= 1
  --source include/assert_grep.inc
}

--echo # Stop the server
--source include/rpl_stop_server.inc
--remove_file $error_log
