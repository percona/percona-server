--source include/have_debug.inc
--source include/have_debug_sync.inc
--source include/not_valgrind.inc

call mtr.add_suppression("Attempting backtrace");

# This test is to check if tables get rotated correctly in case of server crash
# 1) Rotate 100 pages in t1
# 2) Shut down the sever - this will make sure the t1's pages are flushed to disk
#    However it will still simulated the server crash, as only 100 pages will get rotated
#    and the updated to page0 of t1 will not be done
# 3) Restart the server
# 4) Make sure t1 rotation completes

--echo # First we try rotation unencrypted => encrypted

let MYSQLD_DATADIR = `SELECT @@datadir`;

--let $id = `SELECT RAND()`
--let t1_IBD = $MYSQLD_DATADIR/test/t1.ibd

eval CREATE TABLE t1 (id INT NOT NULL PRIMARY KEY, a VARCHAR(255)) ENGINE=InnoDB $t1_table_type;

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

# Make sure encryption is disabled
--let $assert_text= Make sure encryption is disabled
--let $assert_cond= "[SELECT @@GLOBAL.default_table_encryption]" = 0
--source include/assert.inc

if ($is_mk_encrypted)
{
  --let $assert_text= Make sure t1 is encrypted
  --let $assert_cond= "[SELECT FLAG & 8192 FROM INFORMATION_SCHEMA.INNODB_TABLESPACES WHERE NAME=\\'test/t1\\']" = 8192
}
if (!$is_mk_encrypted)
{
  --let $assert_text= Make sure t1 is unencrypted
  --let $assert_cond= "[SELECT FLAG & 8192 FROM INFORMATION_SCHEMA.INNODB_TABLESPACES WHERE NAME=\\'test/t1\\']" = 0
}
--source include/assert.inc

--echo # We want only first 100 pages to be rotated
SET GLOBAL debug="+d,rotate_only_first_100_pages_from_t1";

--echo # Start rotation unnencrypted => encrypted (tables do not have crypt data stored in page 0)
SET GLOBAL innodb_encryption_threads = 4;
SET GLOBAL default_table_encryption=ONLINE_TO_KEYRING;

--let tables_count=`select count(*) from INFORMATION_SCHEMA.INNODB_TABLESPACES`

--echo # Wait max 10 min for key encryption threads to encrypt all spaces
--let $wait_timeout= 600
# All tables should get encrypted. tables_count - 2 because temporary tablespace is not encrypted and t1 will only
# have half of the pages encrypted and thus min_key_version will still be 0
--let $wait_condition=SELECT COUNT(*) = $tables_count - 2 FROM INFORMATION_SCHEMA.INNODB_TABLESPACES_ENCRYPTION WHERE MIN_KEY_VERSION = 1
--source include/wait_condition.inc

--echo # Table t1 should have min_key_version = 0 assigned and ROTATIONG_OR_FLUSHING=1 <= this means that only 100 pages
--echo # have been rotatted.
--let $wait_timeout= 600
--let $wait_condition=SELECT name = 'test/t1' FROM INFORMATION_SCHEMA.INNODB_TABLESPACES_ENCRYPTION WHERE MIN_KEY_VERSION = 0 AND ROTATING_OR_FLUSHING = 1
--source include/wait_condition.inc

--echo # Disable rotation threads
SET GLOBAL innodb_encryption_threads = 0;
--echo # Disable rotation of only first 100 pages
SET GLOBAL debug="-d,rotate_only_first_100_pages_from_t1";

--let $grep_log=`SELECT !$is_t1_compressed && !$is_mk_encrypted`

--source include/shutdown_mysqld.inc

--echo # Make sure that t1 still contains foobar - as it is only encrypted in half
--echo # unless originaly table was compressed or MK encrypted - then it does not
--echo # make sense to check the table as table will not contain plaintext
if ($grep_log)
{
  --let SEARCH_PATTERN=foobar
  --let ABORT_ON=NOT_FOUND
  --let SEARCH_FILE=$t1_IBD
  --source include/search_pattern_in_file.inc
}

--echo # t1 is only half rotatted, now we will check if the encryption can be completed after the restart
--let $restart_parameters=restart:--default-table-encryption=ONLINE_TO_KEYRING --innodb-encryption-rotate-key-age=15 --innodb-encryption-threads=4
--source include/start_mysqld.inc

--let $wait_timeout= 600
--echo # All tables should get encrypted. tables_count - 1 because temporary tablespace should remain unencrypted
--echo # innodb_system
--let $wait_condition=SELECT COUNT(*) = $tables_count - 1 FROM INFORMATION_SCHEMA.INNODB_TABLESPACES_ENCRYPTION WHERE MIN_KEY_VERSION = 1
--source include/wait_condition.inc

--echo # Make sure t1 is encrypted
if (!$is_t1_compressed)
{
  --let SEARCH_PATTERN=foobar
  --let ABORT_ON=FOUND
  --let SEARCH_FILE=$t1_IBD
  --source include/search_pattern_in_file.inc
}

--echo # Now we try rotation encrypted => unencrypted
--echo # Enable rotation of only first 100 pages
SET GLOBAL debug="+d,rotate_only_first_100_pages_from_t1";

SET GLOBAL default_table_encryption=ONLINE_FROM_KEYRING_TO_UNENCRYPTED;

--let $wait_timeout= 600
# All tables should get unencrypted, apart from t1. tables_count - 2 - because t1 will only have half of the pages unencrypted =>
# thus min_key_version will still be 1 and temporary tablespace remains unencrypted
--let $wait_condition=SELECT COUNT(*) = $tables_count - 2 FROM INFORMATION_SCHEMA.INNODB_TABLESPACES_ENCRYPTION WHERE MIN_KEY_VERSION = 0
--source include/wait_condition.inc

--echo # Table t1 should have min_key_version = 1 assigned and ROTATING_OR_FLUSHING=1 <= this means that only 100 pages
--echo # have been rotatted.
--let $wait_timeout= 600
--let $wait_condition=SELECT name = 'test/t1' FROM INFORMATION_SCHEMA.INNODB_TABLESPACES_ENCRYPTION WHERE MIN_KEY_VERSION = 1 AND ROTATING_OR_FLUSHING = 1
--source include/wait_condition.inc

--echo # Disable rotation threads
SET GLOBAL innodb_encryption_threads = 0;
--echo # Disable rotation of only first 100 pages
SET GLOBAL debug="-d,rotate_only_first_100_pages_from_t1";

--source include/shutdown_mysqld.inc

--echo # Make sure that t1 contains foobar - as it is only decrypted in half
if (!$is_t1_compressed)
{
  --let SEARCH_PATTERN=foobar
  --let ABORT_ON=NOT_FOUND
  --let SEARCH_FILE=$t1_IBD
  --source include/search_pattern_in_file.inc
}

--echo # t1 is only half rotatted, now we will check if the decryption can be completed after the restart
--let $restart_parameters=restart:--default-table-encryption=ONLINE_FROM_KEYRING_TO_UNENCRYPTED --innodb-encryption-rotate-key-age=15 --innodb-encryption-threads=4
--source include/start_mysqld.inc

--let $wait_timeout= 600
# All tables should get decrypted. tables_count - 1 because temporary tablespace remains unencrypted
--let $wait_condition=SELECT COUNT(*) = $tables_count - 1 FROM INFORMATION_SCHEMA.INNODB_TABLESPACES_ENCRYPTION WHERE MIN_KEY_VERSION = 0
--source include/wait_condition.inc

--echo # Make sure that t1 contains foobar - as it is decrypted
if (!$is_t1_compressed)
{
  --let SEARCH_PATTERN=foobar
  --let ABORT_ON=NOT_FOUND
  --let SEARCH_FILE=$t1_IBD
  --source include/search_pattern_in_file.inc
}

--echo # Now we try rotation => unencrypted => encrypted (now tables have crypt data in page 0)
--echo # Enable rotation of only first 100 pages
SET GLOBAL debug="+d,rotate_only_first_100_pages_from_t1";

SET GLOBAL default_table_encryption=ONLINE_TO_KEYRING;

--let $wait_timeout= 600
# All tables should get unencrypted, apart from t1 and temporary tablespace.
# tables_count - 2 - because t1 will only have half of the pages encrypted => thus min_key_version will still be 0
# and temporary tablespace remains unencrypted
--let $wait_condition=SELECT COUNT(*) = $tables_count - 2 FROM INFORMATION_SCHEMA.INNODB_TABLESPACES_ENCRYPTION WHERE MIN_KEY_VERSION = 1
--source include/wait_condition.inc

--echo # Table t1 should have min_key_version = 0 assigned and ROTATING_OR_FLUSHING=1 <= this means that only 100 pages
--echo # have been rotatted.
--let $wait_timeout= 600
--let $wait_condition=SELECT name = 'test/t1' FROM INFORMATION_SCHEMA.INNODB_TABLESPACES_ENCRYPTION WHERE MIN_KEY_VERSION = 0 AND ROTATING_OR_FLUSHING = 1
--source include/wait_condition.inc

--echo # Disable rotation threads
SET GLOBAL innodb_encryption_threads = 0;
--echo # Disable rotation of only first 100 pages
SET GLOBAL debug="-d,rotate_only_first_100_pages_from_t1";

--source include/shutdown_mysqld.inc

--echo # Make sure that t1 contains foobar - as it is only encrypted in half

if (!$is_t1_compressed)
{
  --let SEARCH_PATTERN=foobar
  --let ABORT_ON=NOT_FOUND
  --let SEARCH_FILE=$t1_IBD
  --source include/search_pattern_in_file.inc
}

--echo # t1 is only half rotatted, now we will check if the decryption can be completed after the restart
--let $restart_parameters=restart:--default-table-encryption=ONLINE_TO_KEYRING --innodb-encryption-rotate-key-age=15 --innodb-encryption-threads=4
--source include/start_mysqld.inc

--let $wait_timeout= 600
# All tables should get encrypted. tables_count - 1 because temporary tablespace remains unencrypted
--let $wait_condition=SELECT COUNT(*) = $tables_count - 1 FROM INFORMATION_SCHEMA.INNODB_TABLESPACES_ENCRYPTION WHERE MIN_KEY_VERSION = 1
--source include/wait_condition.inc

--echo # Make sure that t1 does not contain foobar - as it is encrypted
if (!$is_t1_compressed)
{
  --let SEARCH_PATTERN=foobar
  --let ABORT_ON=FOUND
  --let SEARCH_FILE=$t1_IBD
  --source include/search_pattern_in_file.inc
}

--echo # After table has been rotated there is update to DD and after that
--echo # there is update to page0 of the table. When a crash occures after DD update we will go out of sync with table's
--echo # flags stored in page 0 of the table. Server discover that and puts a warning into the error log. page0 should get
--echo # updated correctly after the restart. Server will discover that the rotation of the table has not been completed
--echo # and will update page 0.
--echo # In order to check this behavior we will first rotate encrypted=>unencrypted and we will make server commit suicide
--echo # after DD update and before page 0 update.


SET GLOBAL debug="+d,crash_on_t1_flush_after_dd_update";

--source include/expect_crash.inc
SET GLOBAL default_table_encryption=ONLINE_FROM_KEYRING_TO_UNENCRYPTED;
--source include/wait_until_disconnected.inc

# The server crashed after but t1's pages have been flushed to disk by encryption thread
--echo # Make sure that t1 has been decrypted
if (!$is_t1_compressed)
{
  --let SEARCH_PATTERN=foobar
  --let ABORT_ON=NOT_FOUND
  --let SEARCH_FILE=$t1_IBD
  --source include/search_pattern_in_file.inc
}

--echo # First restart the server after crash so any redo logs for t1 were proceed. If there are any logs available
--echo # t1 will be opened without validating idb file to DD.
--let $restart_parameters=restart:--innodb-encryption-rotate-key-age=15 --innodb-encryption-threads=0
--source include/start_mysqld.inc

SET GLOBAL debug="-d,crash_on_t1_flush_after_dd_update";

--echo # Now restart the server and select from t1. Validation of t1's flags should proceed.
--echo # It should be discovered that there is mismatch of flags in t1. However it shoud be possible to use
--echo # the table. There should be a warning message in the error log file.

--let $error_log=$MYSQL_TMP_DIR/my_restart.err

--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR
--let $restart_parameters=restart:--innodb-encryption-rotate-key-age=15 --innodb-encryption-threads=0 --log-error=$error_log
--source include/restart_mysqld.inc

SELECT COUNT(1) FROM t1;

--source include/shutdown_mysqld.inc

--let ABORT_ON=NOT_FOUND
--let SEARCH_FILE=$error_log
--let SEARCH_PATTERN=Table encryption flag is OFF in the data dictionary but the encryption flag in file \.\/test\/t1\.ibd is ON\. This indicates that the rotation of the table was interrupted before space\'s flags were updated\. Please have encryption_thread variable \(innodb-encryption-threads\) set to value > 0\. So the encryption could finish up the rotation\.
--source include/search_pattern_in_file.inc

--remove_file $error_log

--echo # Restart and finish up the rotation
--let $restart_parameters=restart:--default-table-encryption=ONLINE_FROM_KEYRING_TO_UNENCRYPTED --innodb-encryption-rotate-key-age=15 --innodb-encryption-threads=4
--source include/start_mysqld.inc

--let $wait_timeout= 600
# All tables should get unencrypted
--let $wait_condition=SELECT COUNT(*) = 0 FROM INFORMATION_SCHEMA.INNODB_TABLESPACES_ENCRYPTION WHERE MIN_KEY_VERSION <> 0
--source include/wait_condition.inc

# Restart and make sure that warning is no longer available in error log
--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR
--let $restart_parameters=restart:--innodb-encryption-rotate-key-age=15 --innodb-encryption-threads=0 --log-error=$error_log
--source include/restart_mysqld.inc

SELECT COUNT(1) FROM t1;

--source include/shutdown_mysqld.inc

--let ABORT_ON=FOUND
--let SEARCH_FILE=$error_log
--let SEARCH_PATTERN=Table encryption flag is OFF in the data dictionary but the encryption flag in file \.\/test\/t1\.ibd is ON\. This indicates that the rotation of the table was interrupted before space\'s flags were updated\. Please have encryption_thread variable \(innodb-encryption-threads\) set to value > 0\. So the encryption could finish up the rotation\.
--source include/search_pattern_in_file.inc

--echo # Now we need to test the rotation unencrypted=>encrypted, when we get a crash after all pages
--echo # got rotated and DD was updated but before updating page 0

--let $restart_parameters=restart:--default-table-encryption=ONLINE_TO_KEYRING --innodb-encryption-rotate-key-age=15 --innodb-encryption-threads=0
--source include/start_mysqld.inc

SET GLOBAL debug="+d,crash_on_t1_flush_after_dd_update";

--source include/expect_crash.inc
SET GLOBAL innodb_encryption_threads=4;
--source include/wait_until_disconnected.inc

--echo # First restart the server after crash so any redo logs for t1 were proceed. If there are any logs available
--echo # t1 will be opened without validating idb file to DD.
--let $restart_parameters=restart:--innodb-encryption-rotate-key-age=15 --innodb-encryption-threads=0
--source include/start_mysqld.inc

SET GLOBAL debug="-d,crash_on_t1_flush_after_dd_update";

--source include/shutdown_mysqld.inc

--echo # Make sure that t1 has been encrypted
if (!$is_t1_compressed)
{
  --let SEARCH_PATTERN=foobar
  --let ABORT_ON=FOUND
  --let SEARCH_FILE=$t1_IBD
  --source include/search_pattern_in_file.inc
}

--echo # Now start the server and select from t1. Validation of t1's flags should proceed.
--echo # It should be discovered that there is mismatch of flags in t1. However it shoud be possible to use
--echo # the table. There should be a warning message in the error log file.

--remove_file $error_log

--replace_result $MYSQL_TMP_DIR MYSQL_TMP_DIR
--let $restart_parameters=restart:--innodb-encryption-threads=0 --log-error=$error_log --loose-keyring_file_data=$MYSQL_TMP_DIR/mysecret_keyring --early-plugin-load="keyring_file=$KEYRING_PLUGIN"
--source include/start_mysqld.inc

SELECT COUNT(1) FROM t1;

--source include/shutdown_mysqld.inc

--let ABORT_ON=NOT_FOUND
--let SEARCH_FILE=$error_log
--let SEARCH_PATTERN=Table encryption flag is ON in the data dictionary but the encryption flag in file \.\/test\/t1\.ibd is OFF\. This indicates that the rotation of the table was interrupted before space\'s flags were updated\. Please have encryption_thread variable \(innodb-encryption-threads\) set to value > 0\. So the encryption could finish up the rotation\.
--source include/search_pattern_in_file.inc

--remove_file $error_log

--echo # Restart and finish up the rotation
--let $restart_parameters=restart:--default-table-encryption=ONLINE_TO_KEYRING --innodb-encryption-rotate-key-age=15 --innodb-encryption-threads=4
--source include/start_mysqld.inc

--let $wait_timeout= 600
# All tables should get encrypted
--let $wait_condition=SELECT COUNT(*) = 0 FROM INFORMATION_SCHEMA.INNODB_TABLESPACES_ENCRYPTION WHERE MIN_KEY_VERSION = 0
--source include/wait_condition.inc

# cleanup
DROP TABLE t1;
DROP PROCEDURE innodb_insert_proc;

# decrypt all the tables
SET GLOBAL default_table_encryption=ONLINE_FROM_KEYRING_TO_UNENCRYPTED;

--let $wait_timeout= 600
--let $wait_condition=SELECT COUNT(*) = 0 FROM INFORMATION_SCHEMA.INNODB_TABLESPACES_ENCRYPTION WHERE MIN_KEY_VERSION <> 0
--source include/wait_condition.inc

SET GLOBAL default_table_encryption=OFF;
SET GLOBAL innodb_encryption_threads=0;
