--source ../inc/audit_tables_init.inc
--source ../inc/audit_comp_install.inc

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
--let $audit_filter_log_name = `SELECT @@global.audit_log_filter.file`
--let $audit_filter_log_format = xml
--source validate_logs_format.inc

--echo #
--echo # Cleanup
--source ../inc/audit_comp_uninstall.inc
--source ../inc/audit_tables_cleanup.inc
