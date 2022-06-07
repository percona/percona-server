#
# A helper include file for prefix index index-only query tests
#
# Parameters:
# $prefix_index_check_title - title of the test
# $prefix_index_check_query - test query
# $prefix_index_check_read_delta - expected change of
#   'innodb_secondary_index_triggered_cluster_reads' status variable value after
#   running the query
# $prefix_index_check_read_avoided_delta - expected change of
#   'innodb_secondary_index_triggered_cluster_reads_avoided' status variable
#   value after running the query

--let $show_count_statement = show status like 'innodb_secondary_index_triggered_cluster_reads'
--let $show_opt_statement = show status like 'innodb_secondary_index_triggered_cluster_reads_avoided'

--echo # $prefix_index_check_title
--let $base_count = query_get_value($show_count_statement, Value, 1)
--let $base_opt = query_get_value($show_opt_statement, Value, 1)

--eval $prefix_index_check_query

--let $count = query_get_value($show_count_statement, Value, 1)
--let $assert_text= $prefix_index_check_title: $prefix_index_check_read_delta innodb_secondary_index_triggered_cluster_reads
--let $assert_cond= $count - $base_count = $prefix_index_check_read_delta
--source include/assert.inc

--let $opt = query_get_value($show_opt_statement, Value, 1)
--let $assert_text= $prefix_index_check_title: $prefix_index_check_read_avoided_delta innodb_secondary_index_triggered_cluster_reads_avoided
--let $assert_cond= $opt - $base_opt = $prefix_index_check_read_avoided_delta
--source include/assert.inc

