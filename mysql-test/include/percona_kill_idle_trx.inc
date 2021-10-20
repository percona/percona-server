--source include/count_sessions.inc
--let $default_se= `SELECT @@default_storage_engine`

--echo #
--echo # Test kill_idle_transaction_timeout feature with $default_se
--echo #

CREATE TABLE t1 (a INT);

let $saved_kill_idle_transaction= `SELECT @@kill_idle_transaction`;
SET GLOBAL kill_idle_transaction= 1;

BEGIN;
INSERT INTO t1 VALUES (1),(2);
COMMIT;
SELECT * FROM t1;

BEGIN;
INSERT INTO t1 VALUES (3);
sleep 2;

--echo # Current connection idle transaction killed, reconnecting
--source include/wait_until_disconnected.inc
--enable_reconnect
--source include/wait_until_connected_again.inc
--disable_reconnect
SELECT * FROM t1;

--echo #
--echo # Test that row locks are released on idle transaction kill
--echo #

SET GLOBAL kill_idle_transaction= 2;

--connect (conn1,localhost,root,,)
--connection conn1

--echo # Take row locks in connection conn1
BEGIN;
SELECT * FROM t1 FOR UPDATE;

--connection default

sleep 3;
--echo # Take row locks in connection default
UPDATE t1 SET a=4;
SELECT * FROM t1;

--connection conn1

--echo # Show that connection conn1 has been killed
--error ER_CLIENT_INTERACTION_TIMEOUT
SELECT * FROM t1;

--disconnect conn1
--connection default
--echo # connection default

--echo # Cleanup
DROP TABLE t1;
--replace_result $saved_kill_idle_transaction saved_kill_idle_transaction
eval SET GLOBAL kill_idle_transaction= $saved_kill_idle_transaction;
--source include/wait_until_count_sessions.inc
