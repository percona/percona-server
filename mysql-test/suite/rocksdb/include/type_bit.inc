#
# BIT column type
#

# $extra_col_opts is set by a calling test
# $extra_col_opts_ddl_err is set by a calling test
# $extra_col_opts_dml_err is set by a calling test

# Valid values
if ($extra_col_opts_ddl_err == 0)
{
  eval CREATE TABLE t1 (
    a BIT $extra_col_opts,
    b BIT(20) $extra_col_opts,
    c BIT(64) $extra_col_opts,
    d BIT(1) $extra_col_opts,
    PRIMARY KEY (c)
  ) ENGINE=rocksdb;
}
if ($extra_col_opts_ddl_err == 1)
{
  # 5.7 uses strict mode by default, so this will now return an error
  --error ER_PRIMARY_CANT_HAVE_NULL
   eval CREATE TABLE t1 (
    a BIT $extra_col_opts,
    b BIT(20) $extra_col_opts,
    c BIT(64) $extra_col_opts,
    d BIT(1) $extra_col_opts,
    PRIMARY KEY (c)
  ) ENGINE=rocksdb;

  # 5.7 _requires_ a not NULL PK and can't mimick 5.6 behavior, so we are
  # adding in a fake key to satisfy and allow the test to resume
  eval CREATE TABLE t1 (
    a BIT $extra_col_opts,
    b BIT(20) $extra_col_opts,
    c BIT(64) $extra_col_opts,
    d BIT(1) $extra_col_opts,
    fake_key INT NOT NULL AUTO_INCREMENT,
    PRIMARY KEY (fake_key)
  ) ENGINE=rocksdb;
}
SHOW COLUMNS IN t1;

ALTER TABLE t1 DROP COLUMN d;
--error ER_INVALID_FIELD_SIZE
eval ALTER TABLE t1 ADD COLUMN d BIT(0) $extra_col_opts;
SHOW COLUMNS IN t1;
eval ALTER TABLE t1 ADD COLUMN d BIT(1) $extra_col_opts;

INSERT INTO t1 (a,b,c,d) VALUES (0,POW(2,20)-1,b'1111111111111111111111111111111111111111111111111111111111111111',1);
SELECT BIN(a), HEX(b), c+0 FROM t1 WHERE d>0;

INSERT INTO t1 (a,b,c,d) VALUES (1,0,-2,0);
--sorted_result
SELECT a+0, b+0, c+0 FROM t1 WHERE d<100;

INSERT INTO t1 (a,b,c,d) VALUES (b'1', 'f', 0xFF, 0x0);
--sorted_result
SELECT a+0, b+0, c+0 FROM t1 WHERE d IN (0, 2);

DELETE FROM t1;

# Out of range values
# (should produce warnings)

# 5.7 uses strict mode by default, so this will now return an error
--error ER_DATA_TOO_LONG
INSERT INTO t1 (a,b,c,d) VALUES (0x10,0,0,1);

# now test with the 5.6 behavior just to be sure
set @orig_sql_mode=@@session.sql_mode;
set session sql_mode="";
INSERT INTO t1 (a,b,c,d) VALUES (0x10,0,0,1);
set session sql_mode=@orig_sql_mode;

SELECT a+0,b+0,c+0,d+0 FROM t1;

# 5.7 uses strict mode by default, so this will now return an error
--error ER_WARN_DATA_OUT_OF_RANGE
INSERT INTO t1 (a,b,c,d) VALUES (0x01,0,0x10000000000000000,0);

# now test with the 5.6 behavior just to be sure
set @orig_sql_mode=@@session.sql_mode;
set session sql_mode="";
INSERT INTO t1 (a,b,c,d) VALUES (0x01,0,0x10000000000000000,0);
set session sql_mode=@orig_sql_mode;

--sorted_result
SELECT a+0,b+0,c+0,d+0 FROM t1;

DROP TABLE t1;

--error ER_TOO_BIG_DISPLAYWIDTH
eval CREATE TABLE t1 (pk INT PRIMARY KEY, a BIT(65) $extra_col_opts) ENGINE=rocksdb;
