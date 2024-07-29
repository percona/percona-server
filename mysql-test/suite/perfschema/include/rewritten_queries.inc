
#
# Checks to perform after each query.
#
# Parameters:
# - $con1_thread_id
# - $con1_connection_id
#

--disable_query_log
eval SELECT "thread" as mark, PROCESSLIST_INFO
  FROM performance_schema.threads
  WHERE THREAD_ID = $con1_thread_id;

--echo ---------------------------------------

eval SELECT "processlist" as mark, INFO
  FROM performance_schema.processlist
  WHERE ID = $con1_connection_id;

--echo ---------------------------------------

eval SELECT "I_S.PROCESSLIST" as mark, INFO
  FROM INFORMATION_SCHEMA.PROCESSLIST
  WHERE ID = $con1_connection_id;

--echo ---------------------------------------

eval SELECT "current" as mark, EVENT_NAME, SQL_TEXT, DIGEST_TEXT
  FROM performance_schema.events_statements_current
  WHERE THREAD_ID = $con1_thread_id;

--echo ---------------------------------------

eval SELECT "history" as mark, EVENT_NAME, SQL_TEXT, DIGEST_TEXT
  FROM performance_schema.events_statements_history
  WHERE THREAD_ID = $con1_thread_id
  ORDER BY EVENT_ID;

--echo ---------------------------------------

eval SELECT "history_long" as mark, EVENT_NAME, SQL_TEXT, DIGEST_TEXT
  FROM performance_schema.events_statements_history_long
  WHERE THREAD_ID = $con1_thread_id
  ORDER BY EVENT_ID;

--echo ---------------------------------------

--enable_query_log
