#
# NULL attribute and DEFAULT NULL in columns
#
# Usage:
# let $col_type = <column type>;
# let $col_default = <default non-null value for a column>;
# let $col_default_err = 0|1 if we expect ER_BLOB_CANT_HAVE_DEFAULT error from
# --source col_null.inc
#
# We will add NULL attribute to the column options.
#


if ($col_default_err == 0)
{
  eval CREATE TABLE t1 (
    c $col_type NULL,
    c1 $col_type NULL DEFAULT NULL,
    c2 $col_type NULL DEFAULT $col_default,
    pk INT AUTO_INCREMENT PRIMARY KEY
  ) ENGINE=rocksdb;
}
if ($col_default_err == 1)
{
  # 5.7 uses strict mode by default, so this will now return an error
  --error ER_BLOB_CANT_HAVE_DEFAULT
  eval CREATE TABLE t1 (
    c $col_type NULL,
    c1 $col_type NULL DEFAULT NULL,
    c2 $col_type NULL DEFAULT $col_default,
    pk INT AUTO_INCREMENT PRIMARY KEY
  ) ENGINE=rocksdb;

  # now test with the 5.6 behavior just to be sure
  set @orig_sql_mode=@@session.sql_mode;
  set session sql_mode="";
  eval CREATE TABLE t1 (
    c $col_type NULL,
    c1 $col_type NULL DEFAULT NULL,
    c2 $col_type NULL DEFAULT $col_default,
    pk INT AUTO_INCREMENT PRIMARY KEY
  ) ENGINE=rocksdb;
  set session sql_mode=@orig_sql_mode;
}
SHOW COLUMNS IN t1;

INSERT INTO t1 (c,c1,c2) VALUES (NULL,NULL,NULL);
eval INSERT INTO t1 (c,c1,c2) VALUES ($col_default,$col_default,$col_default);
INSERT INTO t1 () VALUES ();

SELECT pk, HEX(c), HEX(c1), HEX(c2) FROM t1 ORDER BY pk;
SELECT pk, HEX(c2) FROM t1 ORDER BY pk;

DROP TABLE t1;
