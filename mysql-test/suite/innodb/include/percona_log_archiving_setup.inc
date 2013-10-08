#
# setup for log archive testing
#


call mtr.add_suppression("InnoDB: Resizing redo log");
call mtr.add_suppression("InnoDB: Starting to delete and rewrite log files");
call mtr.add_suppression("InnoDB: New log files created");
call mtr.add_suppression("InnoDB: The log sequence numbers [0-9]+ and [0-9]+ in ibdata files do not match the log sequence number [0-9]+ in the ib_logfiles");

--exec echo "wait" > $MYSQLTEST_VARDIR/tmp/mysqld.1.expect
--shutdown_server
--source include/wait_until_disconnected.inc
# Do something while server is down
--enable_reconnect
--exec echo "restart: --innodb_log_archive=ON --innodb_log_arch_dir=logs_archive --innodb-log-file-size=1M" > $MYSQLTEST_VARDIR/tmp/mysqld.1.expect
--source include/wait_until_connected_again.inc

--disable_warnings
drop table if exists t;
--enable_warnings

let $MYSQLD_DATADIR= `SELECT @@datadir`;
let $MYSQLD_ARCHIVEDIR= `SELECT IF( locate('/', @@innodb_log_arch_dir) = 1, @@innodb_log_arch_dir, CONCAT(@@datadir, @@innodb_log_arch_dir))`;
let MYSQLD_ARCHIVEDIR = $MYSQLD_ARCHIVEDIR;
--mkdir $MYSQLD_ARCHIVEDIR

let $LOG_FILE_HDR_SIZE = 2048;
let $LOG_FILE_SIZE = `SELECT @@innodb_log_file_size`;
let $LOG_FILE_LSN_STEP = `SELECT $LOG_FILE_SIZE - $LOG_FILE_HDR_SIZE`;

create table t (a int not null) ENGINE=InnoDB;

insert into t values (1),(2),(3),(4),(5),(6),(7),(8),(9),(10),(11),(12),(13);
insert into t values (1),(2),(3),(4),(5),(6),(7),(8),(9),(10),(11),(12),(13);
insert into t values (1),(2),(3),(4),(5),(6),(7),(8),(9),(10),(11),(12),(13);
insert into t values (1),(2),(3),(4),(5),(6),(7),(8),(9),(10),(11),(12),(13);
insert into t values (1),(2),(3),(4),(5),(6),(7),(8),(9),(10),(11),(12),(13);
insert into t values (1),(2),(3),(4),(5),(6),(7),(8),(9),(10),(11),(12),(13);
insert into t values (1),(2),(3),(4),(5),(6),(7),(8),(9),(10),(11),(12),(13);
insert into t values (1),(2),(3),(4),(5),(6),(7),(8),(9),(10),(11),(12),(13);
insert into t values (1),(2),(3),(4),(5),(6),(7),(8),(9),(10),(11),(12),(13);
insert into t values (1),(2),(3),(4),(5),(6),(7),(8),(9),(10),(11),(12),(13);
insert into t values (1),(2),(3),(4),(5),(6),(7),(8),(9),(10),(11),(12),(13);
insert into t (a) select t1.a from t t1, t t2, t t3 LIMIT 40000;

--source percona_log_archiving_stat.inc

let $LAST_ARCHIVED_LSN = $last_log_file_lsn;
