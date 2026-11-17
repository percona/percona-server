# Run one query with a fresh slow query log and extract its tmp table figures.
#
# The measured query is run with the query log off so that the only output of
# this file is the two lines echoed by log_start.inc and log_stop.inc.
#
# Arguments:
# - tts_case:     label, also used to name the log file
# - $tts_query:   the statement to run and measure
# - tts_expected: free-form note recorded in the report file
#
# Sets $tmp_tables, $tmp_disk_tables, $tmp_table_sizes and
# $tmp_disk_table_sizes; see include/percona_slow_log_tmp_table_sizes.inc

--let log_file = percona_slow_query_log.tts_$tts_case
--source include/log_start.inc

--disable_query_log
--disable_result_log
--eval $tts_query
--enable_result_log
--enable_query_log

--source include/log_stop.inc
--source include/percona_slow_log_tmp_table_sizes.inc
