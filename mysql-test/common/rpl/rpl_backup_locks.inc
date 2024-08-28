########################################################################
# Replication-related tests for backup locks
#
# Variables to control test execution:
#   $engine - specifies additional transactional storage engine to test
########################################################################

--source include/have_myisam.inc
--source include/rpl/init_source_replica.inc

eval CREATE TABLE t_$engine (a INT) ENGINE=$engine;
CREATE TABLE t_myisam (a INT) ENGINE=MyISAM;

--source include/rpl/sync_to_replica.inc

--source include/rpl/stop_replica.inc

--connection master
--echo # connection master

eval INSERT INTO t_$engine VALUES (0);

--connection slave
--echo # connection slave

LOCK TABLES FOR BACKUP;

--source include/rpl/start_replica.inc

--connection master
--source include/rpl/sync_to_replica.inc

UNLOCK TABLES;

--source include/rpl/stop_replica.inc

--connection master
--echo # connection master

INSERT INTO t_myisam VALUES (0);

--connection slave
--echo # connection slave

--let $master_log_pos= query_get_value(SHOW REPLICA STATUS, Exec_Source_Log_Pos, 1)

LOCK TABLES FOR BACKUP;

--source include/rpl/start_replica.inc

let $wait_condition=
    SELECT COUNT(*) = 1 FROM INFORMATION_SCHEMA.PROCESSLIST
    WHERE STATE = "Waiting for table backup lock" AND
    (INFO = "INSERT INTO t_myisam VALUES (0)" OR INFO IS NULL);
--source include/wait_condition.inc

UNLOCK TABLES;

--connection master
--echo # connection master

############################################################################
# PS-4758: A sequence of LOCK TABLES FOR BACKUP and STOP REPLICA SQL_THREAD
#          can cause replication to be blocked and cannot be restarted
#          normally
############################################################################

--connection slave
--echo # connection slave

LOCK TABLES FOR BACKUP;

--connection master
--echo # connection master

INSERT INTO t_myisam VALUES (0);

--connection slave
--echo # connection slave

let $wait_condition=
    SELECT COUNT(*) = 1 FROM INFORMATION_SCHEMA.PROCESSLIST
    WHERE STATE = "Waiting for table backup lock" AND
    (INFO = "INSERT INTO t_myisam VALUES (0)" OR INFO IS NULL);
--source include/wait_condition.inc

--error ER_LOCK_OR_ACTIVE_TRANSACTION
STOP REPLICA SQL_THREAD;
UNLOCK TABLES;

--connection master
--echo # connection master

--source include/rpl/sync_to_replica.inc

--connection slave
--echo # connection slave

LOCK INSTANCE FOR BACKUP;

--connection master
--echo # connection master

CREATE TABLE t (a INT);

--connection slave
--echo # connection slave

let $wait_condition=
    SELECT COUNT(*) = 1 FROM INFORMATION_SCHEMA.PROCESSLIST
    WHERE STATE = "Waiting for backup lock" AND
    (INFO = "CREATE TABLE t (a INT)" OR INFO IS NULL);
--source include/wait_condition.inc

--error ER_RPL_CANT_STOP_REPLICA_WHILE_LOCKED_BACKUP
STOP REPLICA SQL_THREAD;
UNLOCK INSTANCE;

--connection master
--echo # connection master

DROP TABLE t;
eval DROP TABLE t_$engine, t_myisam;
--source include/rpl/sync_to_replica.inc

--source include/rpl/deinit.inc
