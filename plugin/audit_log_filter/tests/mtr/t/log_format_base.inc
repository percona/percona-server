--let $audit_filter_log_path = `SELECT @@global.datadir`
--let $audit_filter_log_name = `SELECT @@global.audit_log_filter_file`

SELECT audit_log_filter_set_filter('log_all', '{"filter": {"log": true}}');
SELECT audit_log_filter_set_user('%', 'log_all');

--source generate_audit_events.inc

# rotate log manually for it to properly close and have closing tags added
--replace_column 1 <file_name>
SELECT audit_log_rotate();

if (`SELECT @@global.audit_log_filter_encryption = 'AES'`) {
--let $encryption_options = `SELECT audit_log_encryption_password_get()`
--let $encryption_password = `SELECT JSON_EXTRACT('$encryption_options', '$.password')`
--let $encryption_iterations = `SELECT JSON_EXTRACT('$encryption_options', '$.iterations')`

--source decrypt_rotated_log.inc
}

if (`SELECT @@global.audit_log_filter_compression = 'GZIP'`) {
--source decompress_rotated_log.inc
}

--let $audit_filter_log_format = `SELECT IF(STRCMP(SUBSTRING_INDEX(@@global.audit_log_filter_file, "xml", 1), @@global.audit_log_filter_file), "xml", "json")`
--source validate_logs_format.inc

--echo #
--echo # Cleanup
DELETE FROM mysql.audit_log_user;
DELETE FROM mysql.audit_log_filter;
SELECT audit_log_filter_flush();
