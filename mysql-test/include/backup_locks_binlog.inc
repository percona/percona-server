########################################################################
# Binlog-related backup locks tests
########################################################################

--source include/have_archive.inc
--source include/have_blackhole.inc
--source include/have_log_bin.inc
--source include/have_myisam.inc

SELECT @@binlog_format;

RESET BINARY LOGS AND GTIDS;

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

UNLOCK TABLES;

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

UNLOCK TABLES;

SET SESSION long_query_time = @old_long_query_time;
SET GLOBAL log_output = @old_log_output;
SET GLOBAL slow_query_log = @old_slow_query_log;
SET GLOBAL general_log = @old_general_log;
TRUNCATE TABLE mysql.slow_log;
TRUNCATE TABLE mysql.general_log;

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
# Test that temporary tables are not affected by tables lock
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

UNLOCK TABLES;

--connection con1
--echo # connection con1

DROP TEMPORARY TABLE tt_innodb;
DROP TEMPORARY TABLE tt_myisam;
DROP TEMPORARY TABLE tt_memory;
DROP TEMPORARY TABLE tt_csv;
DROP TEMPORARY TABLE tt_blackhole;
DROP TEMPORARY TABLE tt_archive;

--connection default
--echo # connection default

--echo #-----------------------------------------------------------------------
--echo # Cleanup
--echo #-----------------------------------------------------------------------

DROP USER user@localhost;

--disconnect con1
--disconnect con2

--source include/wait_until_count_sessions.inc

DROP VIEW v_innodb, v_myisam, v_memory, v_csv, v_blackhole, v_archive;
DROP TABLE t_innodb, t_myisam, t_memory, t_csv, t_blackhole, t_archive;
