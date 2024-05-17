--source include/rpl/connection_source.inc
create table t1 (id int primary key, value int) engine = rocksdb;

insert into t1 values (1,1),(2,2),(3,3);

# auto commit failure at engine commit, crash and restart (roll forward with binlog)
--exec echo "restart" > $MYSQLTEST_VARDIR/tmp/mysqld.1.expect
eval set session debug= $myrocks_debug_set_code;
--error 2013
insert into t1 values (4,4);

# Primary instance restart needs this, otherwise rpl_end fails.
--let $rpl_server_number = 1
--source include/rpl/reconnect.inc

select * from t1 order by id;
source include/rpl/sync_to_replica.inc;

--source include/rpl/connection_replica.inc
select * from t1 order by id;


--source include/rpl/connection_source.inc
# tx failure at engine commit, crash and restart (roll forward with binlog)
--exec echo "restart" > $MYSQLTEST_VARDIR/tmp/mysqld.1.expect
eval set session debug= $myrocks_debug_set_code;
begin;
insert into t1 values (5,5);
insert into t1 values (6,6);
--error 2013
commit;

# Primary instance restart needs this, otherwise rpl_end fails.
--let $rpl_server_number = 1
--source include/rpl/reconnect.inc

select * from t1 order by id;
source include/rpl/sync_to_replica.inc;

--source include/rpl/connection_replica.inc
select * from t1 order by id;


--source include/rpl/connection_source.inc
drop table t1;
source include/rpl/sync_to_replica.inc;

