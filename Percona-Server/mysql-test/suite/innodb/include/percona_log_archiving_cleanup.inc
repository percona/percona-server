#
# Cleanup after log archiving testing
#

DROP TABLE t;

--exec echo "wait" > $MYSQLTEST_VARDIR/tmp/mysqld.1.expect
--shutdown_server
--source include/wait_until_disconnected.inc
--enable_reconnect
--exec echo "restart: --innodb-log-file-size=1M" > $MYSQLTEST_VARDIR/tmp/mysqld.1.expect
--source include/wait_until_connected_again.inc

--remove_files_wildcard $MYSQLD_ARCHIVEDIR ib_log_archive_*
--rmdir $MYSQLD_ARCHIVEDIR
