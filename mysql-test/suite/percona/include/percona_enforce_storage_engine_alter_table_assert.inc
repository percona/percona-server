--disable_query_log
--echo Checking '$enforce_table': Engine = '$enforce_engine'
eval SELECT Engine = '$enforce_engine' AS engine_match FROM INFORMATION_SCHEMA.TABLES
  WHERE table_schema = DATABASE() AND table_name ='$enforce_table';

--echo Checking '$enforce_table': Column = '$enforce_column'
eval SELECT column_type = '$enforce_column' AS column_match
  FROM INFORMATION_SCHEMA.COLUMNS
  WHERE table_schema = DATABASE() AND table_name = '$enforce_table' AND column_name='a';

--enable_query_log

