--source ../inc/audit_tables_init.inc
--source ../inc/audit_comp_install.inc
--source clean_all_audit_logs.inc

--let $audit_filter_log_path = `SELECT @@global.datadir`

--echo #
--echo # before shutdown, one active log
--replace_regex /\d{8}T\d{6}-\d*/<keyring_id>/ /\d{8}T\d{6}/<timestamp>/
--list_files $audit_filter_log_path audit_filter.*

--echo #
--echo # after shutdown, one rotated log
--source include/shutdown_mysqld.inc
--replace_regex /\d{8}T\d{6}-\d*/<keyring_id>/ /\d{8}T\d{6}/<timestamp>/
--list_files $audit_filter_log_path audit_filter.*

--echo #
--echo # after restart, one active and one rotated log
--let $restart_parameters="restart: "
--source include/start_mysqld.inc
--sorted_result
--replace_regex /\d{8}T\d{6}-\d*/<keyring_id>/ /\d{8}T\d{6}/<timestamp>/
--list_files $audit_filter_log_path audit_filter.*

--source ../inc/audit_comp_uninstall.inc
--source ../inc/audit_tables_cleanup.inc
