--echo #########
--echo # End upgrade testing with mysql_upgrade
--echo ###

--echo # Stop the server
--source include/shutdown_mysqld.inc

if ($ZENFS_AWARE)
{
  if (!$rocksdb_zenfs_disabled)
  {
    --exec $MYSQL_ZENFS rmdir --zbd=$extracted_zenfs_device --path=/.rocksdb.$ARCH_DATA --force >/dev/null 2>&1
  }
}

--let $restart_parameters =
--let $do_not_echo_parameters =

--echo # Remove temp datadir
--force-rmdir $MYSQLD_DATADIR1

--echo # Restart the server with default options.
--source include/start_mysqld.inc
