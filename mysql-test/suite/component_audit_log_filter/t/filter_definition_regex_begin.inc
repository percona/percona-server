# ==== Purpose ====
#
# Start a measured interval: rotate the audit log so the following records
# go to a fresh file, closed and summarized by
# filter_definition_regex_summary.inc. Must be run from an account which is
# not audited.
#
# ==== Usage ====
#
# --source filter_definition_regex_begin.inc

--disable_query_log
--disable_result_log
SELECT audit_log_rotate();
--enable_result_log
--enable_query_log
