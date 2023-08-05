--source include/rpl/connection_source.inc
--let $binlog_format = `SELECT @@binlog_format`
CREATE TABLE t(vl VARCHAR(64) NOT NULL);
eval INSERT INTO t VALUES($check_determinism_expression);
# number of warnings and worning code must be extracted here, immediately
# after the INSERT statement so that any subsequent assert.inc will not
# overwrite these values
--let $warnings_count = `SHOW COUNT(*) WARNINGS`
if ($warnings_count == 1) {
  --let $warning_code = query_get_value(SHOW WARNINGS, Code, 1)
}
if ($binlog_format != STATEMENT) {
  --let $assert_cond = $warnings_count = 0
  --let $assert_text = The "$check_determinism_expression" must not generate any warnings when binlog_format is ROW / MIXED
  --source include/assert.inc
}
if ($binlog_format == STATEMENT) {
  --let $assert_cond = $warnings_count = 1
  --let $assert_text = The "$check_determinism_expression" must generate exactly one warning when binlog_format is STATEMENT
  --source include/assert.inc

  --let $expected_warning_code = convert_error(ER_BINLOG_UNSAFE_STATEMENT)
  --let $assert_cond = $warning_code = $expected_warning_code
  --let $assert_text = The "$check_determinism_expression" must generate the ER_BINLOG_UNSAFE_STATEMENT warning when binlog_format is STATEMENT
  --source include/assert.inc
}
--let $result = `SELECT vl FROM t`

--source include/rpl/sync_to_replica.inc
if ($binlog_format != STATEMENT) {
  --let $comparison_result = `SELECT vl = '$result' FROM t`
  --let $assert_cond = $comparison_result
  --let $assert_text = The result of "$check_determinism_expression" on primary and replica must match when binlog_format is ROW / MIXED
  --source include/assert.inc
}

--source include/rpl/connection_source.inc
DROP TABLE t;
--source include/rpl/sync_to_replica.inc
