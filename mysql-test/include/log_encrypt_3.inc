# InnoDB transparent encryption on redo log.
# This test case will test basic encryption support features.
--source include/no_valgrind_without_big.inc
--source include/have_innodb.inc
--source include/not_embedded.inc
--source include/have_innodb_max_16k.inc

--disable_query_log
call mtr.add_suppression("Encryption can't find master key, please check the keyring plugin is loaded.");
call mtr.add_suppression("Redo log cannot be encrypted, if the keyring plugin is not loaded.");
call mtr.add_suppression("keyring error: please check that a keyring plugin is loaded.");
call mtr.add_suppression("Redo log key generation failed.");
--enable_query_log

let $old_innodb_file_per_table = `SELECT @@innodb_file_per_table`;
let $old_innodb_redo_log_encrypt = `SELECT @@innodb_redo_log_encrypt`;
CREATE DATABASE tde_db;
USE tde_db;

# Server is not started with keyring plugin and run innodb_redo_log_encrypt=MASTER_KEY/ROTATED_KEY
# Expected Result : Variable is ignored.
# bug : expect warning or error
eval SET GLOBAL innodb_redo_log_encrypt = $redo_log_encrypt_mode;
SHOW WARNINGS;
--let $wait_condition=SELECT @@global.innodb_redo_log_encrypt = 0
--source include/wait_condition.inc
CREATE TABLE tde_db.t4 (a BIGINT PRIMARY KEY, b LONGBLOB) ENGINE=InnoDB;
INSERT INTO t4 (a, b) VALUES (1, REPEAT('a', 6*512*512));
SELECT a,LEFT(b,10) FROM tde_db.t4;

# Restart server
--source include/restart_mysqld.inc

SELECT a,LEFT(b,10) FROM tde_db.t4;
DROP TABLE tde_db.t4;
DROP DATABASE tde_db;
CREATE DATABASE tde_db;
USE tde_db;

# Start server with --early-plugin-load and --innodb_redo_log_encrypt=$redo_log_encrypt_mode .
# Explicitly run , INSTALL PLUGIN keyring_file SONAME 'keyring_file.so'
# Expected : plugin already installed
--echo # Starting server with keyring plugin
let $restart_parameters = restart: --early-plugin-load="keyring_file=$KEYRING_PLUGIN" --loose-keyring_file_data=$MYSQL_TMP_DIR/my_key_log3 $KEYRING_PLUGIN_OPT --innodb_redo_log_encrypt=$redo_log_encrypt_mode;
--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR $KEYRING_PLUGIN_OPT --plugin-dir=KEYRING_PLUGIN_PATH $KEYRING_PLUGIN keyring_file.so
--source include/restart_mysqld_no_echo.inc

SELECT @@global.innodb_redo_log_encrypt;

--replace_regex /\.dll/.so/
--error ER_UDF_EXISTS
eval INSTALL PLUGIN keyring_file SONAME '$KEYRING_PLUGIN';
UNINSTALL PLUGIN keyring_file;
--echo # Starting server with keyring plugin
let $restart_parameters = restart: --early-plugin-load="keyring_file=$KEYRING_PLUGIN" --loose-keyring_file_data=$MYSQL_TMP_DIR/my_key_log3 $KEYRING_PLUGIN_OPT --innodb_redo_log_encrypt=$redo_log_encrypt_mode;
--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR $KEYRING_PLUGIN_OPT --plugin-dir=KEYRING_PLUGIN_PATH $KEYRING_PLUGIN keyring_file.so
--source include/restart_mysqld_no_echo.inc

SET GLOBAL innodb_redo_log_encrypt = 0;
SELECT @@global.innodb_redo_log_encrypt;
--replace_regex /\.dll/.so/
--error ER_UDF_EXISTS
eval INSTALL PLUGIN keyring_file SONAME '$KEYRING_PLUGIN';
UNINSTALL PLUGIN keyring_file;

# Flush logs
#  (closes and reopens any flushable logs for installed storage engines.
#  This causes InnoDB to flush its logs to disk. )
let $restart_parameters = restart: --early-plugin-load="keyring_file=$KEYRING_PLUGIN" --loose-keyring_file_data=$MYSQL_TMP_DIR/my_key_log3 $KEYRING_PLUGIN_OPT --innodb_redo_log_encrypt=$redo_log_encrypt_mode;
--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR $KEYRING_PLUGIN_OPT --plugin-dir=KEYRING_PLUGIN_PATH $KEYRING_PLUGIN keyring_file.so
--source include/restart_mysqld_no_echo.inc

SELECT @@global.innodb_redo_log_encrypt;
eval SET GLOBAL innodb_redo_log_encrypt = $redo_log_encrypt_mode;
SELECT @@global.innodb_redo_log_encrypt;

CREATE TABLE tde_db.t1 (a BIGINT PRIMARY KEY, b LONGBLOB) ENGINE=InnoDB;
INSERT INTO t1 (a, b) VALUES (1, REPEAT('a', 6*512*512));
SELECT a,LEFT(b,10) FROM tde_db.t1;

CREATE TABLE tde_db.t2 (a BIGINT PRIMARY KEY, b LONGBLOB)
			ENCRYPTION='Y' ENGINE=InnoDB;
INSERT INTO t2 (a, b) VALUES (1, REPEAT('a', 6*512*512));
SELECT a,LEFT(b,10) FROM tde_db.t2;
SET GLOBAL innodb_redo_log_encrypt = 0;
SELECT @@global.innodb_redo_log_encrypt;

CREATE TABLE tde_db.t3 (a BIGINT PRIMARY KEY, b LONGBLOB) ENGINE=InnoDB;
INSERT INTO t3 (a, b) VALUES (1, REPEAT('a', 6*512*512));
SELECT a,LEFT(b,10) FROM tde_db.t3;

CREATE TABLE tde_db.t4 (a BIGINT PRIMARY KEY, b LONGBLOB)
			ENCRYPTION='Y' ENGINE=InnoDB;
INSERT INTO t4 (a, b) VALUES (1, REPEAT('a', 6*512*512));
SELECT a,LEFT(b,10) FROM tde_db.t4;
FLUSH LOGS;

let $restart_parameters = restart: --early-plugin-load="keyring_file=$KEYRING_PLUGIN" --loose-keyring_file_data=$MYSQL_TMP_DIR/my_key_log3 $KEYRING_PLUGIN_OPT --innodb_redo_log_encrypt=$redo_log_encrypt_mode;
let $restart_hide_args = 1
--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR $KEYRING_PLUGIN_OPT --plugin-dir=KEYRING_PLUGIN_PATH $KEYRING_PLUGIN keyring_file.so
--source include/kill_and_restart_mysqld.inc

SELECT
 PLUGIN_NAME, PLUGIN_STATUS, PLUGIN_TYPE
FROM INFORMATION_SCHEMA.PLUGINS
  WHERE PLUGIN_NAME LIKE '%keyring_file%';

SELECT a,LEFT(b,10) FROM tde_db.t1;
SELECT a,LEFT(b,10) FROM tde_db.t2;
SELECT a,LEFT(b,10) FROM tde_db.t3;
SELECT a,LEFT(b,10) FROM tde_db.t4;

DROP TABLE tde_db.t1,tde_db.t2,tde_db.t3,tde_db.t4;

# innodb_redo_log_encrypt=ON/OFF  with transaction
SELECT @@global.innodb_redo_log_encrypt;
CREATE TABLE tde_db.t1 (a BIGINT PRIMARY KEY, b LONGBLOB) ENGINE=InnoDB;
CREATE TABLE tde_db.t2 (a BIGINT PRIMARY KEY, b LONGBLOB)
			ENCRYPTION='Y' ENGINE=InnoDB;

START TRANSACTION;
eval SET GLOBAL innodb_redo_log_encrypt = $redo_log_encrypt_mode;
INSERT INTO t1 (a, b) VALUES (1, REPEAT('a', 6*512*512));
INSERT INTO t2 (a, b) VALUES (1, REPEAT('a', 6*512*512));
SELECT a,LEFT(b,10) FROM tde_db.t1;
SELECT a,LEFT(b,10) FROM tde_db.t2;
ROLLBACK;

START TRANSACTION;
INSERT INTO t1 (a, b) VALUES (2, REPEAT('a', 6*512*512));
INSERT INTO t2 (a, b) VALUES (2, REPEAT('a', 6*512*512));
SELECT a,LEFT(b,10) FROM tde_db.t1;
SELECT a,LEFT(b,10) FROM tde_db.t2;
COMMIT;

CREATE TABLE tde_db.t3 (a BIGINT PRIMARY KEY, b LONGBLOB) ENGINE=InnoDB;
CREATE TABLE tde_db.t4 (a BIGINT PRIMARY KEY, b LONGBLOB)
			ENCRYPTION='Y' ENGINE=InnoDB;

START TRANSACTION;
SET GLOBAL innodb_redo_log_encrypt = 0;
SELECT @@global.innodb_redo_log_encrypt;
INSERT INTO t3 (a, b) VALUES (1, REPEAT('a', 6*512*512));
INSERT INTO t4 (a, b) VALUES (1, REPEAT('a', 6*512*512));
SELECT a,LEFT(b,10) FROM tde_db.t3;
SELECT a,LEFT(b,10) FROM tde_db.t4;
ROLLBACK;

START TRANSACTION;
INSERT INTO t3 (a, b) VALUES (2, REPEAT('a', 6*512*512));
INSERT INTO t4 (a, b) VALUES (2, REPEAT('a', 6*512*512));
SELECT a,LEFT(b,10) FROM tde_db.t3;
SELECT a,LEFT(b,10) FROM tde_db.t4;
COMMIT;

let $restart_hide_args = 1
let $restart_parameters = restart: --early-plugin-load="keyring_file=$KEYRING_PLUGIN" --loose-keyring_file_data=$MYSQL_TMP_DIR/my_key_log3 $KEYRING_PLUGIN_OPT --innodb_redo_log_encrypt=$redo_log_encrypt_mode;
--source include/kill_and_restart_mysqld.inc

SELECT
  PLUGIN_NAME, PLUGIN_STATUS, PLUGIN_TYPE
FROM INFORMATION_SCHEMA.PLUGINS
  WHERE PLUGIN_NAME LIKE '%keyring_file%';

SELECT a,LEFT(b,10) FROM tde_db.t1;
SELECT a,LEFT(b,10) FROM tde_db.t2;
SELECT a,LEFT(b,10) FROM tde_db.t3;
SELECT a,LEFT(b,10) FROM tde_db.t4;

# Rotate master key when innodb_redo_log_encrypt=ON/OFF
SET GLOBAL innodb_redo_log_encrypt = 0;
SELECT @@global.innodb_redo_log_encrypt;
ALTER INSTANCE ROTATE INNODB MASTER KEY;
SELECT a,LEFT(b,10) FROM tde_db.t1;
SELECT a,LEFT(b,10) FROM tde_db.t2;
SELECT a,LEFT(b,10) FROM tde_db.t3;
SELECT a,LEFT(b,10) FROM tde_db.t4;

eval SET GLOBAL innodb_redo_log_encrypt = $redo_log_encrypt_mode;
SELECT @@global.innodb_redo_log_encrypt;
ALTER INSTANCE ROTATE INNODB MASTER KEY;
SELECT a,LEFT(b,10) FROM tde_db.t1;
SELECT a,LEFT(b,10) FROM tde_db.t2;
SELECT a,LEFT(b,10) FROM tde_db.t3;
SELECT a,LEFT(b,10) FROM tde_db.t4;

# Non-privileged user can not set innodb_redo_log_encrypt variable.
CREATE USER encryptnonprivuser@localhost IDENTIFIED BY 'noauth';
GRANT SELECT ON *.* to encryptnonprivuser@localhost;
FLUSH PRIVILEGES;

--echo # In connection 1 - with encryptnonprivuser
connect (con1,localhost,encryptnonprivuser,'noauth',);
SELECT @@global.innodb_redo_log_encrypt;
--error ER_SPECIFIC_ACCESS_DENIED_ERROR
SET GLOBAL innodb_redo_log_encrypt = 0;
SELECT @@global.innodb_redo_log_encrypt;
--error ER_SPECIFIC_ACCESS_DENIED_ERROR
eval SET GLOBAL innodb_redo_log_encrypt = $redo_log_encrypt_mode;
SELECT @@global.innodb_redo_log_encrypt;
disconnect con1;

--echo # In connection default
connection default;
UNINSTALL PLUGIN keyring_file;
DROP TABLE tde_db.t1,tde_db.t2,tde_db.t3,tde_db.t4;

# Cleanup
--disable_query_log
eval SET GLOBAL innodb_file_per_table=$old_innodb_file_per_table;
eval SET GLOBAL innodb_redo_log_encrypt=$old_innodb_redo_log_encrypt;
DROP DATABASE tde_db;
--enable_query_log
