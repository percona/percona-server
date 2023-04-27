# ==== Purpose ====
#
# Remove all rotated audit log files and clean current log.
#
# ==== Usage ====
#
# --source clean_all_audit_logs.inc

--disable_query_log
--disable_result_log

--let $rotated_logs_path = `SELECT @@global.datadir`
--replace_regex /([^\.]*)\..*/\1/
--let $rotated_logs_base_name = `SELECT @@global.audit_log_filter_file`
--remove_files_wildcard $rotated_logs_path $rotated_logs_base_name*
SELECT audit_log_rotate();

--enable_result_log
--enable_query_log
