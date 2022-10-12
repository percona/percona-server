# ==== Purpose ====
#
# Clean current audit log file content.
#
# ==== Usage ====
#
# --source clean_current_audit_log.inc
--disable_query_log
--disable_result_log
--let $clean_log_path = `SELECT @@global.datadir`
--let $clean_log_name = `SELECT @@global.audit_log_filter_file`

--remove_file $clean_log_path$clean_log_name
SELECT audit_log_rotate();
--enable_result_log
--enable_query_log
