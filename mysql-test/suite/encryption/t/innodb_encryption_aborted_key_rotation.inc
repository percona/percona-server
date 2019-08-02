--source include/have_debug.inc
--source include/have_debug_sync.inc
--source include/not_valgrind.inc

# This test is to check if tables get rotated correctly with new version of encryption
# key in case of server crash
# 1) Encrypt t1 with version 1 of key 0
# 2) Rotate key 0 to version 2
# 1) Rotate 100 pages in t1
# 2) Shut down the sever - this will make sure the t1's pages are flushed to disk
#    However it will still simulated the server crash, as only 100 pages will get rotated
#    and the updated to page0 of t1 will not be done
# 3) Restart the server
# 4) Make sure t1 rotation completes

SET GLOBAL innodb_encryption_threads=4;

let MYSQLD_DATADIR = `SELECT @@datadir`;

--let $id = `SELECT RAND()`
--let t1_IBD = $MYSQLD_DATADIR/test/t1.ibd

# Disabling warnings as punch hole get generate ones when compression is enabled
# and filesystem is not supporting them
--disable_warnings
eval CREATE TABLE t1 (id INT NOT NULL PRIMARY KEY, a VARCHAR(255)) ENGINE=InnoDB $t1_table_type ENCRYPTION='KEYRING';
--enable_warnings

delimiter //;
create procedure innodb_insert_proc (repeat_count int)
begin
  declare current_num int;
  set current_num = 0;
  while current_num < repeat_count do
    insert into t1 values (current_num,repeat('foobar',42));
    set current_num = current_num + 1;
  end while;
end//
delimiter ;//
commit;

set autocommit=0;
call innodb_insert_proc(30000);
commit;
set autocommit=1;

--let $assert_text= Make sure t1 is encrypted
--let $assert_cond= "[SELECT FLAG & 8192 FROM INFORMATION_SCHEMA.INNODB_TABLESPACES WHERE NAME=\\'test/t1\\']" = 8192
--source include/assert.inc

--echo # Wait for all tables to get encrypted
--let tables_count=`select count(*) from INFORMATION_SCHEMA.INNODB_TABLESPACES`
--let $wait_timeout= 600
# Encrypted tables count should be equal tables_count - 1 because temporary tablespace is not encrypted
--let $wait_condition=SELECT COUNT(*) = $tables_count - 1 FROM INFORMATION_SCHEMA.INNODB_TABLESPACES_ENCRYPTION WHERE MIN_KEY_VERSION = 1
--source include/wait_condition.inc

--echo # We want only first 100 pages to be rotated
SET GLOBAL debug="+d,rotate_only_first_100_pages_from_t1";

--let $assert_text= Make sure re-encryption will happen per every key rotation
--let $assert_cond= "[SELECT @@GLOBAL.innodb_encryption_rotate_key_age = 1]" = 1
--source include/assert.inc

--let $assert_text= Successful rotation of percona_innodb-0 to version 2
--let $assert_cond= "[SELECT rotate_system_key\\(\\"percona_innodb-0\\"\\) = 1]" = 1
--source include/assert.inc

--echo # Table t1 should have min_key_version = 1 assigned and ROTATIONG_OR_FLUSHING=1 <= this means that only 100 pages
--echo # have been rotatted.
--let $wait_timeout= 600
--let $wait_condition=SELECT name = 'test/t1' FROM INFORMATION_SCHEMA.INNODB_TABLESPACES_ENCRYPTION WHERE MIN_KEY_VERSION = 1 AND ROTATING_OR_FLUSHING = 1
--source include/wait_condition.inc

--echo # Disable rotation threads
SET GLOBAL innodb_encryption_threads = 0;
--echo # Disable rotation of only first 100 pages
SET GLOBAL debug="-d,rotate_only_first_100_pages_from_t1";

--source include/shutdown_mysqld.inc

--echo # Make sure that t1 is still encrypted
--let SEARCH_PATTERN=foobar
--let ABORT_ON=FOUND
--let SEARCH_FILE=$t1_IBD
--source include/search_pattern_in_file.inc

--echo # t1 is only half rotatted, now we will check if the encryption can be completed after the restart
--let $restart_parameters=restart:--innodb-encryption-threads=4
--source include/start_mysqld.inc

--let $wait_timeout= 600
--echo # All tables should get encrypted. tables_count - 1 because temporary tablespace is not encrypted
--let $wait_condition=SELECT COUNT(*) = $tables_count - 1 FROM INFORMATION_SCHEMA.INNODB_TABLESPACES_ENCRYPTION WHERE MIN_KEY_VERSION = 2
--source include/wait_condition.inc

--echo # Make sure t1 is encrypted
--let SEARCH_PATTERN=foobar
--let ABORT_ON=FOUND
--let SEARCH_FILE=$t1_IBD
--source include/search_pattern_in_file.inc

SET GLOBAL default_table_encryption=ONLINE_FROM_KEYRING_TO_UNENCRYPTED;

--let $wait_timeout= 600
--echo # All tables should get decrypted, apart from t1.
--let $wait_condition=SELECT COUNT(*) = 1 FROM INFORMATION_SCHEMA.INNODB_TABLESPACES_ENCRYPTION WHERE MIN_KEY_VERSION <> 0
--source include/wait_condition.inc

#Cleanup

SET GLOBAL innodb_encryption_threads=0;
SET GLOBAL default_table_encryption=ONLINE_TO_KEYRING;

DROP TABLE t1;
DROP PROCEDURE innodb_insert_proc;
