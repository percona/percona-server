########################################################################
# Binlog-related backup locks tests
########################################################################

--source include/have_innodb.inc
--source include/not_embedded.inc
--source include/have_archive.inc
--source include/have_csv.inc
--source include/have_blackhole.inc
--source include/have_log_bin.inc

SELECT @@binlog_format;

RESET MASTER;

SHOW VARIABLES LIKE 'have_backup_locks';

--echo #-----------------------------------------------------------------------
--echo # Setup
--echo #-----------------------------------------------------------------------

# Save the initial number of concurrent sessions.
--source include/count_sessions.inc

CREATE TABLE t_innodb(a INT NOT NULL, KEY (a)) ENGINE=InnoDB;
CREATE TABLE t_myisam(a INT NOT NULL, KEY (a)) ENGINE=MyISAM;
CREATE TABLE t_memory(a INT NOT NULL, KEY (a)) ENGINE=MEMORY;
CREATE TABLE t_csv(a INT NOT NULL) ENGINE=CSV;
CREATE TABLE t_blackhole(a INT NOT NULL, KEY (a)) ENGINE=BLACKHOLE;
CREATE TABLE t_archive(a INT NOT NULL) ENGINE=ARCHIVE;

CREATE VIEW v_innodb AS SELECT * FROM t_innodb;
CREATE VIEW v_myisam AS SELECT * FROM t_myisam;
CREATE VIEW v_memory AS SELECT * FROM t_memory;
CREATE VIEW v_csv AS SELECT * FROM t_csv;
CREATE VIEW v_blackhole AS SELECT * FROM t_blackhole;
CREATE VIEW v_archive AS SELECT * FROM t_archive;

--echo #-----------------------------------------------------------------------
--echo # Single-threaded tests
--echo #-----------------------------------------------------------------------

#
# LBFB + DML/DDL in the same connection
#

LOCK BINLOG FOR BACKUP;

DELETE FROM t_innodb;
INSERT INTO t_innodb VALUES(0);
UPDATE t_innodb SET a = 1;
REPLACE INTO t_innodb VALUES(1);
SELECT * from t_innodb;
HANDLER t_innodb OPEN;
HANDLER t_innodb READ a FIRST;
HANDLER t_innodb CLOSE;

DELETE FROM t_myisam;
INSERT INTO t_myisam VALUES(0);
UPDATE t_myisam SET a = 1;
REPLACE INTO t_myisam VALUES(1);
SELECT * from t_myisam;
HANDLER t_myisam OPEN;
HANDLER t_myisam READ a FIRST;
HANDLER t_myisam CLOSE;

DELETE FROM t_memory;
INSERT INTO t_memory VALUES(0);
UPDATE t_memory SET a = 1;
REPLACE INTO t_memory VALUES(1);
SELECT * from t_memory;

DELETE FROM t_archive;
INSERT INTO t_archive VALUES(0);
SELECT * from t_archive;

DELETE FROM t_csv;
INSERT INTO t_csv VALUES(0);
UPDATE t_csv SET a = 1;
REPLACE INTO t_csv VALUES(1);
SELECT * from t_csv;

DELETE FROM t_blackhole;
INSERT INTO t_blackhole VALUES(0);
UPDATE t_blackhole SET a = 1;
REPLACE INTO t_blackhole VALUES(1);
SELECT * from t_blackhole;

CREATE TABLE tt_innodb(a INT NOT NULL) ENGINE=InnoDB;
CREATE TABLE tt_myisam(a INT NOT NULL) ENGINE=MyISAM;
CREATE TABLE tt_memory(a INT NOT NULL) ENGINE=MEMORY;
CREATE TABLE tt_csv(a INT NOT NULL) ENGINE=CSV;
CREATE TABLE tt_blackhole(a INT NOT NULL) ENGINE=BLACKHOLE;
CREATE TABLE tt_archive(a INT NOT NULL) ENGINE=ARCHIVE;

DROP TABLE tt_innodb;
DROP TABLE tt_myisam;
DROP TABLE tt_memory;
DROP TABLE tt_csv;
DROP TABLE tt_blackhole;
DROP TABLE tt_archive;

--error ER_BAD_TABLE_ERROR
DROP TABLE non_existing;

TRUNCATE TABLE t_innodb;
TRUNCATE TABLE t_myisam;
TRUNCATE TABLE t_memory;
TRUNCATE TABLE t_csv;
TRUNCATE TABLE t_blackhole;

RENAME TABLE t_innodb TO tmp, tmp TO t_innodb;
RENAME TABLE t_myisam TO tmp, tmp TO t_myisam;
RENAME TABLE t_memory TO tmp, tmp TO t_memory;
RENAME TABLE t_csv TO tmp, tmp TO t_csv;
RENAME TABLE t_blackhole TO tmp, tmp TO t_blackhole;
RENAME TABLE t_archive TO tmp, tmp TO t_archive;

ALTER TABLE t_innodb ADD COLUMN b CHAR(10) NOT NULL;
ALTER TABLE t_innodb DROP COLUMN b;
ALTER TABLE t_myisam ADD COLUMN b CHAR(10) NOT NULL;
ALTER TABLE t_myisam DROP COLUMN b;
ALTER TABLE t_memory ADD COLUMN b CHAR(10) NOT NULL;
ALTER TABLE t_memory DROP COLUMN b;
ALTER TABLE t_csv ADD COLUMN b CHAR(10) NOT NULL;
ALTER TABLE t_csv DROP COLUMN b;
ALTER TABLE t_blackhole ADD COLUMN b CHAR(10) NOT NULL;
ALTER TABLE t_blackhole DROP COLUMN b;
ALTER TABLE t_archive ADD COLUMN b CHAR(10) NOT NULL;
ALTER TABLE t_archive DROP COLUMN b;

ALTER TABLE t_innodb ADD KEY tmp (a);
ALTER TABLE t_innodb DROP KEY tmp;
ALTER TABLE t_myisam ADD KEY tmp (a);
ALTER TABLE t_myisam DROP KEY tmp;
ALTER TABLE t_memory ADD KEY tmp (a);
ALTER TABLE t_memory DROP KEY tmp;
ALTER TABLE t_blackhole ADD KEY tmp (a);
ALTER TABLE t_blackhole DROP KEY tmp;

CREATE DATABASE test1;
DROP DATABASE test1;

delimiter |;

CREATE PROCEDURE p1()
BEGIN
  SELECT 1;
END|

DROP PROCEDURE p1|

CREATE FUNCTION f1() RETURNS INT DETERMINISTIC
BEGIN
  RETURN 1;
END|

DROP FUNCTION f1|

delimiter ;|

CREATE VIEW v1 AS SELECT * FROM t_innodb;
DROP VIEW v1;

INSERT INTO v_innodb VALUES(1);
INSERT INTO v_blackhole VALUES(1);
INSERT INTO v_myisam VALUES(1);
INSERT INTO v_csv VALUES(1);
INSERT INTO v_memory VALUES(1);
INSERT INTO v_archive VALUES(1);

UNLOCK BINLOG;

#
# Test incompatibility with delay_key_write=ALL
#
SELECT @@delay_key_write;

SET GLOBAL delay_key_write=ALL;
--error ER_OPTION_PREVENTS_STATEMENT
LOCK TABLES FOR BACKUP;

SET GLOBAL delay_key_write=default;
SELECT @@delay_key_write;

#
# read-only=1 + backup locks
#

SET GLOBAL read_only=1;

INSERT INTO t_innodb VALUES(0);
INSERT INTO t_myisam VALUES(0);

LOCK TABLES FOR BACKUP;

INSERT INTO t_innodb VALUES(0);
--error ER_CANT_EXECUTE_WITH_BACKUP_LOCK
INSERT INTO t_myisam VALUES(0);

UNLOCK TABLES;

INSERT INTO t_innodb VALUES(0);
INSERT INTO t_myisam VALUES(0);

SET GLOBAL read_only=0;

#
# Test that temporary tables are not affected by backup locks
#
LOCK TABLES FOR BACKUP;
CREATE TEMPORARY TABLE tt_innodb(a INT NOT NULL) ENGINE=InnoDB;
CREATE TEMPORARY TABLE tt_myisam(a INT NOT NULL) ENGINE=MyISAM;
CREATE TEMPORARY TABLE tt_memory(a INT NOT NULL) ENGINE=MEMORY;
CREATE TEMPORARY TABLE tt_csv(a INT NOT NULL) ENGINE=CSV;
CREATE TEMPORARY TABLE tt_blackhole(a INT NOT NULL) ENGINE=BLACKHOLE;
CREATE TEMPORARY TABLE tt_archive(a INT NOT NULL) ENGINE=ARCHIVE;

INSERT INTO tt_innodb VALUES(5);
INSERT INTO tt_myisam VALUES(5);
INSERT INTO tt_memory VALUES(5);
INSERT INTO tt_csv VALUES(5);
INSERT INTO tt_blackhole VALUES(5);
INSERT INTO tt_archive VALUES(5);

LOCK BINLOG FOR BACKUP;

START TRANSACTION;
SELECT * FROM tt_archive;
SELECT * FROM tt_blackhole;
SELECT * FROM tt_memory;
SELECT * FROM tt_innodb;
SELECT * FROM tt_myisam;
INSERT INTO tt_innodb VALUES(6);
INSERT INTO tt_myisam VALUES(6);
INSERT INTO tt_memory VALUES(6);
INSERT INTO tt_csv VALUES(6);
INSERT INTO tt_blackhole VALUES(6);
INSERT INTO tt_archive VALUES(6);
COMMIT;

SELECT * FROM tt_archive;
SELECT * FROM tt_blackhole;
SELECT * FROM tt_memory;
SELECT * FROM tt_innodb;
SELECT * FROM tt_myisam;

DROP TEMPORARY TABLE tt_innodb;
DROP TEMPORARY TABLE tt_myisam;
DROP TEMPORARY TABLE tt_memory;
DROP TEMPORARY TABLE tt_csv;
DROP TEMPORARY TABLE tt_blackhole;
DROP TEMPORARY TABLE tt_archive;

UNLOCK BINLOG;

UNLOCK TABLES;

#
# test that AUTOCOMMIT updates succeed with a BINLOG lock in the same connection with binlog disabled
#

SELECT @@log_bin;

LOCK BINLOG FOR BACKUP;

INSERT INTO t_innodb VALUES(1);
INSERT INTO t_myisam VALUES(1);

UNLOCK BINLOG;

#
# Log tables
#

SET @old_general_log = @@general_log;
SET @old_slow_query_log = @@slow_query_log;
SET @old_log_output = @@log_output;
SET @old_long_query_time = @@SESSION.long_query_time;

SET SESSION long_query_time = 0;
SET GLOBAL log_output = 'TABLE';
SET GLOBAL general_log = ON;
SET GLOBAL slow_query_log = ON;

LOCK TABLES FOR BACKUP;

SELECT 1;

LOCK BINLOG FOR BACKUP;

SELECT 1;

UNLOCK BINLOG;

UNLOCK TABLES;

SET SESSION long_query_time = @old_long_query_time;
SET GLOBAL log_output = @old_log_output;
SET GLOBAL slow_query_log = @old_slow_query_log;
SET GLOBAL general_log = @old_general_log;

--echo #-----------------------------------------------------------------------
--echo # Multi-threaded tests
--echo #-----------------------------------------------------------------------

--echo # connection default

CREATE USER user@localhost;
GRANT ALL PRIVILEGES ON test.* TO user@localhost;

--connect(con1,localhost,root,,)

--connect(con2,localhost,user,,)

--connection default
LOCK TABLES FOR BACKUP;

--connection con1
--echo # connection con1

SET SESSION lock_wait_timeout = 1;

--error ER_LOCK_WAIT_TIMEOUT
SET GLOBAL delay_key_write=ALL;

SET SESSION lock_wait_timeout = DEFAULT;

--connection default
--echo # connection default
UNLOCK TABLES;

#
# read-only=1 + backup locks
#

--connection con2
--echo # connection con2

START TRANSACTION;
INSERT INTO t_innodb VALUES(0);

--connection default
--echo # connection default

SET GLOBAL read_only=1;

--connection con2
--echo # connection con2

--error ER_OPTION_PREVENTS_STATEMENT
COMMIT;

--error ER_OPTION_PREVENTS_STATEMENT
INSERT INTO t_innodb VALUES(0);
--error ER_OPTION_PREVENTS_STATEMENT
INSERT INTO t_myisam VALUES(0);

--connection con1
--echo # connection con1

LOCK TABLES FOR BACKUP;

--connection con2
--echo # connection con2

--error ER_OPTION_PREVENTS_STATEMENT
INSERT INTO t_innodb VALUES(0);
--error ER_OPTION_PREVENTS_STATEMENT
INSERT INTO t_myisam VALUES(0);

--connection default
--echo # connection default

SET GLOBAL read_only=0;

--connection con2
--echo # connection con2

SET SESSION lock_wait_timeout = 1;

INSERT INTO t_innodb VALUES(0);
--error ER_LOCK_WAIT_TIMEOUT
INSERT INTO t_myisam VALUES(0);

SET SESSION lock_wait_timeout = 0;

--connection default
--echo # connection default

SET GLOBAL read_only=1;

--connection con2
--echo # connection con2

--error ER_OPTION_PREVENTS_STATEMENT
INSERT INTO t_innodb VALUES(0);
--error ER_OPTION_PREVENTS_STATEMENT
INSERT INTO t_myisam VALUES(0);

--connection con1
--echo # connection con1

UNLOCK TABLES;

--connection con2
--echo # connection con2

--error ER_OPTION_PREVENTS_STATEMENT
INSERT INTO t_innodb VALUES(0);
--error ER_OPTION_PREVENTS_STATEMENT
INSERT INTO t_myisam VALUES(0);


--connection default
--echo # connection default

SET GLOBAL read_only=0;

#
# Test that temporary tables are not affected by tables lock, but updates that
# would write into the binary log are blocked by the binlog lock.
#
--connection default
--echo # connection default
LOCK TABLES FOR BACKUP;

--connection con1
--echo # connection con1

CREATE TEMPORARY TABLE tt_innodb(a INT NOT NULL) ENGINE=InnoDB;
CREATE TEMPORARY TABLE tt_myisam(a INT NOT NULL) ENGINE=MyISAM;
CREATE TEMPORARY TABLE tt_memory(a INT NOT NULL) ENGINE=MEMORY;
CREATE TEMPORARY TABLE tt_csv(a INT NOT NULL) ENGINE=CSV;
CREATE TEMPORARY TABLE tt_blackhole(a INT NOT NULL) ENGINE=BLACKHOLE;
CREATE TEMPORARY TABLE tt_archive(a INT NOT NULL) ENGINE=ARCHIVE;

INSERT INTO tt_innodb VALUES(5);
INSERT INTO tt_myisam VALUES(5);
INSERT INTO tt_memory VALUES(5);
INSERT INTO tt_csv VALUES(5);
INSERT INTO tt_blackhole VALUES(5);
INSERT INTO tt_archive VALUES(5);

--connection default
--echo # connection default
LOCK BINLOG FOR BACKUP;

--let $orig_binlog_pos= query_get_value(SHOW MASTER STATUS, Position, 1)

--connection con1
--echo # connection con1

SET SESSION lock_wait_timeout = 1;

START TRANSACTION;
if (`SELECT @@binlog_format = 'STATEMENT'`)
{
   --error ER_LOCK_WAIT_TIMEOUT
   INSERT INTO tt_myisam VALUES(6);
}
if (!`SELECT @@binlog_format = 'STATEMENT'`)
{
  INSERT INTO tt_myisam VALUES(6);
}

# Updates to temporary tables are not binlogged in the ROW mode
INSERT INTO tt_innodb VALUES(6);
SELECT * FROM tt_archive;
SELECT * FROM tt_blackhole;
SELECT * FROM tt_memory;
SELECT * FROM tt_innodb;
SELECT * FROM tt_myisam;
SELECT * FROM tt_csv;
if (`SELECT @@binlog_format = 'STATEMENT'`)
{
   --error ER_LOCK_WAIT_TIMEOUT
   COMMIT;
if (!`SELECT @@binlog_format = 'STATEMENT'`)
{
   COMMIT;
}
}

--connection default
--echo # connection default

--let $binlog_pos= query_get_value(SHOW MASTER STATUS, Position, 1)
--disable_query_log
--eval SELECT $orig_binlog_pos = $binlog_pos AS must_be_1
--enable_query_log

UNLOCK BINLOG;
UNLOCK TABLES;

--connection con1
--echo # connection con1

COMMIT;

DROP TEMPORARY TABLE tt_innodb;
DROP TEMPORARY TABLE tt_myisam;
DROP TEMPORARY TABLE tt_memory;
DROP TEMPORARY TABLE tt_csv;
DROP TEMPORARY TABLE tt_blackhole;
DROP TEMPORARY TABLE tt_archive;

--connection default
--echo # connection default

LOCK BINLOG FOR BACKUP;

--let $orig_binlog_pos= query_get_value(SHOW MASTER STATUS, Position, 1)

--connection con1
--echo # connection con1

#
# test that AUTOCOMMIT updates are blocked with a BINLOG lock in another
# connection with binlog enabled
#

SET SESSION lock_wait_timeout = 1;

--error ER_LOCK_WAIT_TIMEOUT
INSERT INTO t_innodb VALUES(1);
--error ER_LOCK_WAIT_TIMEOUT
INSERT INTO t_myisam VALUES(1);

SET SESSION lock_wait_timeout = DEFAULT;

--connection default
--echo # connection default

--let $binlog_pos= query_get_value(SHOW MASTER STATUS, Position, 1)
--disable_query_log
--eval SELECT $orig_binlog_pos = $binlog_pos AS must_be_1
--enable_query_log

UNLOCK BINLOG;

#
# LTFB + DDL / DML in another connection
#

--connection default
--echo # connection default

LOCK BINLOG FOR BACKUP;

--let $orig_binlog_pos= query_get_value(SHOW MASTER STATUS, Position, 1)

--connection con1
--echo # connection con1

SET SESSION lock_wait_timeout = 1;

--error ER_LOCK_WAIT_TIMEOUT
DELETE FROM t_innodb;
--error ER_LOCK_WAIT_TIMEOUT
INSERT INTO t_innodb VALUES(0);
--error ER_LOCK_WAIT_TIMEOUT
UPDATE t_innodb SET a = 1;
--error ER_LOCK_WAIT_TIMEOUT
REPLACE INTO t_innodb VALUES(1);
SELECT * from t_innodb;
HANDLER t_innodb OPEN;
HANDLER t_innodb READ a FIRST;
HANDLER t_innodb CLOSE;

START TRANSACTION;
SELECT * from t_innodb;
DELETE FROM t_innodb;
INSERT INTO t_innodb VALUES(0);
UPDATE t_innodb SET a = 1;
REPLACE INTO t_innodb VALUES(1);
--error ER_LOCK_WAIT_TIMEOUT
COMMIT;

SELECT * FROM t_innodb;
COMMIT;

START TRANSACTION;
--error ER_LOCK_WAIT_TIMEOUT
INSERT INTO t_myisam VALUES(1);
COMMIT;

--error ER_LOCK_WAIT_TIMEOUT
INSERT INTO t_myisam VALUES(0);
--error ER_LOCK_WAIT_TIMEOUT
UPDATE t_myisam SET a = 1;
SELECT * FROM t_myisam;
HANDLER t_myisam OPEN;
HANDLER t_myisam READ a FIRST;
HANDLER t_myisam CLOSE;

# Blackhole updates are not binlogged in the ROW mode
if (`SELECT @@binlog_format = 'ROW'`)
{
  DELETE FROM t_blackhole;
}
if (!`SELECT @@binlog_format = 'ROW'`)
{
--error ER_LOCK_WAIT_TIMEOUT
  DELETE FROM t_blackhole;
}
--error ER_LOCK_WAIT_TIMEOUT
INSERT INTO t_blackhole VALUES(0);
# Blackhole updates are not binlogged in the ROW mode
if (`SELECT @@binlog_format = 'ROW'`)
{
  UPDATE t_blackhole SET a = 1;
}
if (!`SELECT @@binlog_format = 'ROW'`)
{
  --error ER_LOCK_WAIT_TIMEOUT
  UPDATE t_blackhole SET a = 1;
}
--error ER_LOCK_WAIT_TIMEOUT
REPLACE INTO t_blackhole VALUES(1);
SELECT * FROM t_blackhole;

--error ER_LOCK_WAIT_TIMEOUT
CREATE TABLE tt_innodb(a INT NOT NULL) ENGINE=InnoDB;

--error ER_LOCK_WAIT_TIMEOUT
DROP TABLE tt_innodb;

--error ER_BAD_TABLE_ERROR
DROP TABLE non_existing;

--error ER_TABLE_EXISTS_ERROR
CREATE TABLE t_innodb(a INT NOT NULL) ENGINE=InnoDB;

--error ER_LOCK_WAIT_TIMEOUT
TRUNCATE TABLE t_innodb;

--error ER_LOCK_WAIT_TIMEOUT
RENAME TABLE t_innodb TO tmp, tmp to t_innodb;

--error ER_LOCK_WAIT_TIMEOUT
ALTER TABLE t_innodb ADD COLUMN b CHAR(10);

--error ER_LOCK_WAIT_TIMEOUT
ALTER TABLE t_innodb ADD KEY (a);

--error ER_LOCK_WAIT_TIMEOUT
CREATE DATABASE test1;

--error ER_LOCK_WAIT_TIMEOUT
DROP DATABASE test1;

--error ER_LOCK_WAIT_TIMEOUT
CREATE PROCEDURE p1() SELECT 1;

--error ER_LOCK_WAIT_TIMEOUT
DROP PROCEDURE p1;

--error ER_LOCK_WAIT_TIMEOUT
CREATE VIEW v1 AS SELECT * FROM t_innodb;

--error ER_LOCK_WAIT_TIMEOUT
DROP VIEW v1;

--error ER_LOCK_WAIT_TIMEOUT
INSERT INTO v_innodb VALUES(1);

--connection default
--echo # connection default

--let $binlog_pos= query_get_value(SHOW MASTER STATUS, Position, 1)
--disable_query_log
--eval SELECT $orig_binlog_pos = $binlog_pos AS must_be_1
--enable_query_log

UNLOCK BINLOG;

--connection con1
--echo # connection con1

# DDLs are not transactional, so previous DDL statements against t_innodb will
# be executed despite the error
SHOW CREATE TABLE t_innodb;

DROP TABLE t_innodb;
CREATE TABLE t_innodb(a INT NOT NULL) ENGINE=InnoDB;

--connection default
--echo # connection default

#
# INSERT DELAYED
#

LOCK BINLOG FOR BACKUP;

--let $orig_binlog_pos= query_get_value(SHOW MASTER STATUS, Position, 1)

--connection con1
--echo # connection con1

if (`SELECT @@binlog_format = 'STATEMENT'`)
{
   --error ER_LOCK_WAIT_TIMEOUT
   INSERT DELAYED INTO t_myisam VALUES(0);
}
if (!`SELECT @@binlog_format = 'STATEMENT'`)
{
   INSERT DELAYED INTO t_myisam VALUES(0);
}

--connection default
--echo # connection default

if (!`SELECT @@binlog_format = 'STATEMENT'`)
{
  let $wait_condition=
      SELECT COUNT(*) = 1 FROM INFORMATION_SCHEMA.PROCESSLIST
      WHERE STATE = "Waiting for binlog lock" AND
      COMMAND = "Delayed insert";
  --source include/wait_condition.inc
}
  
--let $binlog_pos= query_get_value(SHOW MASTER STATUS, Position, 1)
--disable_query_log
--eval SELECT $orig_binlog_pos = $binlog_pos AS must_be_1
--enable_query_log

UNLOCK BINLOG;

if (!`SELECT @@binlog_format = 'STATEMENT'`)
{
  let $wait_condition=
      SELECT COUNT(*) = 0 FROM INFORMATION_SCHEMA.PROCESSLIST
      WHERE STATE = "Waiting for binlog lock" AND
      COMMAND = "Delayed";
  --source include/wait_condition.inc
}

SELECT * FROm t_myisam;


#
# Log tables
#

SET @old_general_log = @@general_log;
SET @old_slow_query_log = @@slow_query_log;
SET @old_log_output = @@log_output;

SET GLOBAL log_output = 'TABLE';
SET GLOBAL general_log = ON;
SET GLOBAL slow_query_log = ON;

LOCK BINLOG FOR BACKUP;

--let $orig_binlog_pos= query_get_value(SHOW MASTER STATUS, Position, 1)

--connection con1
--echo # connection con1

SET @old_long_query_time = @@SESSION.long_query_time;
SET SESSION long_query_time = 0;

SELECT 1;

--connection default
--echo # connection default

SELECT 1;

--connection con1
--echo # connection con1

SET SESSION long_query_time = @old_long_query_time;

--connection default
--echo # connection default

--let $binlog_pos= query_get_value(SHOW MASTER STATUS, Position, 1)
--disable_query_log
--eval SELECT $orig_binlog_pos = $binlog_pos AS must_be_1
--enable_query_log

UNLOCK BINLOG;

SET GLOBAL log_output = @old_log_output;
SET GLOBAL slow_query_log = @old_slow_query_log;
SET GLOBAL general_log = @old_general_log;

--echo #-----------------------------------------------------------------------
--echo # Cleanup
--echo #-----------------------------------------------------------------------

DROP USER user@localhost;

--disconnect con1
--disconnect con2

--source include/wait_until_count_sessions.inc

DROP VIEW v_innodb, v_myisam, v_memory, v_csv, v_blackhole, v_archive;
DROP TABLE t_innodb, t_myisam, t_memory, t_csv, t_blackhole, t_archive;
