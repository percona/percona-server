# Usage:
# let $max = <count>;
# let $table = <table name>;
# --source suite/rocksdb/include/drop_table_repopulate_table.inc
#
eval DELETE FROM $table;

--disable_query_log
# Insert in one transaction: a commit per row would sync the WAL and the
# binlog tens of thousands of times and make the test time out on slow
# storage or a loaded machine.
BEGIN;
let $i = 1;
while ($i <= $max) {
  let $insert = INSERT INTO $table VALUES ($i, $i);
  inc $i;
  eval $insert;
}
COMMIT;
--enable_query_log
