--disable_query_log
--let slow_query_log_old=`SELECT Variable_value FROM performance_schema.global_variables WHERE Variable_name = 'slow_query_log';`
SET GLOBAL slow_query_log=0;

--let slow_query_log_file_old=`SELECT Variable_value FROM performance_schema.global_variables WHERE Variable_name = 'slow_query_log_file';`
--let log_output_old=`SELECT Variable_value FROM performance_schema.global_variables WHERE Variable_name = 'log_output';`

SET GLOBAL log_output=FILE;

# Preload mysql.slow_log into DD cache
SET GLOBAL slow_query_log=1;
SET GLOBAL slow_query_log=0;

--enable_query_log
