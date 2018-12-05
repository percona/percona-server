# InnoDB transparent tablespace data encryption
# This test case will test basic encryption support features.

--source include/no_valgrind_without_big.inc
--source include/have_innodb_max_16k.inc
--source include/have_debug.inc

# Create a table with encryption, should fail since keyring is not
# loaded
call mtr.add_suppression("\\[Error\\] InnoDB: Encryption can't find master key, please check the keyring plugin is loaded.");
call mtr.add_suppression("\\[ERROR\\] InnoDB: Failed to find tablespace for table `\.\.*`\.`\.\.*` in the cache.");
call mtr.add_suppression("\\[ERROR\\] InnoDB: Operating system error number 2 in a file operation.");
call mtr.add_suppression("\\[ERROR\\] InnoDB: The error means the system cannot find the path specified.");
call mtr.add_suppression("\\[ERROR\\] InnoDB: Could not find a valid tablespace file for");
call mtr.add_suppression("ibd can't be decrypted , please confirm the keyfile is match and keyring plugin is loaded.");
call mtr.add_suppression("\\[Warning\\] InnoDB: Ignoring tablespace .* because it could not be opened");
call mtr.add_suppression("\\[ERROR\\] InnoDB: If you are installing InnoDB, remember that you must create directories yourself, InnoDB does not create them.");

--let $restart_parameters="$KEYRING_RESTART_PARAM"
--let $restart_hide_args= 1
--source include/restart_mysqld.inc

--disable_warnings
DROP TABLE IF EXISTS t1;
--enable_warnings

let $innodb_file_per_table = `SELECT @@innodb_file_per_table`;
SET GLOBAL innodb_file_per_table=ON;

eval CREATE TABLE t1 (id int unsigned NOT NULL auto_increment PRIMARY KEY, val varchar(20) NOT NULL) ENCRYPTION="$encryption_type" ENGINE=InnoDB;

INSERT INTO t1 (val) VALUES ('Sydney'), ('Melbourne'), ('Brisbane'), ('Perth'), ('Adelaide');

#ALTER TABLE t1 ENCRYPTION = "KEYRING";
#ALTER TABLE t1 ENCRYPTION = 'Y';

#ALTER TABLE t1 ADD COLUMN is_capital char(1) NOT NULL DEFAULT 'N' AFTER val;
#SHOW CREATE TABLE t1;

LET $MYSQLD_DATADIR = `select @@datadir`;
let SEARCH_FILE = $MYSQLD_DATADIR/test/t1.ibd;
let SEARCH_PATTERN=Sydney;
# The string should not be found, since it's encrypted.
--source include/search_pattern.inc

# Cleanup
eval SET GLOBAL innodb_file_per_table=$innodb_file_per_table;
DROP TABLE t1;
