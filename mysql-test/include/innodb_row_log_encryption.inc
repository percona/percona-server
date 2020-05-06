--source include/have_innodb.inc
--source include/not_embedded.inc
--source include/have_debug_sync.inc
--source include/have_perfschema.inc

# Restart the server with keyring loaded
--let $restart_parameters="$KEYRING_RESTART_PARAM"
--let $restart_hide_args=1
--source include/restart_mysqld.inc

--source include/count_sessions.inc

#
# Create a table that will be encrypted and put some sensitive data to it (credit card numbers)
#

CREATE TABLE t1(id INT NOT NULL PRIMARY KEY AUTO_INCREMENT, credit_card VARCHAR(200), private VARCHAR(50)) ENGINE=InnoDB ENCRYPTION='y';

let $rows = 15000;
--disable_query_log
BEGIN;
while ($rows)
{
    eval INSERT INTO t1 VALUES (NULL, '0000-0000-0000-0000', 'private_data');
    dec $rows;
}
COMMIT;
--enable_query_log

#
# Now we create secondary index for credit_card column in parallel we create new rows
# forcing alter table to wait so that row log is used.
#
SET DEBUG_SYNC= 'row_merge_after_scan SIGNAL opened WAIT_FOR rotated';
send ALTER TABLE t1 ADD INDEX secret (credit_card), ALGORITHM=INPLACE, LOCK=NONE;
#
# Force key rotation and create second index for same table
#
connect (con2,localhost,root,,);
SET DEBUG_SYNC= 'now WAIT_FOR opened';
# TODO: once we add key rotation and scrubbing make sure to add it to this test
ALTER INSTANCE ROTATE INNODB MASTER KEY;
SET DEBUG_SYNC= 'now SIGNAL rotated';
CREATE TABLE t2(id int) ENGINE=InnoDB;
SET DEBUG_SYNC= 'row_merge_after_scan SIGNAL opened WAIT_FOR updated';
send ALTER TABLE t1 ADD INDEX secret2 (private), ALGORITHM=INPLACE, LOCK=NONE;
#
# Create new rows to row log
#
connect (con1,localhost,root,,);
SET DEBUG_SYNC= 'now WAIT_FOR opened';
INSERT INTO t1(credit_card) SELECT credit_card FROM t1;
INSERT INTO t1(credit_card) SELECT credit_card FROM t1;
INSERT INTO t1(credit_card) SELECT credit_card FROM t1;
SET DEBUG_SYNC= 'now SIGNAL updated';

connection default;
reap;
disconnect con1;

connection con2;
reap;
DROP TABLE t2;

connection default;
disconnect con2;

#
# Verify that both merge blocks and row log blocks are encryted and decrypted
#

SELECT variable_value > 0 FROM performance_schema.global_status WHERE variable_name = 'innodb_num_pages_encrypted';
SELECT variable_value > 0 FROM performance_schema.global_status WHERE variable_name = 'innodb_encryption_n_merge_blocks_encrypted';
SELECT variable_value > 0 FROM performance_schema.global_status WHERE variable_name = 'innodb_encryption_n_merge_blocks_decrypted';
SELECT variable_value > 0 FROM performance_schema.global_status WHERE variable_name = 'innodb_encryption_n_rowlog_blocks_encrypted';
SELECT variable_value > 0 FROM performance_schema.global_status WHERE variable_name = 'innodb_encryption_n_rowlog_blocks_decrypted';

DROP TABLE t1;

SET DEBUG_SYNC= 'RESET';

# PS-5689: Add an MTR testcase to test encryption of temporary tables created during online alter

let MYSQLD_DATADIR=`SELECT @@datadir`;

CREATE TABLE t (a INT, b TEXT) ENGINE=InnoDB ENCRYPTION='y';

let $i=1000;

--disable_query_log
while ($i) {
eval INSERT INTO t (a, b) VALUES ($i, UUID());
dec $i;
}
--enable_query_log

SET DEBUG_SYNC = 'row_log_table_apply1_before SIGNAL paused WAIT_FOR unpause';

send ALTER TABLE t ADD PRIMARY KEY (a);

connect (con1,localhost,root,,);
SET DEBUG_SYNC= 'now WAIT_FOR paused';

perl;
opendir(my $dh, "$ENV{'MYSQLD_DATADIR'}/test");
@files = grep(/^#sql.*ibd$/, readdir($dh));
closedir($dh);

die "temp table is not found" unless scalar(@files) > 0;

foreach $file (@files) {
  open(my $fh, '<:raw', "$ENV{'MYSQLD_DATADIR'}/test/$file") || die "unable to open $file";
  seek($fh, 38 + 16, 0) || die "unable to seek $file";
  read($fh, my $bytes, 4) || die "unable to read $file";
  my $flags = unpack("N*", $bytes);
  die "temp table $file is not encrypted" unless $flags & 8192 == 8192;
  close($fh);
}
EOF

SET DEBUG_SYNC= 'now SIGNAL unpause';
disconnect con1;

connection default;

reap;

DROP TABLE t;

SET DEBUG_SYNC= 'RESET';

-- source include/wait_until_count_sessions.inc
