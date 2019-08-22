# test temporary tables encryption

call mtr.add_suppression("\\[Error\\] InnoDB: Encryption can't find master key, please check the keyring plugin is loaded.");

--let $MYSQL_DATA_DIR= `select @@datadir`

# we need restart to make sure keyring settings are picked up
let $restart_hide_args = 1;
let $restart_parameters = $KEYRING_RESTART_PARAM --innodb_max_dirty_pages_pct=0;
--source include/restart_mysqld.inc

CREATE TEMPORARY TABLE t01 (a TEXT) ENGINE=InnoDB;
INSERT INTO t01 VALUES ('Lorem ipsum dolor sit amet, consectetur adipiscing elit');

# wait until pages flushed
--let $wait_condition= SELECT variable_value = 0 FROM performance_schema.global_status WHERE LOWER(variable_name) = 'innodb_buffer_pool_pages_dirty'
--source include/wait_condition.inc

# make sure ibtmp1 is not encrypted
--let SEARCH_PATTERN= Lorem ipsum dolor sit amet
--let SEARCH_FILE= $MYSQL_DATA_DIR/ibtmp1
--let ABORT_ON= NOT_FOUND
--source include/search_pattern_in_file.inc


# this table created in separate file per table tablespace, make sure it not
# encrypted
CREATE TEMPORARY TABLE t04 (a TEXT) ENGINE=InnoDB ROW_FORMAT=COMPRESSED;
INSERT INTO t04 VALUES ('Praesent tristique eros a tempus fringilla');

# wait until pages flushed
--let $wait_condition= SELECT variable_value = 0 FROM performance_schema.global_status WHERE LOWER(variable_name) = 'innodb_buffer_pool_pages_dirty'
--source include/wait_condition.inc

--let $t04_fn= `SELECT CONCAT(@@tmpdir, '/', NAME, '.ibd') FROM INFORMATION_SCHEMA.INNODB_TEMP_TABLE_INFO WHERE NAME LIKE '#%';`

# make sure t04 is not encrypted
--let SEARCH_PATTERN= Praesent tristique eros a tempus fringilla
--let SEARCH_FILE= $t04_fn
--let ABORT_ON= NOT_FOUND
--source include/search_pattern_in_file.inc

DROP TABLE t04;

SET GLOBAL innodb_temp_tablespace_encrypt = ON;

CREATE TEMPORARY TABLE t02 (a INT) ENGINE=InnoDB ENCRYPTION='Y';

CREATE TEMPORARY TABLE t03 (a TEXT) ENGINE=InnoDB ENCRYPTION='Y';
INSERT INTO t03 VALUES ('Curabitur laoreet, velit non interdum venenatis');

# wait until pages flushed
--let $wait_condition= SELECT variable_value = 0 FROM performance_schema.global_status WHERE LOWER(variable_name) = 'innodb_buffer_pool_pages_dirty'
--source include/wait_condition.inc

# make sure ibtmp1 is encrypted now
--let SEARCH_PATTERN= Curabitur laoreet, velit non interdum venenatis
--let SEARCH_FILE= $MYSQL_DATA_DIR/ibtmp1
--let ABORT_ON= FOUND
--source include/search_pattern_in_file.inc

# this table created in separate file per table tablespace, make sure it is
# encrypted as well
SET GLOBAL innodb_encrypt_tables=ON;
CREATE TEMPORARY TABLE t04 (a TEXT) ENGINE=InnoDB ROW_FORMAT=COMPRESSED;
SET GLOBAL innodb_encrypt_tables=OFF;
INSERT INTO t04 VALUES ('Praesent tristique eros a tempus fringilla');

# wait until pages flushed
--let $wait_condition= SELECT variable_value = 0 FROM performance_schema.global_status WHERE LOWER(variable_name) = 'innodb_buffer_pool_pages_dirty'
--source include/wait_condition.inc

--let $t04_fn= `SELECT CONCAT(@@tmpdir, '/', NAME, '.ibd') FROM INFORMATION_SCHEMA.INNODB_TEMP_TABLE_INFO WHERE NAME LIKE '#%';`

# make sure t04 is encrypted
--let SEARCH_PATTERN= Praesent tristique eros a tempus fringilla
--let SEARCH_FILE= $t04_fn
--let ABORT_ON= FOUND
--source include/search_pattern_in_file.inc

DROP TABLE t04;

CREATE TEMPORARY TABLE t05 (a TEXT) ENGINE=InnoDB ROW_FORMAT=COMPRESSED ENCRYPTION='y';
INSERT INTO t05 VALUES ('Quisque malesuada placerat nisl');

--let $wait_condition= SELECT variable_value = 0 FROM performance_schema.global_status WHERE LOWER(variable_name) = 'innodb_buffer_pool_pages_dirty'
--source include/wait_condition.inc

--let SEARCH_PATTERN= Quisque malesuada placerat nisl
--let SEARCH_FILE= `SELECT CONCAT(@@tmpdir, '/', NAME, '.ibd') FROM INFORMATION_SCHEMA.INNODB_TEMP_TABLE_INFO WHERE NAME LIKE '#%';`
--let ABORT_ON= FOUND
--source include/search_pattern_in_file.inc

DROP TABLE t05;

CREATE TEMPORARY TABLE t06 (a TEXT) ENGINE=InnoDB ROW_FORMAT=COMPRESSED ENCRYPTION='n';
INSERT INTO t06 VALUES ('Sed in libero ut nibh placerat accumsan');

--let $wait_condition= SELECT variable_value = 0 FROM performance_schema.global_status WHERE LOWER(variable_name) = 'innodb_buffer_pool_pages_dirty'
--source include/wait_condition.inc

--let SEARCH_PATTERN= Sed in libero ut nibh placerat accumsan
--let SEARCH_FILE= `SELECT CONCAT(@@tmpdir, '/', NAME, '.ibd') FROM INFORMATION_SCHEMA.INNODB_TEMP_TABLE_INFO WHERE NAME LIKE '#%';`
--let ABORT_ON= NOT_FOUND
--source include/search_pattern_in_file.inc

DROP TABLE t06;

# test that we can turn encryption OFF and ON

SET GLOBAL innodb_temp_tablespace_encrypt = OFF;

# Setting OFF after encryption allows to create unencrypted tables.
--error ER_ILLEGAL_HA_CREATE_OPTION
CREATE TEMPORARY TABLE t07 (a INT) ENGINE=InnoDB ENCRYPTION='Y';
CREATE TEMPORARY TABLE t07 (a INT) ENGINE=InnoDB;
INSERT INTO t07 VALUES (1), (2), (3);

SET GLOBAL innodb_temp_tablespace_encrypt = ON;

# create large enough table in order to make CREATE INDEX to use temporary table
CREATE TABLE t10 (a INT AUTO_INCREMENT PRIMARY KEY, b INT);
INSERT INTO t10 (b) VALUES (FLOOR(RAND() * 10000)), (FLOOR(RAND() * 10000)), (FLOOR(RAND() * 10000));
INSERT INTO t10 (b) VALUES (FLOOR(RAND() * 10000)), (FLOOR(RAND() * 10000)), (FLOOR(RAND() * 10000));
INSERT INTO t10 (b) VALUES (FLOOR(RAND() * 10000)), (FLOOR(RAND() * 10000)), (FLOOR(RAND() * 10000));
INSERT INTO t10 (b) VALUES (FLOOR(RAND() * 10000)), (FLOOR(RAND() * 10000)), (FLOOR(RAND() * 10000));
INSERT INTO t10 (b) VALUES (FLOOR(RAND() * 10000)), (FLOOR(RAND() * 10000)), (FLOOR(RAND() * 10000));
INSERT INTO t10 (b) VALUES (FLOOR(RAND() * 10000)), (FLOOR(RAND() * 10000)), (FLOOR(RAND() * 10000));
INSERT INTO t10 (b) VALUES (FLOOR(RAND() * 10000)), (FLOOR(RAND() * 10000)), (FLOOR(RAND() * 10000));
INSERT INTO t10 (b) VALUES (FLOOR(RAND() * 10000)), (FLOOR(RAND() * 10000)), (FLOOR(RAND() * 10000));
INSERT INTO t10 (b) VALUES (FLOOR(RAND() * 10000)), (FLOOR(RAND() * 10000)), (FLOOR(RAND() * 10000));
INSERT INTO t10 (b) VALUES (FLOOR(RAND() * 10000)), (FLOOR(RAND() * 10000)), (FLOOR(RAND() * 10000));

INSERT INTO t10 (b) SELECT b FROM t10;
INSERT INTO t10 (b) SELECT b FROM t10;
INSERT INTO t10 (b) SELECT b FROM t10;
INSERT INTO t10 (b) SELECT b FROM t10;

CREATE INDEX t10_b ON t10 (b);
DROP INDEX t10_b ON t10;

CREATE INDEX t10_b ON t10 (b) ALGORITHM=COPY;
DROP INDEX t10_b ON t10 ALGORITHM=COPY;

# test restarting
let $restart_hide_args = 1;
let $restart_parameters = $KEYRING_RESTART_PARAM --innodb-temp-tablespace-encrypt --innodb_max_dirty_pages_pct=0;
--source include/restart_mysqld.inc

CREATE TEMPORARY TABLE t01 (a INT) ENGINE=InnoDB ENCRYPTION='Y';
INSERT INTO t01 VALUES (1), (2), (3);

# test crashing
let $restart_hide_args = 1;
let $restart_parameters = $KEYRING_RESTART_PARAM --innodb-temp-tablespace-encrypt --innodb_max_dirty_pages_pct=0;
--source include/kill_and_restart_mysqld.inc

CREATE TEMPORARY TABLE t01 (a INT) ENGINE=InnoDB ENCRYPTION='Y';
INSERT INTO t01 VALUES (1), (2), (3);

# and able to startup without keyring plugin
let $restart_hide_args = 1;
let $restart_parameters = ;
--source include/restart_mysqld.inc

CREATE TEMPORARY TABLE t01 (a INT) ENGINE=InnoDB;
INSERT INTO t01 VALUES (1), (2), (3);

let $restart_hide_args = 1;
let $restart_parameters = $KEYRING_RESTART_PARAM --innodb-temp-tablespace-encrypt --innodb_max_dirty_pages_pct=0;
--source include/restart_mysqld.inc

CREATE TEMPORARY TABLE t01 (a INT) ENGINE=InnoDB ENCRYPTION='Y';
INSERT INTO t01 VALUES (1), (2), (3);

DROP TABLE t10;



#
# PS-3958: handle_fatal_signal (sig=11) in subselect_hash_sj_engine::cleanup
#
# Make sure MySQL doesn't crash in subselect_hash_sj_engine::cleanup
# when innodb_temp_tablespace_encrypt=ON but keyring isn't set up
#

let $restart_hide_args = 1;
let $restart_parameters = $KEYRING_RESTART_PARAM --innodb-temp-tablespace-encrypt;
--source include/restart_mysqld.inc

CREATE TABLE t1(a INT key) ENGINE = MEMORY;
INSERT INTO t1 VALUES (11061);
INSERT INTO t1 VALUES (3);
SET big_tables=1;
SELECT * FROM t1 WHERE a IN(SELECT MAX(a) FROM t1);
DROP TABLE t1;

#
# PS-5736: Make innodb_temp_tablespace_encrypt truly dynamic
#

--echo # innodb temp table encrypt is ON:
--error ER_ILLEGAL_HA_CREATE_OPTION
CREATE TEMPORARY TABLE t1 (a INT);

CREATE TEMPORARY TABLE tmp1 (a TEXT) ENCRYPTION='y';
INSERT INTO tmp1 VALUES ('Maecenas condimentum leo eu lorem aliquam malesuada');
CREATE TEMPORARY TABLE tmp2 (a TEXT) ENCRYPTION='y';

--let $wait_condition= SELECT variable_value = 0 FROM performance_schema.global_status WHERE LOWER(variable_name) = 'innodb_buffer_pool_pages_dirty'
--source include/wait_condition.inc

--let SEARCH_PATTERN= Maecenas condimentum leo eu lorem aliquam malesuada
--let SEARCH_FILE= $MYSQL_DATA_DIR/ibtmp1
--let ABORT_ON= FOUND
--source include/search_pattern_in_file.inc

# file-per-table temporary tables do not obey 'innodb_temp_tablespace_encrypt'
CREATE TEMPORARY TABLE t1 (a INT) KEY_BLOCK_SIZE=8 ENCRYPTION='y';
DROP TABLE t1;
CREATE TEMPORARY TABLE t1 (a INT) KEY_BLOCK_SIZE=8 ENCRYPTION='n';
DROP TABLE t1;

SET GLOBAL innodb_temp_tablespace_encrypt = OFF;

--error ER_ILLEGAL_HA_CREATE_OPTION
CREATE TEMPORARY TABLE t1 (a INT) ENCRYPTION='y';

INSERT INTO tmp2 VALUES ('Nam vestibulum mauris massa');

CREATE TEMPORARY TABLE tmp3 (a TEXT);
INSERT INTO tmp3 VALUES ('Sed sodales ligula sed enim condimentum');

--let $wait_condition= SELECT variable_value = 0 FROM performance_schema.global_status WHERE LOWER(variable_name) = 'innodb_buffer_pool_pages_dirty'
--source include/wait_condition.inc

--let SEARCH_PATTERN= Nam vestibulum mauris massa
--let SEARCH_FILE= $MYSQL_DATA_DIR/ibtmp1
--let ABORT_ON= NOT_FOUND
--source include/search_pattern_in_file.inc

--let SEARCH_PATTERN= Sed sodales ligula sed enim condimentum
--let SEARCH_FILE= $MYSQL_DATA_DIR/ibtmp1
--let ABORT_ON= NOT_FOUND
--source include/search_pattern_in_file.inc

# file-per-table temporary tables do not obey 'innodb_temp_tablespace_encrypt'
CREATE TEMPORARY TABLE t1 (a INT) ROW_FORMAT=COMPRESSED ENCRYPTION='y';
DROP TABLE t1;
CREATE TEMPORARY TABLE t1 (a INT) ROW_FORMAT=COMPRESSED ENCRYPTION='n';
DROP TABLE t1;

SET GLOBAL innodb_temp_tablespace_encrypt = ON;
