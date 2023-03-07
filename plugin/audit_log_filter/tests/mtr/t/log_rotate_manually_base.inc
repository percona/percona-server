
--disable_query_log
call mtr.add_suppression("Failed to shutdown components infrastructure.");
--enable_query_log

SELECT audit_log_filter_set_filter('log_general', '{"filter": {"class": {"name": "general"}}}');
SELECT audit_log_filter_set_user('%', 'log_general');

--source clean_current_audit_log.inc
--source generate_audit_events.inc

--replace_column 1 <file_name>
SELECT audit_log_rotate();

--source generate_audit_events.inc

--replace_column 1 <file_name>
SELECT audit_log_rotate();

--let $audit_filter_log_path = `SELECT @@global.datadir`
--source validate_logs_format.inc

--echo #
--echo # Cleanup
DELETE FROM mysql.audit_log_user;
DELETE FROM mysql.audit_log_filter;
SELECT audit_log_filter_flush();
