# Prepared XA transactions on MyRocks tables survive server kills under
# every write policy. Used by xa_prepared_recovery (binary log on) and
# xa_prepared_recovery_binlog_off. Clean restarts with a prepared
# transaction are covered by xa_prepared_restart* (write_committed) and
# xa_prepared_shutdown.

CALL mtr.add_suppression("Found [0-9]+ prepared XA transactions");

CREATE TABLE t1 (a INT PRIMARY KEY, b INT, KEY kb (b)) ENGINE=ROCKSDB;
INSERT INTO t1 VALUES (1, 1), (2, 2), (3, 3);

--echo # The prepared transaction keeps its row locks after a kill and is
--echo # then committed.
XA START 'p1';
INSERT INTO t1 VALUES (4, 4);
UPDATE t1 SET b = 20 WHERE a = 2;
DELETE FROM t1 WHERE a = 3;
XA END 'p1';
XA PREPARE 'p1';
SHOW WARNINGS;
--source include/kill_and_restart_mysqld.inc
XA RECOVER;
SET SESSION rocksdb_lock_wait_timeout = 1;
--error ER_LOCK_WAIT_TIMEOUT
INSERT INTO t1 VALUES (4, 40);
--error ER_LOCK_WAIT_TIMEOUT
UPDATE t1 SET b = 200 WHERE a = 2;
--error ER_LOCK_WAIT_TIMEOUT
DELETE FROM t1 WHERE a = 3;
--echo # Rows the transaction did not touch are not locked.
UPDATE t1 SET b = 10 WHERE a = 1;
XA COMMIT 'p1';
SELECT * FROM t1 ORDER BY a;
SELECT * FROM t1 FORCE INDEX (kb) ORDER BY b;
--echo # The locks are released by XA COMMIT.
UPDATE t1 SET b = 21 WHERE a = 2;

--echo # Locks that are not row writes survive a kill too: the unique
--echo # secondary key value and a locking read.
CREATE TABLE t2 (id INT PRIMARY KEY, u INT, UNIQUE KEY ku (u)) ENGINE=ROCKSDB;
INSERT INTO t2 VALUES (1, 1);
XA START 'u1';
INSERT INTO t2 VALUES (2, 10);
SELECT * FROM t2 WHERE id = 5 FOR UPDATE;
XA END 'u1';
XA PREPARE 'u1';
--source include/kill_and_restart_mysqld.inc
XA RECOVER;
SET SESSION rocksdb_lock_wait_timeout = 1;
--error ER_LOCK_WAIT_TIMEOUT
INSERT INTO t2 VALUES (3, 10);
--error ER_LOCK_WAIT_TIMEOUT
INSERT INTO t2 VALUES (5, 50);
INSERT INTO t2 VALUES (4, 40);
XA COMMIT 'u1';
--error ER_DUP_ENTRY
INSERT INTO t2 VALUES (3, 10);
INSERT INTO t2 VALUES (5, 50);
SELECT * FROM t2 ORDER BY id;
DROP TABLE t2;

--echo # A prepared transaction keeps its row locks after a kill and is
--echo # rolled back.
XA START 'p2';
INSERT INTO t1 VALUES (5, 5);
DELETE FROM t1 WHERE a = 1;
XA END 'p2';
XA PREPARE 'p2';
--source include/kill_and_restart_mysqld.inc
XA RECOVER;
--error ER_LOCK_WAIT_TIMEOUT
DELETE FROM t1 WHERE a = 1;
XA ROLLBACK 'p2';
SELECT * FROM t1 ORDER BY a;
DELETE FROM t1 WHERE a = 1;
INSERT INTO t1 VALUES (1, 1);

--echo # The prepared transaction survives two kills; before the second one
--echo # its XA PREPARE is no longer in the last binary log.
XA START 'p3';
INSERT INTO t1 VALUES (6, 6);
XA END 'p3';
XA PREPARE 'p3';
if ($xa_with_binlog)
{
  FLUSH BINARY LOGS;
}
--source include/kill_and_restart_mysqld.inc
XA RECOVER;
--source include/kill_and_restart_mysqld.inc
XA RECOVER;
XA COMMIT 'p3';
SELECT * FROM t1 ORDER BY a;

--echo # Several prepared transactions from several sessions, detached and
--echo # not; a committed XID is reused.
XA START 'p1';
INSERT INTO t1 VALUES (7, 7);
XA END 'p1';
XA PREPARE 'p1';
--connect (con1,localhost,root,,test)
SET SESSION xa_detach_on_prepare = OFF;
XA START 'p5';
UPDATE t1 SET b = 40 WHERE a = 4;
XA END 'p5';
XA PREPARE 'p5';
--connection default
XA START 'p6';
DELETE FROM t1 WHERE a = 6;
XA END 'p6';
XA PREPARE 'p6';
XA COMMIT 'p6';
--source include/kill_and_restart_mysqld.inc
--disconnect con1
--sorted_result
XA RECOVER;
XA COMMIT 'p1';
XA ROLLBACK 'p5';
--source include/kill_and_restart_mysqld.inc
XA RECOVER;
SELECT * FROM t1 ORDER BY a;

DROP TABLE t1;
