--disable_query_log
FLUSH LOGS;
SET GLOBAL slow_query_log=OFF;
--echo [log_stop.inc] $log_file
SET GLOBAL slow_query_log_file= @slow_query_log_file_old;
SET GLOBAL slow_query_log=      @slow_query_log_old;
--enable_query_log
