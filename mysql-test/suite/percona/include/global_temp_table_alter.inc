--source include/count_sessions.inc

if (!$create_table_query) {
  --die "please set the variable $create_table_query to a valid query"
}

if (!$alter_table_query) {
  --die "please set the variable $alter_table_query to a valid query"
}

if (!$drop_table_query) {
  --die "please set the variable $alter_table_query to a valid query"
}

connect (con1, localhost, root,,);
--connection default
--eval $create_table_query


SET DEBUG_SYNC="alter_before_close_temp_tables SIGNAL enter_alter_table_before_close_temp_tables WAIT_FOR continue_commit_alter_ddl";
--send_eval $alter_table_query

--connection con1
SET DEBUG_SYNC="now WAIT_FOR enter_alter_table_before_close_temp_tables";
--replace_column 1 SESSION_ID 3 TABLE_NAME 5 NAME
SELECT * FROM information_schema.global_temporary_tables;
SET DEBUG_SYNC="now SIGNAL continue_commit_alter_ddl";
--connection default
--reap
--eval $drop_table_query
--disconnect con1

--source include/wait_until_count_sessions.inc
