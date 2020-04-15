# test temporary tables encryption

call mtr.add_suppression("\\[Error\\] InnoDB: Encryption can't find master key, please check the keyring plugin is loaded.");

--let $MYSQL_DATA_DIR= `select @@datadir`

# we need restart to make sure keyring settings are picked up
let $restart_hide_args = 1;
let $restart_parameters = $KEYRING_RESTART_PARAM --innodb_max_dirty_pages_pct=0 --innodb_max_dirty_pages_pct_lwm=0;
--source include/restart_mysqld.inc

CREATE TEMPORARY TABLE t01 (a TEXT) ENGINE=InnoDB;
INSERT INTO t01 VALUES ('Lorem ipsum dolor sit amet, consectetur adipiscing elit');

# wait until pages flushed
--let $wait_condition= SELECT variable_value = 0 FROM performance_schema.global_status WHERE LOWER(variable_name) = 'innodb_buffer_pool_pages_dirty'
--source include/wait_condition.inc

# make sure the temp table went to unencrypted session temp tablespace

--let $session_temp_file = `SELECT CONCAT(@@datadir, PATH) FROM information_schema.innodb_session_temp_tablespaces WHERE ID = connection_id() AND PURPOSE="USER"`

--let SEARCH_PATTERN= Lorem ipsum dolor sit amet
--let SEARCH_FILE= $MYSQL_DATA_DIR/ibtmp1
--let ABORT_ON= FOUND
--source include/search_pattern_in_file.inc


# this table created in separate file per table tablespace, make sure it not
# encrypted
--error  ER_UNSUPPORT_COMPRESSED_TEMPORARY_TABLE
CREATE TEMPORARY TABLE t04 (a TEXT) ENGINE=InnoDB ROW_FORMAT=COMPRESSED;

SET GLOBAL innodb_temp_tablespace_encrypt = ON;

CREATE TEMPORARY TABLE t02 (a INT) ENGINE=InnoDB;

CREATE TEMPORARY TABLE t03 (a TEXT) ENGINE=InnoDB;
INSERT INTO t03 VALUES ('Curabitur laoreet, velit non interdum venenatis');

# wait until pages flushed
--let $wait_condition= SELECT variable_value = 0 FROM performance_schema.global_status WHERE LOWER(variable_name) = 'innodb_buffer_pool_pages_dirty'
--source include/wait_condition.inc

# make sure the temp table went to encrypted session temp tablespace
--let $session_temp_file = `SELECT CONCAT(@@datadir, PATH) FROM information_schema.innodb_session_temp_tablespaces WHERE ID = connection_id() AND PURPOSE="USER ENCRYPTED"`

--let SEARCH_PATTERN= Curabitur laoreet, velit non interdum venenatis
--let SEARCH_FILE= $MYSQL_DATA_DIR/ibtmp1
--let ABORT_ON= FOUND
--source include/search_pattern_in_file.inc

CREATE TEMPORARY TABLE t05 (a INT) ENGINE=InnoDB;
INSERT INTO t05 VALUES (1), (2), (3);

--error ER_ILLEGAL_HA_CREATE_OPTION
CREATE TEMPORARY TABLE t06 (a INT) ENGINE=InnoDB ENCRYPTION='n';

# test that we can turn encryption OFF and ON

SET GLOBAL innodb_temp_tablespace_encrypt = OFF;

# Setting OFF makes table to unencrypted session temp tablespace
CREATE TEMPORARY TABLE t07 (a INT) ENGINE=InnoDB;
INSERT INTO t07 VALUES (1), (2), (3);
DROP TABLE t07;

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

# and able to startup without keyring plugin
let $restart_hide_args = 1;
let $restart_parameters = ;
--source include/restart_mysqld.inc

CREATE TEMPORARY TABLE t01 (a INT) ENGINE=InnoDB;
INSERT INTO t01 VALUES (1), (2), (3);

let $restart_hide_args = 1;
let $restart_parameters = $KEYRING_RESTART_PARAM --innodb-temp-tablespace-encrypt --innodb_max_dirty_pages_pct=0 --innodb_max_dirty_pages_pct_lwm=0;
--source include/restart_mysqld.inc

CREATE TEMPORARY TABLE t01 (a INT) ENGINE=InnoDB;
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
