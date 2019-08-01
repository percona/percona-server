# WL#9290 InnoDB: Support transparent tablespace data encryption for redo log
# This test case will test basic redo log encryption support features.

--source include/no_valgrind_without_big.inc
--source include/have_innodb_max_16k.inc

--disable_query_log
call mtr.add_suppression("Ignoring tablespace .* because it could not be opened");
call mtr.add_suppression("Encryption can't find master key, please check the keyring plugin is loaded.");
call mtr.add_suppression("Redo log cannot be encrypted, if the keyring plugin is not loaded.");
call mtr.add_suppression("keyring error: please check that a keyring plugin is loaded.");
call mtr.add_suppression("Redo log key generation failed.");
--enable_query_log

# Enable redo log encryption, should report error in server log, since keyring is not loaded.
eval SET GLOBAL innodb_redo_log_encrypt = $redo_log_mode;

# Create a table with encryption, should fail since keyring is not
# loaded.
--error ER_CANNOT_FIND_KEY_IN_KEYRING
CREATE TABLE t1(c1 INT, c2 char(20)) ENCRYPTION="Y" ENGINE = InnoDB;

CREATE TABLE t1(c1 INT, c2 char(20)) ENGINE = InnoDB;

--error ER_CANNOT_FIND_KEY_IN_KEYRING
ALTER TABLE t1 ENCRYPTION="Y", algorithm=copy;

let $restart_parameters = restart: $KEYRING_PARAMS --general-log --log-output=FILE --general_log_file=$MYSQL_TMP_DIR/keyring_query_log $KEYRING_PLUGIN_OPT;
--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR $KEYRING_PLUGIN_OPT --plugin-dir=KEYRING_PLUGIN_PATH
--replace_regex /\.dll/.so/
--source include/restart_mysqld_no_echo.inc

# Enable redo log encryption
eval SET GLOBAL innodb_redo_log_encrypt = $redo_log_mode;

SHOW CREATE TABLE t1;

INSERT INTO t1 VALUES(0, "aaaaa");
INSERT INTO t1 VALUES(1, "bbbbb");
INSERT INTO t1 VALUES(2, "ccccc");
INSERT INTO t1 VALUES(3, "ddddd");
INSERT INTO t1 VALUES(4, "eeeee");
INSERT INTO t1 VALUES(5, "fffff");
INSERT INTO t1 VALUES(6, "ggggg");
INSERT INTO t1 VALUES(7, "hhhhh");
INSERT INTO t1 VALUES(8, "iiiii");
INSERT INTO t1 VALUES(9, "jjjjj");
INSERT INTO t1 select * from t1;
INSERT INTO t1 select * from t1;
INSERT INTO t1 select * from t1;
INSERT INTO t1 select * from t1;
INSERT INTO t1 select * from t1;
INSERT INTO t1 select * from t1;

SELECT * FROM t1 ORDER BY c1 LIMIT 10;

# Restart to confirm the encryption info can be retrieved properly.
let $restart_parameters = restart: $KEYRING_PARAMS --general-log --log-output=FILE --general_log_file=$MYSQL_TMP_DIR/keyring_query_log --innodb_redo_log_encrypt=$redo_log_mode;
--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR $KEYRING_PLUGIN_OPT --plugin-dir=KEYRING_PLUGIN_PATH
--replace_regex /\.dll/.so/
--source include/restart_mysqld_no_echo.inc

SELECT * FROM t1 ORDER BY c1 LIMIT 10;

DROP TABLE t1;

# Crash/recovery test.
CREATE TABLE t1(c1 INT, c2 char(20)) ENGINE = InnoDB;

INSERT INTO t1 VALUES(0, "aaaaa");
INSERT INTO t1 VALUES(1, "bbbbb");
INSERT INTO t1 VALUES(2, "ccccc");
INSERT INTO t1 VALUES(3, "ddddd");
INSERT INTO t1 VALUES(4, "eeeee");
INSERT INTO t1 VALUES(5, "fffff");
INSERT INTO t1 VALUES(6, "ggggg");
INSERT INTO t1 VALUES(7, "hhhhh");
INSERT INTO t1 VALUES(8, "iiiii");
INSERT INTO t1 VALUES(9, "jjjjj");

# Restart to confirm the encryption info can be retrieved properly.
--let $restart_parameters = restart: $KEYRING_PARAMS
--source include/restart_mysqld_no_echo.inc

SELECT * FROM t1 ORDER BY c1 LIMIT 10;
DELETE FROM t1;

START TRANSACTION;
INSERT INTO t1 VALUES(0, "aaaaa");
INSERT INTO t1 VALUES(1, "bbbbb");
INSERT INTO t1 VALUES(2, "ccccc");
INSERT INTO t1 VALUES(3, "ddddd");
INSERT INTO t1 VALUES(4, "eeeee");
INSERT INTO t1 VALUES(5, "fffff");
INSERT INTO t1 VALUES(6, "ggggg");
INSERT INTO t1 VALUES(7, "hhhhh");
INSERT INTO t1 VALUES(8, "iiiii");
INSERT INTO t1 VALUES(9, "jjjjj");

# Restart to confirm the encryption info can be retrieved properly.
let $restart_parameters = restart: $KEYRING_PARAMS --general-log --log-output=FILE --general_log_file=$MYSQL_TMP_DIR/keyring_query_log --innodb_redo_log_encrypt=$redo_log_mode;
--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR $KEYRING_PLUGIN_OPT --plugin-dir=KEYRING_PLUGIN_PATH
--replace_regex /\.dll/.so/
--source include/restart_mysqld_no_echo.inc

SELECT * FROM t1 ORDER BY c1 LIMIT 10;

INSERT INTO t1 VALUES(0, "aaaaa");
INSERT INTO t1 VALUES(1, "bbbbb");
INSERT INTO t1 VALUES(2, "ccccc");
INSERT INTO t1 VALUES(3, "ddddd");
INSERT INTO t1 VALUES(4, "eeeee");
INSERT INTO t1 VALUES(5, "fffff");
INSERT INTO t1 VALUES(6, "ggggg");
INSERT INTO t1 VALUES(7, "hhhhh");
INSERT INTO t1 VALUES(8, "iiiii");
INSERT INTO t1 VALUES(9, "jjjjj");

SELECT * FROM t1 ORDER BY c1 LIMIT 10;

# Cleanup
DROP TABLE t1;
