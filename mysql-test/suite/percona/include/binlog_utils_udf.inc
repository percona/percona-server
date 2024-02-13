#
# Create functions from binlog_utils_udf
#

--disable_query_log
SET @path_separator = '/';
if(`SELECT CONVERT(@@version_compile_os USING latin1) IN ("Win32", "Win64", "Windows") = 1`)
{
  SET @path_separator = '\\';
}
--let $binlog_base_dir = `SELECT LEFT(@@global.log_bin_basename, CHAR_LENGTH(@@global.log_bin_basename) - CHAR_LENGTH(SUBSTRING_INDEX(@@global.log_bin_basename, @path_separator, -1)))`
--enable_query_log

--let $non_existing_binlog_name = non_existing.000001
--let $non_existing_binlog_name_regex = /'.*($non_existing_binlog_name)'/'\1'/

--echo
--echo *** creating a table that will be used to generate binlog events
CREATE TABLE t1(
  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  PRIMARY KEY(id)
) ENGINE=InnoDB;

--echo
--echo *** creating a temporary table that will store recorded binlog names and GTIDs
CREATE TEMPORARY TABLE captured_gtid(
  stage BIGINT UNSIGNED NOT NULL,
  substage BIGINT UNSIGNED NOT NULL,
  binlog_name VARCHAR(64) NOT NULL,
  gtid_value VARCHAR(64) NOT NULL,
  PRIMARY KEY(stage, substage)
) ENGINE=InnoDB;

# Flush binary logs so that previous "CREATE TABLE" statement won't affect GTID
# recording logic.
FLUSH BINARY LOGS;

--echo
--echo *** generating several binlogs
--disable_query_log
--disable_result_log
--let $number_of_stages = 16
--let $number_of_substages = 16

--let $stage = 1
while($stage <= $number_of_stages)
{
  --let $binlog_file_name = query_get_value(SHOW MASTER STATUS, File, 1)

  --let $substage = 1
  while($substage <= $number_of_substages)
  {
    SET @saved_gtid_executed = @@global.gtid_executed;
    START TRANSACTION;
    --let $i = 1
    while ($i <= $substage)
    {
      INSERT INTO t1 VALUES(DEFAULT);
      --inc $i
    }
    COMMIT;
    eval INSERT INTO captured_gtid VALUES($stage, $substage, '$binlog_file_name', GTID_SUBTRACT(@@global.gtid_executed, @saved_gtid_executed));
    # sleep here is needed to ensure binlog records will have different timestamps
    if ($substage == 1)
    {
      --sleep 1
    }
    --inc $substage
    # sleep here is needed to ensure binlog records will have different timestamps
    if ($substage == $number_of_substages)
    {
      --sleep 1
    }
  }

  FLUSH BINARY LOGS;

  --inc $stage
}
--enable_result_log
--enable_query_log

--echo
--echo *** checking if UDF works without loading the plugin
--replace_result $binlog_file_name <binlog_file_name>
--error ER_SP_DOES_NOT_EXIST
eval SELECT get_last_gtid_from_binlog('$binlog_file_name');

INSTALL COMPONENT 'file://component_binlog_utils_udf';

--echo
--echo *** checking 'get_last_gtid_from_binlog()' function

--error ER_CANT_INITIALIZE_UDF
SELECT get_last_gtid_from_binlog();

--error ER_CANT_INITIALIZE_UDF
SELECT get_last_gtid_from_binlog('blah', 'blah');

--error ER_UDF_ERROR
SELECT get_last_gtid_from_binlog(NULL);

# EE_FILENOTFOUND 29
--replace_regex $non_existing_binlog_name_regex
--error 29
eval SELECT get_last_gtid_from_binlog('$non_existing_binlog_name');

--let $stage = 1
while($stage <= $number_of_stages)
{
  --let $current_binlog_name = `SELECT binlog_name FROM captured_gtid WHERE stage = $stage AND substage = $number_of_substages`
  --let $current_gtid_value = `SELECT gtid_value FROM captured_gtid WHERE stage = $stage AND substage = $number_of_substages`

  --let $assert_text = 'GTID extracted via get_last_gtid_from_binlog() for stage $stage should match the recorded value'
  --let $assert_cond = get_last_gtid_from_binlog("$current_binlog_name") = "$current_gtid_value"
  --source include/assert.inc

  --inc $stage
}


--echo
--echo *** checking 'get_binlog_by_gtid()' function

--error ER_CANT_INITIALIZE_UDF
SELECT get_binlog_by_gtid();

--error ER_CANT_INITIALIZE_UDF
SELECT get_binlog_by_gtid('blah', 'blah');

--error ER_UDF_ERROR
SELECT get_binlog_by_gtid(NULL);

--let $stage = 1
while($stage <= $number_of_stages)
{
  --let $current_binlog_name = `SELECT binlog_name FROM captured_gtid WHERE stage = $stage AND substage = $number_of_substages`
  --let $substage = 1
  while($substage <= $number_of_substages)
  {
    --let $current_gtid_value = `SELECT gtid_value FROM captured_gtid WHERE stage = $stage AND substage = $substage`

    --let $assert_text = 'binlog name extracted via get_binlog_by_gtid() for stage $stage, substage $substage should match the recorded value'
    --let $assert_cond = get_binlog_by_gtid("$current_gtid_value") = "$current_binlog_name"
    --source include/assert.inc

    --inc $substage
  }
  --inc $stage
}


--echo
--echo *** checking 'get_gtid_set_by_binlog()' function

--error ER_CANT_INITIALIZE_UDF
SELECT get_gtid_set_by_binlog();

--error ER_CANT_INITIALIZE_UDF
SELECT get_gtid_set_by_binlog('blah', 'blah');

--error ER_UDF_ERROR
SELECT get_gtid_set_by_binlog(NULL);

--error ER_UDF_ERROR
eval SELECT get_gtid_set_by_binlog('$non_existing_binlog_name');

--let $stage = 1
while($stage <= $number_of_stages)
{
  --let $current_binlog_name = `SELECT binlog_name FROM captured_gtid WHERE stage = $stage AND substage = $number_of_substages`
  # use GTID_SUBTRACT() here to normalize concatenated GTIDs
  --let $current_gtid_set = `SELECT GTID_SUBTRACT(GROUP_CONCAT(gtid_value ORDER BY substage SEPARATOR ','), '') FROM captured_gtid WHERE stage = $stage`

  --let $assert_text = 'GTID set extracted via get_gtid_set_by_binlog() for stage $stage should match the recorded value'
  --let $assert_cond = get_gtid_set_by_binlog("$current_binlog_name") = "$current_gtid_set"
  --source include/assert.inc

  --inc $stage
}


--echo
--echo *** checking 'get_binlog_by_gtid_set()' function

--error ER_CANT_INITIALIZE_UDF
SELECT get_binlog_by_gtid_set();

--error ER_CANT_INITIALIZE_UDF
SELECT get_binlog_by_gtid_set('blah', 'blah');

--error ER_UDF_ERROR
SELECT get_binlog_by_gtid_set(NULL);

SET SESSION group_concat_max_len = 65536;

--let $number_of_iterations = `SELECT $number_of_stages * 3`
--let $outer_index = 0
while($outer_index < $number_of_iterations)
{
  --let $outer_stage = `SELECT ($outer_index DIV 3) + 1`
  --let $outer_substage = `SELECT ELT(($outer_index MOD 3) + 1, 1, $number_of_substages DIV 2, $number_of_substages)`
  --let $lower_bound = `SELECT ($outer_stage - 1) * $number_of_substages + $outer_substage - 1`
  --let $current_binlog_name = `SELECT binlog_name FROM captured_gtid WHERE stage = $outer_stage AND substage = $number_of_substages`

  --let $inner_index = $outer_index
  while($inner_index < $number_of_iterations)
  {
    --let $inner_stage = `SELECT ($inner_index DIV 3) + 1`
    --let $inner_substage = `SELECT ELT(($inner_index MOD 3) + 1, 1, $number_of_substages DIV 2, $number_of_substages)`
    --let $upper_bound = `SELECT ($inner_stage - 1) * $number_of_substages + $inner_substage - 1`

    --let $current_gtid_set = `SELECT GTID_SUBTRACT(GROUP_CONCAT(gtid_value SEPARATOR ','), '') AS result FROM captured_gtid WHERE ((stage - 1) * $number_of_substages + substage - 1) BETWEEN $lower_bound AND $upper_bound`
    --let $assert_text = 'binlog name extracted via get_binlog_by_gtid_set() for ($outer_stage, $outer_substage)..($inner_stage, $inner_substage) should match the recorded value'
    --let $assert_cond = get_binlog_by_gtid_set("$current_gtid_set") = "$current_binlog_name"
    --source include/assert.inc

    --inc $inner_index
  }

  --inc $outer_index
}


# creating a temporary table which will be used for LOAD DATA LOCAL INFILE
CREATE TEMPORARY TABLE loaded_ts(
  val VARCHAR(255) NOT NULL
) ENGINE=InnoDB;

--echo
--echo *** checking 'get_first_record_timestamp_by_binlog()' function

--error ER_CANT_INITIALIZE_UDF
SELECT get_first_record_timestamp_by_binlog();

--error ER_CANT_INITIALIZE_UDF
SELECT get_first_record_timestamp_by_binlog('blah', 'blah');

--error ER_UDF_ERROR
SELECT get_first_record_timestamp_by_binlog(NULL);

# EE_FILENOTFOUND 29
--replace_regex $non_existing_binlog_name_regex
--error 29
eval SELECT get_first_record_timestamp_by_binlog('$non_existing_binlog_name');

--let $filtered_binlog_file = $MYSQLTEST_VARDIR/filtered_binlog_file

--let $stage = 1
while($stage <= $number_of_stages)
{
  --let $current_binlog_name = `SELECT binlog_name FROM captured_gtid WHERE stage = $stage AND substage = $number_of_substages`
  --let $current_binlog_name_for_utility = $binlog_base_dir$current_binlog_name

  # Extracting lines with timestamps (e.g. "#201116 20:54:02") from the mysqlbinlog output
  --exec $MYSQL_BINLOG $current_binlog_name_for_utility | grep -Eo '^#[[:digit:]]+[[:blank:]]+[[:digit:]]+:[[:digit:]]+:[[:digit:]]+' | head -n 1 > $filtered_binlog_file

  DELETE FROM loaded_ts;
  --replace_result $MYSQLTEST_VARDIR MYSQLTEST_VARDIR
  eval LOAD DATA LOCAL INFILE '$filtered_binlog_file' INTO TABLE loaded_ts;
  --remove_file $filtered_binlog_file

  # Converting "#201116 20:54:02" extracted from mysqlbinlog output into
  # "20-11-16 20:54:02" and then into Unix timestamps
  --let $expected_ts = `SELECT FLOOR(UNIX_TIMESTAMP(INSERT(INSERT(SUBSTRING(val, 2), 5, 0, '-'), 3, 0, '-'))) FROM loaded_ts`

  --let $assert_text = 'first record timestamp extracted via get_first_record_timestamp_by_binlog() for stage $stage should match the value extracted via mysqlbinlog'
  --let $assert_cond = get_first_record_timestamp_by_binlog("$current_binlog_name") DIV 1000000 = $expected_ts
  --source include/assert.inc

  --inc $stage
}


--echo
--echo *** checking 'get_last_record_timestamp_by_binlog()' function

--error ER_CANT_INITIALIZE_UDF
SELECT get_last_record_timestamp_by_binlog();

--error ER_CANT_INITIALIZE_UDF
SELECT get_last_record_timestamp_by_binlog('blah', 'blah');

--error ER_UDF_ERROR
SELECT get_last_record_timestamp_by_binlog(NULL);

# EE_FILENOTFOUND 29
--replace_regex $non_existing_binlog_name_regex
--error 29
eval SELECT get_last_record_timestamp_by_binlog('$non_existing_binlog_name');

--let $filtered_binlog_file = $MYSQLTEST_VARDIR/filtered_binlog_file

--let $stage = 1
while($stage <= $number_of_stages)
{
  --let $current_binlog_name = `SELECT binlog_name FROM captured_gtid WHERE stage = $stage AND substage = $number_of_substages`
  --let $current_binlog_name_for_utility = $binlog_base_dir$current_binlog_name

  # Extracting lines with timestamps (e.g. "#201116 20:54:02") from the mysqlbinlog output
  --exec $MYSQL_BINLOG $current_binlog_name_for_utility | grep -Eo '^#[[:digit:]]+[[:blank:]]+[[:digit:]]+:[[:digit:]]+:[[:digit:]]+' | tail -n 1 > $filtered_binlog_file

  DELETE FROM loaded_ts;
  --replace_result $MYSQLTEST_VARDIR MYSQLTEST_VARDIR
  eval LOAD DATA LOCAL INFILE '$filtered_binlog_file' INTO TABLE loaded_ts;
  --remove_file $filtered_binlog_file

  # Converting "#201116 20:54:02" extracted from mysqlbinlog output into
  # "20-11-16 20:54:02" and then into Unix timestamps
  --let $expected_ts = `SELECT FLOOR(UNIX_TIMESTAMP(INSERT(INSERT(SUBSTRING(val, 2), 5, 0, '-'), 3, 0, '-'))) FROM loaded_ts`

  --let $assert_text = 'last record timestamp extracted via get_last_record_timestamp_by_binlog() for stage $stage should match the value extracted via mysqlbinlog'
  --let $assert_cond = get_last_record_timestamp_by_binlog("$current_binlog_name") DIV 1000000 = $expected_ts
  --source include/assert.inc

  --inc $stage
}

# temporary table cleanup
DROP TABLE loaded_ts;

UNINSTALL COMPONENT 'file://component_binlog_utils_udf';

DROP TABLE captured_gtid;
DROP TABLE t1;
