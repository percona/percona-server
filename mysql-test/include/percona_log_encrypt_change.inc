# InnoDB transparent tablespace data encryption
# This test case will verify that log encryption type can't be changed

--source include/have_innodb.inc
--source include/not_embedded.inc

call mtr.add_suppression("Redo log encryption mode can't be switched without stopping the server and recreating the redo logs");

# Test: changing when it is explicitly set

SELECT @@innodb_redo_log_encrypt;
--eval SET GLOBAL innodb_redo_log_encrypt=$LOG_ENCRYPT_OTHER_TYPE
SELECT @@innodb_redo_log_encrypt;

# Test: changing when it is turned off, but the logs ARE encrypted

--eval SET GLOBAL innodb_redo_log_encrypt=OFF
--eval SET GLOBAL innodb_redo_log_encrypt=$LOG_ENCRYPT_OTHER_TYPE
SELECT @@innodb_redo_log_encrypt;

--let $assert_text=Check that there is a warning in the error log
--let $assert_select=Redo log encryption mode can't be switched without stopping the server and recreating the redo logs
--let $assert_file=$MYSQLTEST_VARDIR/log/mysqld.1.err
--let $assert_count=2
--let $assert_only_after=CURRENT_TEST: innodb.percona_log_encrypt_change_$LOG_ENCRYPT_TEST_END
--source include/assert_grep.inc

# Test: using a different parameter during restart

--let MYSQLD_DATADIR = `SELECT @@datadir`
--let MYSQLD_LOG = $MYSQL_TMP_DIR/server.log
--source include/shutdown_mysqld.inc

--let NEW_CMD = $MYSQLD $extra_args $KEYRING_PARAMS --datadir=$MYSQLD_DATADIR --innodb_redo_log_encrypt=$LOG_ENCRYPT_OTHER_TYPE >$MYSQLD_LOG 2>&1;
--error 1
--exec $NEW_CMD

# Test: crash the server AND restart with a different parameter
--source include/start_mysqld_no_echo.inc

--let $assert_text = Check that there is a warning in the error log
--let $assert_select = Redo log encryption mode can't be switched without stopping the server and recreating the redo logs
--let $assert_file = $MYSQLD_LOG
--let $assert_count = 1
--source include/assert_grep.inc

--remove_file $MYSQLD_LOG

CREATE TABLE t1(c1 INT, c2 char(20)) ENCRYPTION="Y" ENGINE = InnoDB;
INSERT INTO t1 VALUES(0, "aaaaa");
INSERT INTO t1 VALUES(1, "bbbbb");
INSERT INTO t1 VALUES(2, "ccccc");

--source include/kill_mysqld.inc

--let NEW_CMD = $MYSQLD $extra_args $KEYRING_PARAMS --datadir=$MYSQLD_DATADIR --innodb_redo_log_encrypt=$LOG_ENCRYPT_OTHER_TYPE >$MYSQLD_LOG 2>&1;
--error 1
--exec $NEW_CMD

--source include/start_mysqld_no_echo.inc

--let $assert_text=Check that there is a warning in the error log
--let $assert_select=Redo log encryption mode can't be switched without stopping the server and recreating the redo logs
--let $assert_file=$MYSQLD_LOG
--let $assert_count=1
--let $assert_only_after=CURRENT_TEST: innodb.percona_log_encrypt_change_$LOG_ENCRYPT_TEST_END
--source include/assert_grep.inc

# Cleanup
DROP TABLE t1;
--eval SET GLOBAL innodb_redo_log_encrypt=$LOG_ENCRYPT_TYPE
--remove_file $MYSQL_TMP_DIR/mysecret_keyring_change
--remove_file $MYSQLD_LOG

