# InnoDB transparent tablespace data encryption
# This test case will verify that log encryption type can't be changed

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
--let $assert_only_after=CURRENT_TEST:innodb.percona_log_encrypt_change_$LOG_ENCRYPT_TEST_END
--source include/assert_grep.inc

# Test: using a different parameter during restart

--let $restart_parameters = restart: $KEYRING_PARAMS --innodb-redo_log_encrypt=$LOG_ENCRYPT_OTHER_TYPE
--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR $KEYRING_PLUGIN_OPT --plugin-dir=KEYRING_PLUGIN_PATH
--replace_regex /\.dll/.so/
--source include/restart_mysqld_no_echo.inc

SELECT @@innodb_redo_log_encrypt;

--let $assert_text=Check that there is a warning in the error log
--let $assert_select=Redo log encryption mode can't be switched without stopping the server and recreating the redo logs
--let $assert_file=$MYSQLTEST_VARDIR/log/mysqld.1.err
--let $assert_count=3
--let $assert_only_after=CURRENT_TEST:innodb.percona_log_encrypt_change_$LOG_ENCRYPT_TEST_END
--source include/assert_grep.inc

# Cleanup
--eval SET GLOBAL innodb_redo_log_encrypt=$LOG_ENCRYPT_TYPE
