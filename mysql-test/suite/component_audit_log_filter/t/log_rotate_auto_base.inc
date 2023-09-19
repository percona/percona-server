--source ../inc/audit_tables_init.inc
--source ../inc/audit_comp_install.inc

SELECT audit_log_filter_set_filter('log_all', '{"filter": {"log": true}}');
SELECT audit_log_filter_set_user('%', 'log_all');

--source clean_all_audit_logs.inc

--source generate_audit_events.inc
--source generate_audit_events.inc
--source generate_audit_events.inc
--source generate_audit_events.inc
--source generate_audit_events.inc

--let $audit_filter_log_path = `SELECT @@global.datadir`
--let $audit_filter_log_name = `SELECT @@global.audit_log_filter.file`
--let $audit_filter_log_format = xml
--source validate_logs_format.inc

--echo #
--echo # Cleanup
--source ../inc/audit_comp_uninstall.inc
--source ../inc/audit_tables_cleanup.inc
