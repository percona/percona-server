#
# BINARY column types
#

# $extra_col_opts is set by a calling test
# $extra_col_opts_ddl_err is set by a calling test
# $extra_col_opts_dml_err is set by a calling test

eval CREATE TABLE t1 (
  b BINARY $extra_col_opts,
  b0 BINARY(0) $extra_col_opts,
  b1 BINARY(1) $extra_col_opts,
  b20 BINARY(20) $extra_col_opts,
  b255 BINARY(255) $extra_col_opts,
  pk BINARY PRIMARY KEY
) ENGINE=rocksdb;

SHOW COLUMNS IN t1;

# Valid values

INSERT INTO t1 VALUES ('','','','','','');

INSERT INTO t1 VALUES ('a','','b','abcdefghi klmnopqrst', 'Creating an article for the Knowledgebase is similar to asking questions. First, navigate to the category where you feel the article should be. Once there, double check that an article doesn\'t already exist which would work.','a');


--sorted_result
SELECT HEX(b), HEX(b0), HEX(b1), HEX(b20), HEX(b255), HEX(pk) FROM t1 ORDER BY pk;

# Invalid values

# 5.7 uses strict mode by default, so this will now return an error
--error ER_DATA_TOO_LONG
INSERT INTO t1 VALUES ('abc', 'a', 'abc', REPEAT('a',21), REPEAT('x',256),'b');

# now test with the 5.6 behavior just to be sure
set @orig_sql_mode=@@session.sql_mode;
set session sql_mode="";
INSERT INTO t1 VALUES ('abc', 'a', 'abc', REPEAT('a',21), REPEAT('x',256),'b');
set session sql_mode=@orig_sql_mode;

set @orig_tx_iso=@@session.tx_isolation;
set session tx_isolation='READ-COMMITTED';
# 5.7 uses strict mode by default, so this will now return an error
--error ER_DATA_TOO_LONG
INSERT INTO t1 SELECT b255, b255, b255, b255, CONCAT('a',b255,b255), 'c' FROM t1;

# now test with the 5.6 behavior just to be sure
set @orig_sql_mode=@@session.sql_mode;
set session sql_mode="";
--error ER_DUP_ENTRY
INSERT INTO t1 SELECT b255, b255, b255, b255, CONCAT('a',b255,b255), 'c' FROM t1;
set session sql_mode=@orig_sql_mode;
set session tx_isolation=@orig_tx_iso;

--sorted_result
SELECT HEX(b), HEX(b0), HEX(b1), HEX(b20), HEX(b255), HEX(pk) FROM t1 ORDER BY pk;

--error ER_TOO_BIG_FIELDLENGTH
eval ALTER TABLE t1 ADD COLUMN b257 BINARY(257) $extra_col_opts;

SHOW COLUMNS IN t1;

DROP TABLE t1;
