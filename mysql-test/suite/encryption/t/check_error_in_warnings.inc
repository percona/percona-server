--let $warning_message= query_get_value(SHOW WARNINGS, Message, 1)
--let $warning_count= `SHOW COUNT(*) WARNINGS`
--let $warning_level= query_get_value(SHOW WARNINGS, Level, 1)
# The message may contain a single quote ('), which confuses
# include/assert.inc So we remove any single quotes.
--let $warning_message= `SELECT REPLACE("$warning_message", "'", "")`
--let $assert_text= One error message should be generated.
--let $assert_cond= $warning_count = 1
--source include/assert.inc
--let $assert_text= It should be an Error, not a Warning.
--let $assert_cond= "$warning_level" = "Error"
--source include/assert.inc
--let $assert_text= It should contain text
--let $assert_cond= "$warning_message" LIKE "$expected_error_message"
--source include/assert.inc

