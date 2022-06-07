#
# NOT NULL attribute in columns
#
# Usage:
# let $col_type = <column type>;
# let $col_default = <default value for a column>;
# let $col_default_err = 0|1 if we expect ER_BLOB_CANT_HAVE_DEFAULT error from
#   CREATE TABLE
# --source col_not_null.inc
#
# We will add NOT NULL to the column options;
#

--echo #----------------------------------
--echo # $col_type NOT NULL columns without a default
--echo #----------------------------------

eval CREATE TABLE t1 (pk INT AUTO_INCREMENT PRIMARY KEY, c $col_type NOT NULL) ENGINE=rocksdb;
SHOW COLUMNS IN t1;

--error ER_BAD_NULL_ERROR
INSERT INTO t1 (c) VALUES (NULL);
eval INSERT INTO t1 (c) VALUES ($col_default);
SELECT HEX(c) FROM t1;

DROP TABLE t1;

--echo #----------------------------------
--echo # $col_type NOT NULL columns with a default
--echo #----------------------------------

if ($col_default_err == 0)
{
  eval CREATE TABLE t1 (
    pk INT AUTO_INCREMENT PRIMARY KEY,
    c $col_type NOT NULL DEFAULT $col_default
  ) ENGINE=rocksdb;
}
if ($col_default_err == 1)
{
  # 5.7 uses strict mode by default, so this will now return an error
  --error ER_BLOB_CANT_HAVE_DEFAULT
  eval CREATE TABLE t1 (
    pk INT AUTO_INCREMENT PRIMARY KEY,
    c $col_type NOT NULL DEFAULT $col_default
  ) ENGINE=rocksdb;

  # now test with the 5.6 behavior just to be sure
  set @orig_sql_mode=@@session.sql_mode;
  set session sql_mode="";
  eval CREATE TABLE t1 (
    pk INT AUTO_INCREMENT PRIMARY KEY,
    c $col_type NOT NULL DEFAULT $col_default
  ) ENGINE=rocksdb;
  set session sql_mode=@orig_sql_mode;
}
SHOW COLUMNS IN t1;

--error ER_INVALID_DEFAULT
eval ALTER TABLE t1 ADD COLUMN err $col_type NOT NULL DEFAULT NULL;

--error ER_BAD_NULL_ERROR
INSERT INTO t1 (c) VALUES (NULL);

eval INSERT INTO t1 (c) VALUES ($col_default);
eval INSERT INTO t1 () VALUES ();

# HEX should be universal for all column types
SELECT pk, HEX(c) FROM t1 ORDER BY pk;

DROP TABLE t1;
