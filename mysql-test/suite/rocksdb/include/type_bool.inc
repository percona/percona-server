#
# BOOLEAN column type
#

# $extra_col_opts is set by a calling test
# $extra_col_opts_ddl_err is set by a calling test
# $extra_col_opts_dml_err is set by a calling test

eval CREATE TABLE t1 (
  pk INT AUTO_INCREMENT PRIMARY KEY,
  b1 BOOL $extra_col_opts,
  b2 BOOLEAN $extra_col_opts
) ENGINE=rocksdb;

SHOW COLUMNS IN t1;

# Valid values

INSERT INTO t1 (b1,b2) VALUES (1,TRUE);
SELECT b1,b2 FROM t1;

INSERT INTO t1 (b1,b2) VALUES (FALSE,0);
--sorted_result
SELECT b1,b2 FROM t1;

INSERT INTO t1 (b1,b2) VALUES (2,3);
--sorted_result
SELECT b1,b2 FROM t1;

INSERT INTO t1 (b1,b2) VALUES (-1,-2);
--sorted_result
SELECT b1,b2 FROM t1;

--sorted_result
SELECT IF(b1,'true','false') AS a, IF(b2,'true','false') AS b FROM t1;

--sorted_result
SELECT b1,b2 FROM t1 WHERE b1 = TRUE;

--sorted_result
SELECT b1,b2 FROM t1 WHERE b2 = FALSE;

# Invalid values

# 5.7 uses strict mode by default, so this will now return an error
--error ER_TRUNCATED_WRONG_VALUE_FOR_FIELD
INSERT INTO t1 (b1,b2) VALUES ('a','b');

# now test with the 5.6 behavior just to be sure
set @orig_sql_mode=@@session.sql_mode;
set session sql_mode="";
INSERT INTO t1 (b1,b2) VALUES ('a','b');
set session sql_mode=@orig_sql_mode;

--sorted_result
SELECT b1,b2 FROM t1;

# 5.7 uses strict mode by default, so this will now return an error
--error ER_WARN_DATA_OUT_OF_RANGE
INSERT INTO t1 (b1,b2) VALUES (128,-129);

# now test with the 5.6 behavior just to be sure
set @orig_sql_mode=@@session.sql_mode;
set session sql_mode="";
INSERT INTO t1 (b1,b2) VALUES (128,-129);
set session sql_mode=@orig_sql_mode;

--sorted_result
SELECT b1,b2 FROM t1;

# This is why we don't have zerofill and unsigned tests
# for boolean columns:
--error ER_PARSE_ERROR
eval ALTER TABLE t1 ADD COLUMN b3 BOOLEAN UNSIGNED $extra_col_opts;

--error ER_PARSE_ERROR
eval ALTER TABLE ADD COLUMN b3 BOOL ZEROFILL $extra_col_opts;

DROP TABLE t1;
