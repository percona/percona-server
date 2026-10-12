# Check that the XA transaction 'z2', prepared with
# rocksdb_flush_log_at_trx_commit = 0 and inserting (21, 21) into t1, was
# either kept prepared or lost entirely by a kill. It is committed if it
# was kept.
--let $xa_data = query_get_value(XA RECOVER, data, 1)
--let $xa_kept = 0
--let $assert_text = The row is not visible before the transaction is committed
--let $assert_cond = [SELECT COUNT(*) FROM t1 WHERE a = 21] = 0
--source include/assert.inc
if ($xa_data == 'z2')
{
  --let $xa_kept = 1
  --disable_query_log
  XA COMMIT 'z2';
  --enable_query_log
}
--let $xa_rows = `SELECT COUNT(*) FROM t1 WHERE a = 21`
--let $assert_text = The transaction is either kept prepared or lost with its row
--let $assert_cond = $xa_rows = $xa_kept
--source include/assert.inc
--let $assert_text = Nothing else is prepared
--let $assert_cond = "[XA RECOVER, data, 1]" = "No such row"
--source include/assert.inc
DELETE FROM t1 WHERE a = 21;
