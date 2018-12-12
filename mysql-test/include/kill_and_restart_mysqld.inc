if (!$restart_parameters)
{
  let $restart_parameters = restart;
}

--let $_server_id= `SELECT @@server_id`
--let $_expect_file_name= $MYSQLTEST_VARDIR/tmp/mysqld.$_server_id.expect

if (!$restart_hide_args)
{
  --echo # Kill and $restart_parameters
}
if ($restart_hide_args)
{
  --echo # Kill and restart:<hidden args>
  let $restart_hide_args=;
}
--exec echo "$restart_parameters" > $_expect_file_name
--shutdown_server 0
--source include/wait_until_disconnected.inc
--enable_reconnect
--source include/wait_until_connected_again.inc
--disable_reconnect
