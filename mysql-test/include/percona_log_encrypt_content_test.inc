
CREATE TABLE t1(c1 INT, c2 char(20)) ENGINE = InnoDB;

INSERT INTO t1 VALUES(0, "asdfghjkl");
INSERT INTO t1 VALUES(1, "qwertyuio");
INSERT INTO t1 VALUES(2, "zxcvbnm");

# Check file content
--let $MYSQLD_DATADIR= `select @@datadir`
--let SEARCH_PATTERN= asdfghjkl
-- let SEARCH_FILE= $MYSQLD_DATADIR/ib_logfile1
-- source include/search_pattern_in_file.inc

DROP TABLE t1;
