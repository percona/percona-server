--echo Creating auxiliary 'test_hint' SP which returns variable value before hint, with hint and after hint applying

--disable_query_log
DELIMITER \\;

CREATE PROCEDURE test_hint (hint_str VARCHAR(255), var_str VARCHAR(64))
BEGIN

SET @orig_q= CONCAT("SELECT VARIABLE_VALUE INTO @before_val FROM performance_schema.session_variables where VARIABLE_NAME = '",  var_str, "'");

SET @hint_q= CONCAT("SELECT /*+ ", hint_str,
                    "*/ VARIABLE_VALUE" ,
                    " INTO @hint_val FROM performance_schema.session_variables where VARIABLE_NAME = '",  var_str, "'");

SET @after_q= CONCAT("SELECT VARIABLE_VALUE INTO @after_val FROM performance_schema.session_variables where VARIABLE_NAME = '",  var_str, "'");

PREPARE orig_stmt FROM @orig_q;
PREPARE hint_stmt FROM @hint_q;
PREPARE after_stmt FROM @after_q;

EXECUTE orig_stmt;
EXECUTE hint_stmt;
EXECUTE after_stmt;

SELECT  hint_str;
SELECT @before_val, @hint_val, @after_val;

DEALLOCATE PREPARE orig_stmt;
DEALLOCATE PREPARE hint_stmt;
DEALLOCATE PREPARE after_stmt;

END\\

DELIMITER ;\\

--enable_query_log
