--disable_query_log
--disable_result_log
SET GLOBAL QUERY_RESPONSE_TIME_STATS = 0;
SET GLOBAL QUERY_RESPONSE_TIME_FLUSH = 1;
SET SESSION QUERY_RESPONSE_TIME_EXEC_TIME_DEBUG = 500000;
--enable_query_log
--enable_result_log
SELECT * FROM INFORMATION_SCHEMA.QUERY_RESPONSE_TIME WHERE count != 0;
SELECT * FROM INFORMATION_SCHEMA.QUERY_RESPONSE_TIME_READ WHERE count != 0;
SELECT * FROM INFORMATION_SCHEMA.QUERY_RESPONSE_TIME_WRITE WHERE count != 0;
--disable_query_log
--disable_result_log
SET GLOBAL QUERY_RESPONSE_TIME_STATS = 1;
--enable_query_log
--enable_result_log
--echo ------------------Test body begin--------------------
