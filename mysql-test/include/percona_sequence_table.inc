# $st_field_expression           - the list of fileds which will be passed to a SELECT $st_field_expression FROM ...
# $st_table_expression           - a table function expression which will be passed to a SELECT ... FROM $st_table_expression
#                                  (do not use single quotes here as they will not be interpolated properly by assert.inc)
# $st_where                      - optional WHERE clause that will be added to SELECT $st_field_expression FROM $st_table_expression
# $st_expected_number_of_records - an expected number of records returned
# $st_expected_signature         - a comma-separated list of expected returned values
# $st_sort_order                 - sort_order: <empty>, ASC or DESC (used in GROUP_CONCAT() ORDER BY clause)

--let $st_query_suffix = FROM $st_table_expression
if ($st_where)
{
  --let $st_query_suffix = $st_query_suffix WHERE $st_where
}

--echo ### SELECT $st_field_expression $st_query_suffix ###
--eval SELECT $st_field_expression $st_query_suffix
--let $assert_cond = "[SELECT COUNT(*) $st_query_suffix]" = $st_expected_number_of_records
--let $assert_text = [SELECT COUNT(*) $st_query_suffix] should return $st_expected_number_of_records records
--source include/assert.inc

--let $assert_cond = "[SELECT IFNULL(GROUP_CONCAT($st_field_expression ORDER BY 1 $st_sort_order SEPARATOR ","), "") $st_query_suffix]" = "$st_expected_signature"
--let $assert_text = [SELECT $st_field_expression $st_query_suffix] should return "$st_expected_signature"
--source include/assert.inc

--eval EXPLAIN SELECT $st_field_expression $st_query_suffix
--eval EXPLAIN FORMAT=TREE SELECT $st_field_expression $st_query_suffix

--echo
