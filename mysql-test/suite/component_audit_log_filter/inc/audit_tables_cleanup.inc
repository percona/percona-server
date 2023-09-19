# ==== Purpose ====
#
# Remove the audit plugin configuration tables
#
# ==== Usage ====
#
# --let $audit_db_name = database name
# --source audit_tables_cleanup.inc
#
# Parameters:
#
# $audit_db_name
#   Database name, defaults to 'mysql'

if ($audit_db_name == '') {
--let $audit_db_name = mysql
}

--disable_query_log
--disable_result_log

eval DROP TABLE IF EXISTS $audit_db_name.audit_log_user;
eval DROP TABLE IF EXISTS $audit_db_name.audit_log_filter;

--enable_query_log
--enable_result_log
