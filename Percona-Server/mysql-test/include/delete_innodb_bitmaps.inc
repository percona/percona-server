# Remove all the InnoDB bitmap files. A temporary measure until RESET user request is implemented

let $MYSQLD_DATADIR= `select @@datadir`;

--exec echo "wait" > $MYSQLTEST_VARDIR/tmp/mysqld.1.expect
--shutdown_server 10
--source include/wait_until_disconnected.inc
remove_files_wildcard $MYSQLD_DATADIR ib_modified_log*;
--enable_reconnect
--exec echo "restart" > $MYSQLTEST_VARDIR/tmp/mysqld.1.expect
--source include/wait_until_connected_again.inc
