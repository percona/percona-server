--source include/have_rocksdb.inc

#
# Basic LOAD DATA statements
#

CREATE TABLE t1 (a INT, b CHAR(8), pk INT AUTO_INCREMENT PRIMARY KEY) ENGINE=rocksdb;

let $datadir = `SELECT @@datadir`;

--write_file $datadir/se_loaddata.dat
1,foo,
2,bar,
3,,
4,abc,
EOF

--replace_result $datadir <DATADIR>
eval
LOAD DATA INFILE '$datadir/se_loaddata.dat' INTO TABLE t1
  FIELDS TERMINATED BY ',' (a,b);
--sorted_result
SELECT a,b FROM t1;

--replace_result $datadir <DATADIR>
eval
LOAD DATA LOCAL INFILE '$datadir/se_loaddata.dat' INTO TABLE t1
  CHARACTER SET utf8 COLUMNS TERMINATED BY ','
  ESCAPED BY '/' (a,b);
--sorted_result
SELECT a,b FROM t1;

--remove_file $datadir/se_loaddata.dat
--write_file $datadir/se_loaddata.dat
5;YYY;
102;'zzz';
0;'test';
EOF

--replace_result $datadir <DATADIR>
eval
LOAD DATA LOCAL INFILE '$datadir/se_loaddata.dat' INTO TABLE t1
  FIELDS TERMINATED BY ';'
  (a) SET b='loaded';

--sorted_result
SELECT a,b FROM t1;

--remove_file $datadir/se_loaddata.dat
--write_file $datadir/se_loaddata.dat
prefix:5;'foo';
prefix:6;'';
prefix:100;foo;
prefix:7;'test';suffix
101;abc;
102;'z';
prefix:0;;
EOF

# 5.7 uses strict mode by default, so this will now return an error
# Since we can't --replace_result and --error on the same command, we must
# split them somehow so we use send_eval and reap
--replace_result $datadir <DATADIR>
send_eval
LOAD DATA INFILE '$datadir/se_loaddata.dat' INTO TABLE t1
  FIELDS TERMINATED BY ';'
  OPTIONALLY ENCLOSED BY ''''
  LINES STARTING BY 'prefix:'
  IGNORE 2 LINES (a,b);

--error ER_WARN_TOO_MANY_RECORDS
reap;

# now test with the 5.6 behavior just to be sure
set @orig_sql_mode=@@session.sql_mode;
set session sql_mode="";
--replace_result $datadir <DATADIR>
eval
LOAD DATA INFILE '$datadir/se_loaddata.dat' INTO TABLE t1
  FIELDS TERMINATED BY ';'
  OPTIONALLY ENCLOSED BY ''''
  LINES STARTING BY 'prefix:'
  IGNORE 2 LINES (a,b);
set session sql_mode=@orig_sql_mode;

--sorted_result
SELECT a,b FROM t1;

--remove_file $datadir/se_loaddata.dat
--write_file $datadir/se_loaddata.dat
1	foo
2	bar
3
4	abc
EOF

# 5.7 uses strict mode by default, so this will now return an error
# Since we can't --replace_result and --error on the same command, we must
# split them somehow so we use send_eval and reap
--replace_result $datadir <DATADIR>
send_eval
LOAD DATA INFILE '$datadir/se_loaddata.dat' INTO TABLE t1;

--error ER_WARN_TOO_FEW_RECORDS
reap;

# now test with the 5.6 behavior just to be sure
set @orig_sql_mode=@@session.sql_mode;
set session sql_mode="";
--replace_result $datadir <DATADIR>
eval
LOAD DATA INFILE '$datadir/se_loaddata.dat' INTO TABLE t1;
set session sql_mode=@orig_sql_mode;

--sorted_result
SELECT a,b FROM t1;
--remove_file $datadir/se_loaddata.dat


--write_file $datadir/se_replacedata.dat
1	aaa
2	bbb
3	ccc
4	ddd
EOF

if ($skip_unique_check == 1)
{
  # 5.7 uses strict mode by default, so this will now return an error
  # Since we can't --replace_result and --error on the same command, we must
  # split them somehow so we use send_eval and reap
  --replace_result $datadir <DATADIR>
  send_eval
  LOAD DATA INFILE '$datadir/se_replacedata.dat' REPLACE INTO TABLE t1;
  # we need this --replace_result here because the --error will contain
  # the complete ^^ query, including the path to the data file, which will
  # potentially be different on every run
  --replace_result $datadir <DATADIR>
  --error ER_UNKNOWN_ERROR
  reap;
}
if ($skip_unique_check == 0)
{
  # 5.7 uses strict mode by default, so this will now return an error
  # Since we can't --replace_result and --error on the same command, we must
  # split them somehow so we use send_eval and reap
  --replace_result $datadir <DATADIR>
  send_eval
  LOAD DATA INFILE '$datadir/se_replacedata.dat' REPLACE INTO TABLE t1;

  --error ER_WARN_TOO_FEW_RECORDS
  reap;

  # now test with the 5.6 behavior just to be sure
  set @orig_sql_mode=@@session.sql_mode;
  set session sql_mode="";
  --replace_result $datadir <DATADIR>
  eval
  LOAD DATA INFILE '$datadir/se_replacedata.dat' REPLACE INTO TABLE t1;
  set session sql_mode=@orig_sql_mode;
}
--sorted_result
SELECT a,b FROM t1;
--remove_file $datadir/se_replacedata.dat


# Cleanup
DROP TABLE t1;
