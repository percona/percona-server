#
# $expression         - an expression to evaluate
# $regexp             - a regular expression that the result should match (^ and $ are added automatically)
# $value              - a value that the result must be equal to
# $expected_charset   - the expected character set of the result
# $expected_collation - the expected collation of the result (optional)
# $hide_value         - if non-zero, then the $value will be hidden from the assertion statements
#
# Only one of the $regexp / $value should be specified - if both, $regexp has precedence
# if $expected_collation is not specified, then the default one for the given $expected_charset will be assumed

eval SET @check_expression_result = $expression;
if ($regexp) {
  --let $assert_cond = `SELECT REGEXP_LIKE(CONVERT(@check_expression_result USING utf8mb4), CONCAT(_utf8mb4 '^', _utf8mb4 '$regexp', _utf8mb4 0x24))`
  --let $assert_text = the result of evaluating '$expression' must match the '$regexp' pattern
}
if ($value) {
  --let $assert_cond = `SELECT CONVERT(@check_expression_result USING utf8mb4) = _utf8mb4 '$value'`
  --let $value_for_assert_text = $value
  if ($hide_value) {
    --let $value_for_assert_text = <hidden>
  }
  --let $assert_text = the result of evaluating '$expression' must be equal to '$value_for_assert_text'
}
--source include/assert.inc
--let $assert_cond = CHARSET(@check_expression_result) = "$expected_charset"
--let $assert_text = charset of the result of evaluating '$expression' must be '$expected_charset'
--source include/assert.inc
--let $fixed_expected_collation = $expected_collation
if (!$expected_collation) {
  --let $fixed_expected_collation = `SELECT COLLATION_NAME FROM INFORMATION_SCHEMA.COLLATIONS WHERE CHARACTER_SET_NAME = '$expected_charset' AND IS_DEFAULT = 'Yes'`
}
--let $assert_cond = COLLATION(@check_expression_result) = "$fixed_expected_collation"
--let $assert_text = collation of the result of evaluating '$expression' must be '$fixed_expected_collation'
--source include/assert.inc
