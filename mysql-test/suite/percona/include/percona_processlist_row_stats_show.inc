--replace_column 1 ### 3 ### 6 ### 7 ### 9 ###
SHOW PROCESSLIST;

--replace_column 1 ###
SELECT id, info, rows_sent, rows_examined FROM performance_schema.processlist ORDER BY id;

SET DEBUG_SYNC= 'now SIGNAL threads_dumped';
