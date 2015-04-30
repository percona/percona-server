# Bug lp:1438990
# "SAVEPOINT", "ROLLBACK TO savepoint" wipe out table map
# on slave during execution binary log events. For trigger
# the map is written to binary log once for all trigger body
# and if trigger contains "SAVEPOINT" or "ROLLBACK TO savepoint"
# statements any trigger's events after these statements will not
# have table map.
#
# Run it for all types of replication to be sure
# the fix did not break anything.
#

CREATE TABLE t1 (f1 INTEGER PRIMARY KEY);
CREATE TABLE t2 (f1 INTEGER PRIMARY KEY);
CREATE TABLE t3 (f1 INTEGER PRIMARY KEY);

DELIMITER |;

CREATE TRIGGER tr1 AFTER INSERT ON t1 FOR EACH ROW
BEGIN
        DECLARE EXIT HANDLER FOR SQLEXCEPTION
                BEGIN
                        ROLLBACK TO event_logging_1;
                        INSERT t3 VALUES (1);
                END;       
        
        SAVEPOINT event_logging_1;

        INSERT INTO t2 VALUES (1);

        RELEASE SAVEPOINT event_logging_1;

END|
DELIMITER ;|

INSERT INTO t2 VALUES (1);
INSERT INTO t1 VALUES (1);

--source include/show_binlog_events.inc

--sync_slave_with_master

--connection master

DROP TRIGGER tr1;
DELETE FROM t1;
DELETE FROM t2;
DELETE FROM t3;

DELIMITER |;

CREATE PROCEDURE p1()
BEGIN
        DECLARE EXIT HANDLER FOR SQLEXCEPTION
                BEGIN
                        ROLLBACK TO event_logging_2;
                        INSERT t3 VALUES (3);
                END;

        SAVEPOINT event_logging_2;

        INSERT INTO t2 VALUES (1);

        RELEASE SAVEPOINT event_logging_2;
END|

CREATE TRIGGER tr1 AFTER INSERT ON t1 FOR EACH ROW CALL p1()|

DELIMITER ;|

INSERT INTO t2 VALUES (1);
INSERT INTO t1 VALUES (1);

--source include/show_binlog_events.inc

--sync_slave_with_master

--connection master

DROP TABLE t1;
DROP TABLE t2;
DROP TABLE t3;

DROP PROCEDURE p1;
--source include/rpl_end.inc
