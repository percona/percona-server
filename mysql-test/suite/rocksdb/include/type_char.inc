#
# CHAR column types
#

# $extra_col_opts is set by a calling test
# $extra_col_opts_ddl_err is set by a calling test
# $extra_col_opts_dml_err is set by a calling test

if ($extra_col_opts_ddl_err == 0)
{
  eval CREATE TABLE t1 (
    c CHAR $extra_col_opts,
    c0 CHAR(0) $extra_col_opts,
    c1 CHAR(1) $extra_col_opts,
    c20 CHAR(20) $extra_col_opts,
    c255 CHAR(255) $extra_col_opts,
    PRIMARY KEY (c255)
  ) ENGINE=rocksdb;
}
if ($extra_col_opts_ddl_err == 1)
{
  # 5.7 uses strict mode by default, so this will now return an error
  --error ER_PRIMARY_CANT_HAVE_NULL
  eval CREATE TABLE t1 (
    c CHAR $extra_col_opts,
    c0 CHAR(0) $extra_col_opts,
    c1 CHAR(1) $extra_col_opts,
    c20 CHAR(20) $extra_col_opts,
    c255 CHAR(255) $extra_col_opts,
    PRIMARY KEY (c255)
  ) ENGINE=rocksdb;

  # 5.7 _requires_ a not NULL PK and can't mimick 5.6 behavior, so we are
  # adding in a fake key to satisfy and allow the test to resume
  eval CREATE TABLE t1 (
    c CHAR $extra_col_opts,
    c0 CHAR(0) $extra_col_opts,
    c1 CHAR(1) $extra_col_opts,
    c20 CHAR(20) $extra_col_opts,
    c255 CHAR(255) $extra_col_opts,
    fake_key INT NOT NULL AUTO_INCREMENT,
    PRIMARY KEY (fake_key)
  ) ENGINE=rocksdb;
}
SHOW COLUMNS IN t1;

# Valid values

INSERT INTO t1 (c,c0,c1,c20,c255) VALUES ('','','','','');
INSERT INTO t1 (c,c0,c1,c20,c255) VALUES ('a','','b','abcdefghi klmnopqrst', 'Creating an article for the Knowledgebase is similar to asking questions. First, navigate to the category where you feel the article should be. Once there, double check that an article doesn');

--sorted_result
SELECT c,c0,c1,c20,c255 FROM t1;

# Invalid values

# 5.7 uses strict mode by default, so this will now return an error
--error ER_DATA_TOO_LONG
INSERT INTO t1 (c,c0,c1,c20,c255) VALUES ('abc', 'a', 'abc', REPEAT('a',21), REPEAT('x',256));

# now test with the 5.6 behavior just to be sure
set @orig_sql_mode=@@session.sql_mode;
set session sql_mode="";
INSERT INTO t1 (c,c0,c1,c20,c255) VALUES ('abc', 'a', 'abc', REPEAT('a',21), REPEAT('x',256));
set session sql_mode=@orig_sql_mode;

set @orig_tx_iso=@@session.transaction_isolation;
set session transaction_isolation='READ-COMMITTED';
# 5.7 uses strict mode by default, so this will now return an error
--error ER_DATA_TOO_LONG
INSERT INTO t1 (c,c0,c1,c20,c255) SELECT c255, c255, c255, c255, CONCAT('a',c255,c1) FROM t1;

# now test with the 5.6 behavior just to be sure
set @orig_sql_mode=@@session.sql_mode;
set session sql_mode="";
INSERT INTO t1 (c,c0,c1,c20,c255) SELECT c255, c255, c255, c255, CONCAT('a',c255,c1) FROM t1;
set session sql_mode=@orig_sql_mode;
set session transaction_isolation=@orig_tx_iso;

--sorted_result
SELECT c,c0,c1,c20,c255 FROM t1;

--sorted_result
SELECT DISTINCT c20, REPEAT('a',LENGTH(c20)), COUNT(*) FROM t1 GROUP BY c1, c20;

--error ER_TOO_BIG_FIELDLENGTH
eval ALTER TABLE t1 ADD COLUMN c257 CHAR(257) $extra_col_opts;

DROP TABLE t1;
