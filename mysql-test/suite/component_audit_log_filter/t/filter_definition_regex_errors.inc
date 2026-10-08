# ==== Purpose ====
#
# Print the change of Audit_log_filter_regex_match_errors since the previous
# call (or since $rx_errors_base was reset).
#
# ==== Usage ====
#
# --source filter_definition_regex_errors.inc

--source filter_definition_regex_sync.inc
--let $_rx_errors_now = query_get_value(SHOW GLOBAL STATUS LIKE 'Audit_log_filter_regex_match_errors', Value, 1)
if ($rx_errors_base == '') {
  --let $rx_errors_base = $_rx_errors_now
}
--expr $_rx_errors_delta = $_rx_errors_now - $rx_errors_base
--echo # Regex match errors: +$_rx_errors_delta
--let $rx_errors_base = $_rx_errors_now
