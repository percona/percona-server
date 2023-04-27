SELECT audit_log_filter_set_filter('log_all', '{"filter": {"log": true}}');
SELECT audit_log_filter_set_user('%', 'log_all');

--source generate_audit_events.inc
--source generate_audit_events.inc
--source generate_audit_events.inc
--source generate_audit_events.inc
--source generate_audit_events.inc

--let $audit_filter_log_path = `SELECT @@global.datadir`
--let $audit_filter_log_name = `SELECT @@global.audit_log_filter_file`
--let $audit_filter_log_format = xml
--source validate_logs_format.inc

--echo #
--echo # Cleanup
DELETE FROM mysql.audit_log_user;
DELETE FROM mysql.audit_log_filter;
SELECT audit_log_filter_flush();
