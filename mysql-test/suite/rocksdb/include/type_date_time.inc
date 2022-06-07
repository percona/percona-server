#
# Date and time column types
# (DATE, DATETIME, TIMESTAMP, TIME, YEAR)
#

# $extra_col_opts is set by a calling test
# $extra_col_opts_ddl_err is set by a calling test
# $extra_col_opts_dml_err is set by a calling test

eval CREATE TABLE t1 (
  d DATE $extra_col_opts,
  dt DATETIME $extra_col_opts,
  ts TIMESTAMP $extra_col_opts,
  t TIME $extra_col_opts,
  y YEAR $extra_col_opts,
  y4 YEAR(4) $extra_col_opts,
  pk DATETIME PRIMARY KEY
) ENGINE=rocksdb;

SHOW COLUMNS IN t1;

SET @tm = '2012-04-09 05:27:00';

# Valid values
# '1970-01-01 00:00:01'

# 5.7 uses strict mode by default, so this will now return an error
--error ER_TRUNCATED_WRONG_VALUE
INSERT INTO t1 (d,dt,ts,t,y,y4,pk) VALUES
('1000-01-01', '1000-01-01 00:00:00', FROM_UNIXTIME(1), '-838:59:59', '1901', '1901', '2012-12-12 12:12:12'),
('9999-12-31', '9999-12-31 23:59:59', FROM_UNIXTIME(2147483647), '838:59:59', '2155', '2155', '2012-12-12 12:12:13'),
('0000-00-00', '0000-00-00 00:00:00', '0000-00-00 00:00:00', '00:00:00', '0', '0', '2012-12-12 12:12:14'),
(DATE(@tm),@tm,TIMESTAMP(@tm),TIME(@tm),YEAR(@tm),YEAR(@tm),'2012-12-12 12:12:15');

# now test with the 5.6 behavior just to be sure
set @orig_sql_mode=@@session.sql_mode;
set session sql_mode="";
INSERT INTO t1 (d,dt,ts,t,y,y4,pk) VALUES
('1000-01-01', '1000-01-01 00:00:00', FROM_UNIXTIME(1), '-838:59:59', '1901', '1901', '2012-12-12 12:12:12'),
('9999-12-31', '9999-12-31 23:59:59', FROM_UNIXTIME(2147483647), '838:59:59', '2155', '2155', '2012-12-12 12:12:13'),
('0000-00-00', '0000-00-00 00:00:00', '0000-00-00 00:00:00', '00:00:00', '0', '0', '2012-12-12 12:12:14'),
(DATE(@tm),@tm,TIMESTAMP(@tm),TIME(@tm),YEAR(@tm),YEAR(@tm),'2012-12-12 12:12:15');
set session sql_mode=@orig_sql_mode;

--sorted_result
SELECT d,dt,ts,t,y,y4 FROM t1;

# Invalid values

# 5.7 uses strict mode by default, so this will now return an error
--error ER_TRUNCATED_WRONG_VALUE
INSERT INTO t1 (d,dt,ts,t,y,y4,pk) VALUES
('999-13-32', '999-11-31 00:00:00', '0', '-839:00:00', '1900', '1900', '2012-12-12 12:12:16');

# now test with the 5.6 behavior just to be sure
set @orig_sql_mode=@@session.sql_mode;
set session sql_mode="";
INSERT INTO t1 (d,dt,ts,t,y,y4,pk) VALUES
('999-13-32', '999-11-31 00:00:00', '0', '-839:00:00', '1900', '1900', '2012-12-12 12:12:16');
set session sql_mode=@orig_sql_mode;

SELECT d,dt,ts,t,y,y4 FROM t1;

DROP TABLE t1;
