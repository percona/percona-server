--echo #
--echo # Tests for kill_idle_transaction feature that do not depend on transactional SEs
--echo #

--disable_query_log
call mtr.add_suppression("IO-layer timeout before wait_timeout was reached");
--enable_query_log

CREATE TABLE t1 (a INT) ENGINE=MyISAM;

let $saved_kill_idle_transaction= `SELECT @@kill_idle_transaction`;
SET GLOBAL kill_idle_transaction= 1;

--echo # Autocommitting non-trx SE queries should not result in kills
INSERT INTO t1 VALUES (1);
SELECT * FROM t1;

sleep 2;

--echo # Should not have been disconnected
INSERT INTO t1 VALUES (2);

SELECT * FROM t1;

--echo # An idle transaction should be killed even if it does not use any transactional SE
BEGIN;
sleep 2;

--echo # Current connection idle transaction killed, reconnecting
--source include/wait_until_disconnected.inc
--source include/wait_until_connected_again.inc

--echo # With a low wait_timeout and high kill_idle_transaction the lower value takes precedence
SET GLOBAL kill_idle_transaction= 100000;
SET SESSION wait_timeout= 5;

BEGIN;
sleep 6;
--echo # Current connection idle transaction killed, reconnecting
--source include/wait_until_disconnected.inc
--source include/wait_until_connected_again.inc

--echo # Cleanup
DROP TABLE t1;
--replace_result $saved_kill_idle_transaction saved_kill_idle_transaction
eval SET GLOBAL kill_idle_transaction= $saved_kill_idle_transaction;
