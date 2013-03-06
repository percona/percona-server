--let slow_query_log_file_old=`SELECT Variable_value FROM INFORMATION_SCHEMA.GLOBAL_VARIABLES WHERE Variable_name = 'slow_query_log_file';`
--let slow_query_log_old=`SELECT Variable_value FROM INFORMATION_SCHEMA.GLOBAL_VARIABLES WHERE Variable_name = 'slow_query_log';`
--let log_output_old=`SELECT Variable_value FROM INFORMATION_SCHEMA.GLOBAL_VARIABLES WHERE Variable_name = 'log_output';`
--disable_query_log
--let log_file_full_path = $MYSQLTEST_VARDIR/$log_file.slog
SET GLOBAL slow_query_log=0;
SET GLOBAL log_output=FILE;
--echo [log_start.inc] $log_file
EVAL SET GLOBAL slow_query_log_file="$log_file_full_path";
SET GLOBAL slow_query_log=1;
--enable_query_log
