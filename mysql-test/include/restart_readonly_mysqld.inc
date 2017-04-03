# Change buffer must be empty before restarting server as read-only
# If the innodb_fast_shutdown=0, InnoDB does a full purge and an insert buffer
# merge before a shutdown
--disable_query_log
SET GLOBAL innodb_fast_shutdown=0;
--enable_query_log

--let $_restart_readonly_mysqld_saved_restart_parameters= $restart_parameters

--let $restart_parameters=restart: --innodb-read-only
--source include/restart_mysqld.inc

--let $restart_parameters= $_restart_readonly_mysqld_saved_restart_parameters
