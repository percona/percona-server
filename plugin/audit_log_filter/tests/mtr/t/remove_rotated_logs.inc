# ==== Purpose ====
#
# Remove rotated audit log files.
#
# ==== Usage ====
#
# --source remove_rotated_logs.inc

--disable_query_log
--disable_result_log

--let $rotated_logs_path = `SELECT @@global.datadir`
--replace_regex /([^\.]*)\..*/\1/
--let $rotated_logs_base_name = `SELECT @@global.audit_log_filter_file`
--remove_files_wildcard $rotated_logs_path $rotated_logs_base_name.????????T??????.*

--enable_result_log
--enable_query_log
