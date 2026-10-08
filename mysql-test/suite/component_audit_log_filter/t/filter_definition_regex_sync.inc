# ==== Purpose ====
#
# Wait until audited sessions have finished processing their statements.
# The server sends the statement result to the client before the
# general/status event is notified and switches the session to 'Sleep'
# only afterwards, so a client receiving a result does not imply that the
# statement's audit record was evaluated and written yet.
#
# ==== Usage ====
#
# --source filter_definition_regex_sync.inc

--disable_query_log
let $wait_condition = SELECT COUNT(*) = 0 FROM performance_schema.threads
  WHERE TYPE = 'FOREGROUND' AND PROCESSLIST_USER IS NOT NULL
    AND PROCESSLIST_USER <> 'root'
    AND PROCESSLIST_COMMAND NOT IN ('Sleep', 'Daemon');
--source include/wait_condition.inc
--enable_query_log
