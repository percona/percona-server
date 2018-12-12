--let $MYSQLD_DATADIR=`select @@datadir`

--let $restart_parameters = "restart: --loose-$disable_native_partition_opt=0"
--source include/restart_mysqld.inc

eval CREATE TABLE t1 (a INT UNSIGNED NOT NULL, PRIMARY KEY(a)) ENGINE = $engine
PARTITION BY RANGE (a) (
    PARTITION p0 VALUES LESS THAN (100),
    PARTITION p1 VALUES LESS THAN (200)
);

--list_files $MYSQLD_DATADIR/test *.par

DROP TABLE t1;

--let $restart_parameters = "restart: "
--source include/restart_mysqld.inc

eval CREATE TABLE t1 (a INT UNSIGNED NOT NULL, PRIMARY KEY(a)) ENGINE = $engine
PARTITION BY RANGE (a) (
    PARTITION p0 VALUES LESS THAN (100),
    PARTITION p1 VALUES LESS THAN (200)
);

--list_files $MYSQLD_DATADIR/test *.par

DROP TABLE t1;
