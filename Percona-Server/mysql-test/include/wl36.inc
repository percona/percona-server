#
# WL #36: http://askmonty.org/worklog/Server-Sprint/?tid=36
# This is basic test for --rewrite-db option.
# mysqlbinlog --rewrite-db="from->to" should rewrite database
#

CREATE DATABASE a; USE a; CREATE TABLE t1(a INT);
CREATE DATABASE b; USE b; CREATE TABLE t1(b INT);
CREATE DATABASE c; USE c; CREATE TABLE t1(c INT);

RESET MASTER;
--let MYSQLD_DATA_DIR=`select @@datadir`
--let BIN_LOG_FILE_NAME=query_get_value(show master status, File,    1)
--let BIN_LOG_START_POSITION=query_get_value(show master status, Position, 1)
USE a; INSERT INTO t1 VALUES(0);
USE b; INSERT INTO t1 VALUES(1);
--let BIN_LOG_STOP_POSITION=query_get_value(show master status, Position, 1)
FLUSH LOGS;

--let BIN_LOG_FULL_PATH=`SELECT CONCAT("$MYSQLD_DATA_DIR", "$BIN_LOG_FILE_NAME")`

--let i=3
# i=3: rewrite nothing
# i=2: rewrite a->c
# i=0: rewrite a->b
--disable_query_log
while ($i)
{
--let REWRITE=`SELECT CASE $i WHEN 3 THEN '' WHEN 2 THEN '--rewrite-db="a->c"' WHEN 1 THEN '--rewrite-db="a->b"' END`

USE a; DELETE FROM t1;
USE b; DELETE FROM t1;
USE c; DELETE FROM t1;

--echo #
--echo # Apply log with '$REWRITE'
--echo #

--exec $MYSQL_BINLOG $REWRITE --start-position=$BIN_LOG_START_POSITION --stop-position=$BIN_LOG_STOP_POSITION $BIN_LOG_FULL_PATH | $MYSQL

USE a; SELECT * FROM t1;
USE b; SELECT * FROM t1;
USE c; SELECT * FROM t1;

dec $i;
}
--enable_query_log

USE a; DROP TABLE t1; DROP DATABASE a;
USE b; DROP TABLE t1; DROP DATABASE b;
USE c; DROP TABLE t1; DROP DATABASE c;
