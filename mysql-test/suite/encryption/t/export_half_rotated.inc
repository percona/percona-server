
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
eval call innodb_insert_proc($number_of_records);
commit;
set autocommit=1;

# Make sure encryption is disabled
--let $assert_text= Make sure encryption is disabled
--let $assert_cond= "[SELECT @@GLOBAL.default_table_encryption]" = 0
--source include/assert.inc


if ($is_encrypted)
{
--let $assert_text= Make sure t1 is encrypted
--let $assert_cond= "[SELECT FLAG & 8192 FROM INFORMATION_SCHEMA.INNODB_TABLESPACES WHERE NAME=\\'test/t1\\']" = 8192
--source include/assert.inc
}

if (!$is_encrypted)
{
--let $assert_text= Make sure t1 is not encrypted
--let $assert_cond= "[SELECT FLAG & 8192 FROM INFORMATION_SCHEMA.INNODB_TABLESPACES WHERE NAME=\\'test/t1\\']" = 0
--source include/assert.inc
}
--echo # We want only first 100 pages to be rotated
SET GLOBAL debug="+d,rotate_only_first_x_pages_from_t1";

--echo # Start rotation to online keyring encrypted (tables do not have crypt data stored in page 0)
SET GLOBAL innodb_encryption_threads = 0;
SET GLOBAL default_table_encryption=ONLINE_TO_KEYRING;
SET GLOBAL innodb_encryption_threads = 1;

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
SET GLOBAL debug="-d,rotate_only_first_x_pages_from_t1";

--source include/shutdown_mysqld.inc

--echo # Make sure that t1 deos not contain foobar - if first 100 pages should be
--echo # KEYRING encrypted and the remaining pages should be Master Key encrypted.
--echo # Or check if t1 is not encrypted if we requested only first 100 pages to
--echo # be KEYRING encrypted and rest un-encrypted.
--let SEARCH_PATTERN=foobar
if ($is_encrypted)
{
--let ABORT_ON=FOUND
}
if (!$is_encrypted)
{
--let ABORT_ON=NOT_FOUND
}
--let SEARCH_FILE=$t1_IBD
--source include/search_pattern_in_file.inc

--echo # Now let's restart with encryption threads diabled and check whether
--echo # the table can be FLUSH FOR EXPORT
--let $restart_parameters=restart:--default-table-encryption=OFF --innodb-encryption-rotate-key-age=15 --innodb-encryption-threads=0
--source include/start_mysqld.inc

FLUSH TABLE t1 FOR EXPORT;

--perl
do "$ENV{MYSQL_TEST_DIR}/include/innodb-util.inc";
ib_backup_tablespaces("test", "t1");
EOF

UNLOCK TABLES;
DROP TABLE t1;

