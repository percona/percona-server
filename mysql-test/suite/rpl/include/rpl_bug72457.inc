--source include/master-slave.inc

call mtr.add_suppression("Slave SQL for channel '': The incident LOST_EVENTS occured on the master\\. Message: The content of the statement cache is corrupted while writing a rollback record of the transaction to the binary log\\. Error_code:");
call mtr.add_suppression("The content of the statement cache is corrupted while writing a rollback record of the transaction to the binary log\\. An incident event has been written to the binary log which will stop the slaves\\.");

SET @saved_binlog_error_action = @@global.binlog_error_action;
SET GLOBAL binlog_error_action = IGNORE_ERROR;

CREATE TABLE t1(f1 TEXT) ENGINE=MyISAM;
INSERT INTO t1 VALUES(MD5(1));

--let $i = 10
while($i)
{
  INSERT INTO t1 SELECT * FROM t1;
  --dec $i
}
--source include/sync_slave_sql_with_master.inc

--source include/rpl_connection_master.inc
SET SESSION debug = "+d,simulate_tmpdir_partition_full";
--replace_regex /Error writing file .*/Error writing file <tmp_file_name> (Errcode: ##)/
--error 3
INSERT INTO t1 SELECT * FROM t1;
SET SESSION debug = "-d,simulate_tmpdir_partition_full";

--source include/rpl_connection_slave.inc
--let $slave_sql_errno = convert_error(ER_SLAVE_INCIDENT)
--let $show_slave_sql_error = 1
--source include/wait_for_slave_sql_error.inc
--source include/stop_slave_io.inc
RESET SLAVE;
DROP TABLE t1;

--source include/rpl_connection_master.inc
DROP TABLE t1;

SET GLOBAL binlog_error_action = @saved_binlog_error_action;

--let $rpl_only_running_threads = 1
--source include/rpl_end.inc
