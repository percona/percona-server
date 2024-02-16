--let $fts_parser_clause =
if ($fts_parser != '')
{
  --let $fts_parser_clause = WITH PARSER $fts_parser
}

--echo *** saving global system variables
SET @old_innodb_optimize_fulltext_only = @@global.innodb_optimize_fulltext_only;
SET GLOBAL innodb_optimize_fulltext_only = ON;

SET @old_innodb_ft_aux_table = @@global.innodb_ft_aux_table;

--echo
--echo *** creating a simple table with a full text index
SET innodb_ft_enable_stopword = OFF;
eval CREATE TABLE t1 (c TINYTEXT, FULLTEXT KEY (c) $fts_parser_clause) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
SET GLOBAL innodb_ft_aux_table = 'test/t1';

--echo
--echo ***************************************
--echo *** Part I: original crash scenario ***
--echo ***************************************

--echo
--echo *** inserting a record containing the '%' character
SET @special_string = 'vdf%vdfd%ghdi%opu';
--let $special_string = `SELECT @special_string`
# for both default and mecab parser
if ($fts_parser != ngram)
{
  # there are 4 words separated by the '%' character
  --let $expected_number_of_records = 4
}
if ($fts_parser == ngram)
{
  --let $expected_number_of_records = `SELECT LENGTH(@special_string) - 1`
}
INSERT INTO t1 VALUES (@special_string);

--let $assert_text = number of records in the index cache after inserting '$special_string' is expected to be $expected_number_of_records
--let $assert_cond = [SELECT COUNT(*) FROM INFORMATION_SCHEMA.INNODB_FT_INDEX_CACHE] = $expected_number_of_records
--source include/assert.inc
SELECT WORD FROM INFORMATION_SCHEMA.INNODB_FT_INDEX_CACHE ORDER BY WORD;

--let $assert_text = number of records in the index table after inserting '$special_string' is expected to be zero
--let $assert_cond = [SELECT COUNT(*) FROM INFORMATION_SCHEMA.INNODB_FT_INDEX_TABLE] = 0
--source include/assert.inc

OPTIMIZE TABLE t1;

--echo
--echo *** updating the table with another record also containing the '%' character
SET @special_string = 'sd%he%ff';
--let $special_string = `SELECT @special_string`
--let $old_expected_number_of_records = $expected_number_of_records
# for both default and mecab parser
if ($fts_parser != ngram)
{
  --let $expected_number_of_records = 3
}
if ($fts_parser == ngram)
{
  --let $expected_number_of_records = `SELECT LENGTH(@special_string) - 1`
}
UPDATE t1 SET c = @special_string;

--let $assert_text = number of records in the index cache after updating to '$special_string' is expected to be $expected_number_of_records
--let $assert_cond = [SELECT COUNT(*) FROM INFORMATION_SCHEMA.INNODB_FT_INDEX_CACHE] = $expected_number_of_records
--source include/assert.inc
SELECT WORD FROM INFORMATION_SCHEMA.INNODB_FT_INDEX_CACHE ORDER BY WORD;

--let $assert_text = number of records in the index table after updating to '$special_string' is expected to be $old_expected_number_of_records
--let $assert_cond = [SELECT COUNT(*) FROM INFORMATION_SCHEMA.INNODB_FT_INDEX_TABLE] = $old_expected_number_of_records
--source include/assert.inc
SELECT WORD FROM INFORMATION_SCHEMA.INNODB_FT_INDEX_TABLE ORDER BY WORD;

OPTIMIZE TABLE t1;

--let $assert_text = number of records in the index cache after updating to '$special_string' and optimizing is expected to be zero
--let $assert_cond = [SELECT COUNT(*) FROM INFORMATION_SCHEMA.INNODB_FT_INDEX_CACHE] = 0
--source include/assert.inc

--let $assert_text = number of records in the index table after updating to '$special_string' and optimizing is expected to be $expected_number_of_records
--let $assert_cond = [SELECT COUNT(*) FROM INFORMATION_SCHEMA.INNODB_FT_INDEX_TABLE] = $expected_number_of_records
--source include/assert.inc
SELECT WORD FROM INFORMATION_SCHEMA.INNODB_FT_INDEX_TABLE ORDER BY WORD;

--echo
--echo *** deleting the record
DELETE FROM t1;
OPTIMIZE TABLE t1;

--echo
--echo **************************************************
--echo *** Part II: indexing other special characters ***
--echo **************************************************

--echo
--echo *** creating a list of all printable characters (ASCII 33..126)
--echo *** (whitespace and control characters are excluded)

SELECT GROUP_CONCAT(CHAR(value + 33 USING utf8mb4) SEPARATOR '') INTO @special_characters FROM SEQUENCE_TABLE(127 - 33) AS tt;
SELECT @special_characters;

--echo
--echo *** for each character from the set we create a string containing this character
--echo *** and perform SELECTs with various MATCH() ... AGAINST() clauses (both in
--echo *** NATURAL LANGUAGE and BOOLEAN modes)

--echo
--echo *** please note that it is totally OK that some of these checks do not return
--echo *** the result we call "expected" - a number of characters have special meaning
--echo *** (especially in BOOLEAN mode) - our goal here is to test for crashes

--echo
--echo *** also we expect the number of mismatches to be much lower when
--echo *** 'ft_query_extra_word_chars' is set to 'ON'
--echo

--disable_query_log
--disable_result_log

--let $number_of_characters = `SELECT LENGTH(@special_characters)`
--let $charaacted_idx = 0
while ($charaacted_idx < $number_of_characters)
{
  eval SET @special_char = SUBSTRING(@special_characters, $charaacted_idx + 1, 1);
  --let $special_char = `SELECT @special_char`
  SET @special_string = CONCAT('abc', @special_char, 'def');
  # for both default and mecab parser
  if ($fts_parser == '')
  {
    SET @alnum_plus_character = @special_char REGEXP '^[a-zA-Z0-9_]';
    --let $expected_number_of_records = `SELECT IF(@alnum_plus_character, 1, 2)`
  }
  if ($fts_parser == ngram)
  {
    --let $expected_number_of_records = `SELECT LENGTH(@special_string) - 1`
  }
  if ($fts_parser == mecab)
  {
    SET @alnum_plus_character = @special_char REGEXP '^[a-zA-Z]';
    --let $expected_number_of_records = `SELECT IF(@alnum_plus_character, 1, 2)`
  }
  INSERT INTO t1 VALUES(@special_string);

  --let $assert_text = number of records for the special string containing '$special_char' is expected to be $expected_number_of_records
  --let $assert_cond = [SELECT COUNT(*) FROM INFORMATION_SCHEMA.INNODB_FT_INDEX_CACHE] = $expected_number_of_records
  --source include/assert.inc

  if ($fts_parser != ngram)
  {
    SET @expected_matches = JSON_ARRAY(
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'abc', 'result', IF(@alnum_plus_character, 0, 1)),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'def', 'result', IF(@alnum_plus_character, 0, 1)),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', CONCAT('abc', @special_char, 'def'), 'result', IF(@alnum_plus_character, 1, 0)),

      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'a', 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'b', 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'c', 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', @special_char, 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'd', 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'e', 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'f', 'result', 0),

      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'ab', 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'bc', 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', CONCAT('c', @special_char), 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', CONCAT(@special_char, 'd'), 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'de', 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'ef', 'result', 0),

      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'zbc', 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'dez', 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', CONCAT('bc', @special_char, 'de'), 'result', 0),

      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', CONCAT('abc', @special_char), 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', CONCAT(@special_char, 'def'), 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', CONCAT('zabc', @special_char, 'def'), 'result', 0),

      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'abc', 'result', IF(@alnum_plus_character, 0, 1)),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'def', 'result', IF(@alnum_plus_character, 0, 1)),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', CONCAT('abc', @special_char, 'def'), 'result', IF(@alnum_plus_character, 1, 0)),

      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'a', 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'b', 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'c', 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', @special_char, 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'd', 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'e', 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'f', 'result', 0),

      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'ab', 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'bc', 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', CONCAT('c', @special_char), 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', CONCAT(@special_char, 'd'), 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'de', 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'ef', 'result', 0),

      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'zbc', 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'dez', 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', CONCAT('bc', @special_char, 'de'), 'result', 0),

      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', CONCAT('abc', @special_char), 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', CONCAT(@special_char, 'def'), 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', CONCAT('zabc', @special_char, 'def'), 'result', 0)
    );
  }

  if ($fts_parser == ngram)
  {
    SET @expected_matches = JSON_ARRAY(
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'a', 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'b', 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'c', 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', @special_char, 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'd', 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'e', 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'f', 'result', 0),

      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'ab', 'result', 1),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'bc', 'result', 1),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', CONCAT('c', @special_char), 'result', 1),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', CONCAT(@special_char, 'd'), 'result', 1),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'de', 'result', 1),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'ef', 'result', 1),

      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'ac', 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', CONCAT('z', @special_char), 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', CONCAT(@special_char, 'z'), 'result', 0),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'df', 'result', 0),

      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'abc', 'result', 1),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', CONCAT('bc', @special_char), 'result', 1),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', CONCAT('c', @special_char, 'd'), 'result', 1),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', CONCAT(@special_char, 'de'), 'result', 1),
      JSON_OBJECT('mode', 'NATURAL LANGUAGE', 'pattern', 'def', 'result', 1),

      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'a', 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'b', 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'c', 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', @special_char, 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'd', 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'e', 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'f', 'result', 0),

      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'ab', 'result', 1),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'bc', 'result', 1),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', CONCAT('c', @special_char), 'result', 1),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', CONCAT(@special_char, 'd'), 'result', 1),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'de', 'result', 1),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'ef', 'result', 1),

      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'ac', 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', CONCAT('z', @special_char), 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', CONCAT(@special_char, 'z'), 'result', 0),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'df', 'result', 0),

      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'abc', 'result', 1),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', CONCAT('bc', @special_char), 'result', 1),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', CONCAT('c', @special_char, 'd'), 'result', 1),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', CONCAT(@special_char, 'de'), 'result', 1),
      JSON_OBJECT('mode', 'BOOLEAN', 'pattern', 'def', 'result', 1)
    );
  }

  --let $number_of_queries = `SELECT JSON_LENGTH(@expected_matches)`
  --let $query_idx = 0
  while ($query_idx < $number_of_queries)
  {
    eval SET @extraction_common_path = CONCAT(CHAR(36 USING utf8mb4), '[$query_idx]');
    eval SET @extraction_mode_path = CONCAT(@extraction_common_path, '.mode');
    eval SET @extraction_pattern_path = CONCAT(@extraction_common_path, '.pattern');
    eval SET @extraction_result_path = CONCAT(@extraction_common_path, '.result');

    --let $mode = `SELECT JSON_UNQUOTE(JSON_EXTRACT(@expected_matches, @extraction_mode_path))`
    SET @pattern = JSON_UNQUOTE(JSON_EXTRACT(@expected_matches, @extraction_pattern_path));
    --let $pattern = `SELECT @pattern`
    --let $expected_result = `SELECT JSON_EXTRACT(@expected_matches, @extraction_result_path)`

    --error 0, ER_PARSE_ERROR
    eval SELECT COUNT(*) INTO @query_result FROM t1 WHERE MATCH(c) AGAINST (@pattern IN $mode MODE);
    if ($mysql_errno)
    {
      --echo *** mode: "$mode", pattern: "$pattern", expected result: $expected_result, FTS syntax error
    }
    if (!$mysql_errno)
    {
      --let $result = `SELECT @query_result`
      if ($result != $expected_result)
      {
        --echo *** mode: "$mode", pattern: "$pattern", expected result: $expected_result, result: $result
      }
    }


    --inc $query_idx
  }

  DELETE FROM t1;
  OPTIMIZE TABLE t1;

  --inc $charaacted_idx
}

--enable_result_log
--enable_query_log

--echo
--echo *** dropping the table
DROP TABLE t1;

--echo
--echo *** restoring global system variables
SET GLOBAL innodb_ft_aux_table = @old_innodb_ft_aux_table;
SET GLOBAL innodb_optimize_fulltext_only = @old_innodb_optimize_fulltext_only;
