# InnoDB transparent encryption on redo log.
# This test case will check innodb_redo_log_encrypt=ON/OFF with bootstrap
# and start server by setting innodb_redo_log_encrypt=ON/OFF

--source include/no_valgrind_without_big.inc
--source include/have_innodb.inc
--source include/not_embedded.inc
--source include/have_innodb_max_16k.inc

let $old_innodb_file_per_table = `SELECT @@innodb_file_per_table`;
let $old_innodb_redo_log_encrypt = `SELECT @@innodb_redo_log_encrypt`;
let $START_PAGE_SIZE= `select @@innodb_page_size`;
let $LOG_FILE_SIZE= `select @@innodb_log_file_size`;
let $MYSQLD_BASEDIR= `select @@basedir`;

let MYSQLD_LOG=$MYSQL_TMP_DIR/server.log;
let extra_args=--no-defaults --console --log-syslog=0 --loose-skip-auto_generate_certs --loose-skip-sha256_password_auto_generate_rsa_keys --skip-ssl --basedir=$BASEDIR --lc-messages-dir=$MYSQL_SHAREDIR;

--mkdir $MYSQL_TMP_DIR/log_encrypt_dir1_$redo_log_mode
--mkdir $MYSQL_TMP_DIR/log_encrypt_dir2_$redo_log_mode
--mkdir $MYSQL_TMP_DIR/log_encrypt_dir3_$redo_log_mode

let $MYSQLD_DATADIR1 = $MYSQL_TMP_DIR/log_encrypt_dir1_$redo_log_mode;
let $MYSQLD_DATADIR2 = $MYSQL_TMP_DIR/log_encrypt_dir2_$redo_log_mode;
let $MYSQLD_DATADIR3 = $MYSQL_TMP_DIR/log_encrypt_dir3_$redo_log_mode;

let BOOTSTRAP_SQL=$MYSQL_TMP_DIR/boot.sql;

--echo # create bootstrap file
write_file $BOOTSTRAP_SQL;
CREATE DATABASE test;
EOF
--echo # Stop the MTR default DB server
--source include/shutdown_mysqld.inc

let NEW_CMD = $MYSQLD $extra_args --initialize-insecure $KEYRING_PARAMS --innodb_log_file_size=$LOG_FILE_SIZE --innodb_page_size=$START_PAGE_SIZE --datadir=$MYSQLD_DATADIR1 --init-file=$BOOTSTRAP_SQL --secure-file-priv="" --innodb_redo_log_encrypt=$redo_log_mode --innodb_undo_tablespaces=2 >>$MYSQLD_LOG 2>&1;

--echo # Run the bootstrap command of datadir1
--exec $NEW_CMD
--sleep 5

--echo # Start the DB server with datadir1
--replace_result $MYSQL_TMP_DIR TMP_DIR $KEYRING_PLUGIN_OPT --plugin-dir=KEYRING_PLUGIN_PATH $MYSQLD_DATADIR1 DATADIR1 $START_PAGE_SIZE PAGE_SIZE $LOG_FILE_SIZE LOG_FILE_SIZE
--let $restart_parameters="restart: $KEYRING_PARAMS --innodb_redo_log_encrypt=$redo_log_mode --innodb_undo_tablespaces=2 --datadir=$MYSQLD_DATADIR1 --innodb_page_size=$START_PAGE_SIZE --innodb_log_file_size=$LOG_FILE_SIZE"
--replace_regex /\.dll/.so/
--source include/start_mysqld_no_echo.inc


SELECT @@global.innodb_redo_log_encrypt;

USE test;
CREATE TABLE tab1(c1 INT, c2 VARCHAR(30));
INSERT INTO tab1 VALUES(1, 'Test consistency undo*');
SELECT * FROM tab1;
CREATE TABLE tab2(c1 INT, c2 VARCHAR(30)) ENCRYPTION="Y";
INSERT INTO tab2 VALUES(1, 'Test consistency undo*');
SELECT * FROM tab2;
DROP TABLE tab1,tab2;

--let $restart_parameters= restart:
--source include/restart_mysqld.inc

--force-rmdir $MYSQL_TMP_DIR/log_encrypt_dir1_$redo_log_mode
#------------------------------------------------------------------------------


--echo # Stop the MTR default DB server
--source include/shutdown_mysqld.inc

#bootstrap should fail if keyring plugin is not loaded along with
#innodb_redo_log_encrypt=ON
let NEW_CMD = $MYSQLD $extra_args --initialize-insecure --datadir=$MYSQLD_DATADIR2 --init-file=$BOOTSTRAP_SQL  --secure-file-priv="" --innodb_redo_log_encrypt=$redo_log_mode >>$MYSQLD_LOG 2>&1;

--echo # Run the bootstrap command of datadir2, it should fail since the keyring is not loaded.
--error 1
--exec $NEW_CMD
--sleep 10

--force-rmdir $MYSQL_TMP_DIR/log_encrypt_dir2_$redo_log_mode

let NEW_CMD = $MYSQLD $extra_args --initialize-insecure $KEYRING_PARAMS --innodb_log_file_size=$LOG_FILE_SIZE --innodb_page_size=$START_PAGE_SIZE --datadir=$MYSQLD_DATADIR2 --init-file=$BOOTSTRAP_SQL --secure-file-priv="" --innodb_redo_log_encrypt=$redo_log_mode --innodb_undo_tablespaces=2 >>$MYSQLD_LOG 2>&1;

--echo # Run the bootstrap command of datadir2
--exec $NEW_CMD
--sleep 10

--echo # Start the DB server with datadir2
--replace_result $MYSQL_TMP_DIR TMP_DIR $KEYRING_PLUGIN_OPT --plugin-dir=KEYRING_PLUGIN_PATH $MYSQLD_DATADIR2 DATADIR2 $START_PAGE_SIZE PAGE_SIZE $LOG_FILE_SIZE LOG_FILE_SIZE
--let $restart_parameters="restart: $KEYRING_PARAMS --innodb_redo_log_encrypt=$redo_log_mode --innodb_undo_tablespaces=2 --datadir=$MYSQLD_DATADIR2 --innodb_page_size=$START_PAGE_SIZE --innodb_log_file_size=$LOG_FILE_SIZE"
--replace_regex /\.dll/.so/
--source include/start_mysqld_no_echo.inc

SELECT @@global.innodb_redo_log_encrypt;

USE test;
CREATE TABLE tab1(c1 INT, c2 VARCHAR(30));
INSERT INTO tab1 VALUES(1, 'Test consistency undo*');
SELECT * FROM tab1;
CREATE TABLE tab2(c1 INT, c2 VARCHAR(30)) ENCRYPTION="Y";
INSERT INTO tab2 VALUES(1, 'Test consistency undo*');
SELECT * FROM tab2;
DROP TABLE tab1,tab2;

--let $restart_parameters= restart:
--source include/restart_mysqld.inc

--force-rmdir $MYSQL_TMP_DIR/log_encrypt_dir2_$redo_log_mode

#------------------------------------------------------------------------------
--echo # Stop the MTR default DB server
--source include/shutdown_mysqld.inc

let NEW_CMD = $MYSQLD $extra_args --initialize-insecure --innodb_log_file_size=$LOG_FILE_SIZE --innodb_page_size=$START_PAGE_SIZE --datadir=$MYSQLD_DATADIR3 --init-file=$BOOTSTRAP_SQL --secure-file-priv="" --innodb_redo_log_encrypt=$redo_log_mode --innodb_undo_tablespaces=2 $KEYRING_PARAMS >>$MYSQLD_LOG 2>&1;

--echo # Run the bootstrap command of datadir3
--exec $NEW_CMD
--sleep 10

--echo # Start the DB server with datadir3 and keyring loaded.
--replace_result $MYSQL_TMP_DIR TMP_DIR $KEYRING_PLUGIN_OPT --plugin-dir=KEYRING_PLUGIN_PATH $MYSQLD_DATADIR3 DATADIR3 $START_PAGE_SIZE PAGE_SIZE $LOG_FILE_SIZE LOG_FILE_SIZE
--let $restart_parameters="restart: $KEYRING_PARAMS --innodb_undo_tablespaces=2 --datadir=$MYSQLD_DATADIR3 --innodb_page_size=$START_PAGE_SIZE --innodb_log_file_size=$LOG_FILE_SIZE"
--replace_regex /\.dll/.so/
--source include/start_mysqld_no_echo.inc

SELECT @@global.innodb_redo_log_encrypt;

USE test;
CREATE TABLE tab1(c1 INT, c2 VARCHAR(30));
INSERT INTO tab1 VALUES(1, 'Test consistency undo*');
SELECT * FROM tab1;
CREATE TABLE tab2(c1 INT, c2 VARCHAR(30)) ENCRYPTION="Y";
DROP TABLE tab1;

--let $restart_parameters= restart:
--source include/restart_mysqld.inc

--force-rmdir $MYSQL_TMP_DIR/log_encrypt_dir3_$redo_log_mode
--remove_file $BOOTSTRAP_SQL

#cleanup
--disable_query_log
eval SET GLOBAL innodb_file_per_table=$old_innodb_file_per_table;
eval SET GLOBAL innodb_redo_log_encrypt=$old_innodb_redo_log_encrypt;
--enable_query_log
