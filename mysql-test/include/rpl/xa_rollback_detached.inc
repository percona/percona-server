# ==== Purpose ====
#
# Execute XA ROLLBACK on a replica for an XA transaction that the
# replica applier has prepared, retrying while the applier has not yet
# detached the transaction.
#
# XA RECOVER lists an XID as soon as XA PREPARE is visible
# (Bug#35872823), but XA ROLLBACK from another session finds the XID
# only after the applier worker has detached the transaction from its
# THD (applier_reset_xa_trans()). Until then XA ROLLBACK fails with
# ER_XAER_NOTA, even if sync_to_replica.inc and XA RECOVER succeeded.
#
# The statement is echoed once, exactly as a plain XA ROLLBACK would
# be, so the result file is the same as without this include.
#
# ==== Usage ====
#
# --let $xa_rollback_xid = 'a'
# --source include/rpl/xa_rollback_detached.inc
#
# Parameters:
#   $xa_rollback_xid
#     The XID, as written in the XA ROLLBACK statement.

if (!$xa_rollback_xid)
{
  --die !!!ERROR IN TEST: you must set $xa_rollback_xid
}

--echo XA ROLLBACK $xa_rollback_xid;
--disable_query_log
--let $_xa_rollback_retries = 300
while ($_xa_rollback_retries)
{
  --error 0,ER_XAER_NOTA
  eval XA ROLLBACK $xa_rollback_xid;
  if (!$mysql_errno)
  {
    --let $_xa_rollback_retries = 0
  }
  if ($mysql_errno)
  {
    --dec $_xa_rollback_retries
    if (!$_xa_rollback_retries)
    {
      --die XA ROLLBACK $xa_rollback_xid still fails with ER_XAER_NOTA after 30 seconds
    }
    --sleep 0.1
  }
}
--enable_query_log
--let $xa_rollback_xid =
