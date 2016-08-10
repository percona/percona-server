if (!$restart_parameters)
{
  let $restart_parameters = restart;
}

if ($restart_parameters != "restart")
{
  # Only echo $restart_parameters if they are not empty
  # This is different from both Percona Server 5.5 (which backported this
  # file from 5.7) and Percona Server 5.7, where upstream has
  # $restart_parameters too and echoes them unconditionally.
  # This is to avoid unnecassary re-records of upstream test cases.
  --echo # $restart_parameters
}

# Include this script only after using shutdown_mysqld.inc
# or kill_mysqld.inc
# where $_expect_file_name was initialized.
# Write file to make mysql-test-run.pl start up the server again
--exec echo "$restart_parameters" > $_expect_file_name

# Turn on reconnect
--enable_reconnect

# Call script that will poll the server waiting for it to be back online again
--source include/wait_until_connected_again.inc

# Turn off reconnect again
--disable_reconnect

