# InnoDB transparent encryption on redo log.
# This test case will test basic encryption support features.
--source include/no_valgrind_without_big.inc
--source include/have_innodb_max_16k.inc

call mtr.add_suppression("ibd can't be decrypted, please confirm the keyfile is match and keyring plugin is loaded.");
call mtr.add_suppression("Encryption can't find master key, please check the keyring plugin is loaded.");
call mtr.add_suppression("Redo log cannot be encrypted if the keyring plugin is not loaded.");
call mtr.add_suppression("Check keyring plugin fail, please check the keyring plugin is loaded.");
call mtr.add_suppression("Redo log key generation failed.");

let $old_innodb_redo_log_encrypt = `SELECT @@innodb_redo_log_encrypt`;
<<<<<<< HEAD:mysql-test/include/log_encrypt_3.inc
||||||| ea7d2e2d16a:mysql-test/suite/innodb/t/log_encrypt_3.test

--disable_warnings
DROP DATABASE IF EXISTS tde_db;
=======

>>>>>>> mysql-8.0.20:mysql-test/suite/innodb/t/log_encrypt_3.test
CREATE DATABASE tde_db;
USE tde_db;
<<<<<<< HEAD:mysql-test/include/log_encrypt_3.inc

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
||||||| ea7d2e2d16a:mysql-test/suite/innodb/t/log_encrypt_3.test
--enable_warnings
# Server is not started with keyring plugin and run innodb_redo_log_encrypt=ON
# Expected Result : Variable is ignored.
# bug : expect warning or error
SET GLOBAL innodb_redo_log_encrypt = 1;

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


--disable_warnings
DROP DATABASE IF EXISTS tde_db;
CREATE DATABASE tde_db;
USE tde_db;
--enable_warnings


# Start server with --early-plugin-load and --innodb_redo_log_encrypt=ON .
=======

>>>>>>> mysql-8.0.20:mysql-test/suite/innodb/t/log_encrypt_3.test
# Explicitly run , INSTALL PLUGIN keyring_file SONAME 'keyring_file.so'
# Expected : plugin already installed
<<<<<<< HEAD:mysql-test/include/log_encrypt_3.inc
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
||||||| ea7d2e2d16a:mysql-test/suite/innodb/t/log_encrypt_3.test

--echo # Starting server with keyring plugin
let $restart_parameters = restart: --early-plugin-load="keyring_file=$KEYRING_PLUGIN" --loose-keyring_file_data=$MYSQL_TMP_DIR/my_key_log3 $KEYRING_PLUGIN_OPT --innodb_redo_log_encrypt=ON ;
--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR $KEYRING_PLUGIN_OPT --plugin-dir=KEYRING_PLUGIN_PATH $KEYRING_PLUGIN keyring_file.so
--source include/restart_mysqld.inc
SELECT @@global.innodb_redo_log_encrypt ;
--replace_regex /\.dll/.so/
--error ER_UDF_EXISTS
eval INSTALL PLUGIN keyring_file SONAME '$KEYRING_PLUGIN';
UNINSTALL PLUGIN keyring_file;

--echo # Starting server with keyring plugin
let $restart_parameters = restart: --early-plugin-load="keyring_file=$KEYRING_PLUGIN" --loose-keyring_file_data=$MYSQL_TMP_DIR/my_key_log3 $KEYRING_PLUGIN_OPT --innodb_redo_log_encrypt=ON ;
--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR $KEYRING_PLUGIN_OPT --plugin-dir=KEYRING_PLUGIN_PATH $KEYRING_PLUGIN keyring_file.so
--source include/restart_mysqld.inc
SET GLOBAL innodb_redo_log_encrypt = 0;
SELECT @@global.innodb_redo_log_encrypt ;
=======

SELECT @@global.innodb_redo_log_encrypt ;
>>>>>>> mysql-8.0.20:mysql-test/suite/innodb/t/log_encrypt_3.test
--replace_regex /\.dll/.so/
--error ER_UDF_EXISTS
eval INSTALL PLUGIN keyring_file SONAME '$KEYRING_PLUGIN';
UNINSTALL PLUGIN keyring_file;

# Flush logs
#  (closes and reopens any flushable logs for installed storage engines.
#  This causes InnoDB to flush its logs to disk. )
<<<<<<< HEAD:mysql-test/include/log_encrypt_3.inc
let $restart_parameters = restart: --early-plugin-load="keyring_file=$KEYRING_PLUGIN" --loose-keyring_file_data=$MYSQL_TMP_DIR/my_key_log3 $KEYRING_PLUGIN_OPT --innodb_redo_log_encrypt=$redo_log_encrypt_mode;
||||||| ea7d2e2d16a:mysql-test/suite/innodb/t/log_encrypt_3.test
let $restart_parameters = restart: --early-plugin-load="keyring_file=$KEYRING_PLUGIN" --loose-keyring_file_data=$MYSQL_TMP_DIR/my_key_log3 $KEYRING_PLUGIN_OPT --innodb_redo_log_encrypt=ON  ;
=======
--let $restart_parameters = restart:
>>>>>>> mysql-8.0.20:mysql-test/suite/innodb/t/log_encrypt_3.test
--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR $KEYRING_PLUGIN_OPT --plugin-dir=KEYRING_PLUGIN_PATH $KEYRING_PLUGIN keyring_file.so
<<<<<<< HEAD:mysql-test/include/log_encrypt_3.inc
--source include/restart_mysqld_no_echo.inc

SELECT @@global.innodb_redo_log_encrypt;
eval SET GLOBAL innodb_redo_log_encrypt = $redo_log_encrypt_mode;
SELECT @@global.innodb_redo_log_encrypt;

||||||| ea7d2e2d16a:mysql-test/suite/innodb/t/log_encrypt_3.test
--source include/restart_mysqld.inc
SELECT @@global.innodb_redo_log_encrypt ;
SET GLOBAL innodb_redo_log_encrypt = 1;
SELECT @@global.innodb_redo_log_encrypt ;
=======
--replace_regex /\.dll/.so/
--source include/restart_mysqld.inc

SELECT @@global.innodb_redo_log_encrypt ;
SET GLOBAL innodb_redo_log_encrypt = 1;
SELECT @@global.innodb_redo_log_encrypt ;
>>>>>>> mysql-8.0.20:mysql-test/suite/innodb/t/log_encrypt_3.test
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
<<<<<<< HEAD:mysql-test/include/log_encrypt_3.inc
||||||| ea7d2e2d16a:mysql-test/suite/innodb/t/log_encrypt_3.test
let $restart_parameters = restart: --early-plugin-load="keyring_file=$KEYRING_PLUGIN" --loose-keyring_file_data=$MYSQL_TMP_DIR/my_key_log3 $KEYRING_PLUGIN_OPT --innodb_redo_log_encrypt=ON  ;
#--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR $KEYRING_PLUGIN_OPT --plugin-dir=KEYRING_PLUGIN_PATH $KEYRING_PLUGIN keyring_file.so
#--source include/kill_and_restart_mysqld.inc
--sleep 3
--let $_server_id= `SELECT @@server_id`
--let $_expect_file_name= $MYSQLTEST_VARDIR/tmp/mysqld.$_server_id.expect
--exec echo "$restart_parameters" > $_expect_file_name
--shutdown_server 0
--source include/wait_until_disconnected.inc
--enable_reconnect
--source include/wait_until_connected_again.inc
--disable_reconnect
=======
--let $restart_parameters = restart:
--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR $KEYRING_PLUGIN_OPT --plugin-dir=KEYRING_PLUGIN_PATH $KEYRING_PLUGIN keyring_file.so
--replace_regex /\.dll/.so/
--source include/restart_mysqld.inc
>>>>>>> mysql-8.0.20:mysql-test/suite/innodb/t/log_encrypt_3.test

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
<<<<<<< HEAD:mysql-test/include/log_encrypt_3.inc
||||||| ea7d2e2d16a:mysql-test/suite/innodb/t/log_encrypt_3.test
let $restart_parameters = restart: --early-plugin-load="keyring_file=$KEYRING_PLUGIN" --loose-keyring_file_data=$MYSQL_TMP_DIR/my_key_log3 $KEYRING_PLUGIN_OPT --innodb_redo_log_encrypt=ON  ;
#--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR $KEYRING_PLUGIN_OPT --plugin-dir=KEYRING_PLUGIN_PATH $KEYRING_PLUGIN keyring_file.so
#--source include/kill_and_restart_mysqld.inc
--sleep 3
--let $_server_id= `SELECT @@server_id`
--let $_expect_file_name= $MYSQLTEST_VARDIR/tmp/mysqld.$_server_id.expect
--exec echo "$restart_parameters" > $_expect_file_name
--shutdown_server 0
--source include/wait_until_disconnected.inc
--enable_reconnect
--source include/wait_until_connected_again.inc
--disable_reconnect
=======
--let $restart_parameters = restart:
--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR $KEYRING_PLUGIN_OPT --plugin-dir=KEYRING_PLUGIN_PATH $KEYRING_PLUGIN keyring_file.so
--replace_regex /\.dll/.so/
--source include/restart_mysqld.inc
>>>>>>> mysql-8.0.20:mysql-test/suite/innodb/t/log_encrypt_3.test

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
<<<<<<< HEAD:mysql-test/include/log_encrypt_3.inc
UNINSTALL PLUGIN keyring_file;
||||||| ea7d2e2d16a:mysql-test/suite/innodb/t/log_encrypt_3.test


UNINSTALL PLUGIN keyring_file;
=======

>>>>>>> mysql-8.0.20:mysql-test/suite/innodb/t/log_encrypt_3.test
DROP TABLE tde_db.t1,tde_db.t2,tde_db.t3,tde_db.t4;

# Cleanup
--disable_query_log
eval SET GLOBAL innodb_redo_log_encrypt=$old_innodb_redo_log_encrypt;
DROP USER encryptnonprivuser@localhost;
DROP DATABASE tde_db;
--enable_query_log
