select @@rocksdb_default_cf_options;
create table t1(id int primary key, data int) engine=rocksdb;

--disable_query_log
let $i = 1;
while ($i <= 10000) {
  eval insert into t1 values ($i, $i);
  inc $i;
}
--enable_query_log

set global rocksdb_force_flush_memtable_now = true;
set global rocksdb_compact_cf = 'default';

select SUM(id), SUM(data) from t1;

select cf_name, option_type, value from information_schema.rocksdb_cf_options where cf_name='default' and option_type like 'COMPRESSION%' order by option_type;
select table_name, index_name, cf from information_schema.rocksdb_ddl where table_name='t1' order by table_name, index_name;

drop table t1;
