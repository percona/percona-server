# InnoDB transparent tablespace data encryption
# This test case will verify that no unencrypted data is in the logs

--source include/no_valgrind_without_big.inc

# Test: command line parameter

--let ABORT_ON=FOUND
--source include/percona_log_encrypt_content_test.inc

# Test: variable

# Restart the server with keyring loaded
--let restart_parameters="restart:$KEYRING_PARAMS"
--source include/restart_mysqld_no_echo.inc

--eval SET GLOBAL innodb_redo_log_encrypt=$LOG_ENCRYPT_TYPE

--source include/percona_log_encrypt_content_test.inc

# Restart the server with keyring loaded
--let restart_parameters="restart:$KEYRING_PARAMS"
--source include/restart_mysqld_no_echo.inc
SET GLOBAL innodb_redo_log_encrypt=OFF;

--let ABORT_ON=NOT_FOUND
--source include/percona_log_encrypt_content_test.inc

# Cleanup
--eval SET GLOBAL innodb_redo_log_encrypt=$LOG_ENCRYPT_TYPE
#--remove_file $MYSQL_TMP_DIR/mysecret_keyring_content

