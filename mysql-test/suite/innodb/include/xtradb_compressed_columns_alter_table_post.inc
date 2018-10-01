--echo *** after table alteration

eval SELECT MD5($modified_column_name) = @original_long_value_checksum AS long_value_match
  FROM $modified_table_name
  WHERE id = 1;
eval SELECT MD5($modified_column_name) = @original_short_value_checksum AS short_value_match
  FROM $modified_table_name
  WHERE id = 2;

--let $need_check_for_original_dictionary_reference_cleared = 0
if ($original_dict_name_defined)
{
  --let $need_check_for_original_dictionary_reference_cleared = 1
  if ($modified_dict_name_defined)
  {
    if ($original_dict_name == $modified_dict_name)
    {
      --let $need_check_for_original_dictionary_reference_cleared = 0
    }
  }
}
if ($need_check_for_original_dictionary_reference_cleared)
{
    eval SELECT COUNT(*) = 0 AS original_dictionary_reference_cleared
    FROM information_schema.compression_dictionary_tables
    WHERE TABLE_SCHEMA='test' AND TABLE_NAME= '$original_table_name'
    AND COLUMN_NAME= '$original_column_name';

}

if (!$storage_engine_changed)
{
  if ($modified_dict_name_defined)
  {
      eval SELECT COUNT(*) = 1 INTO @modified_dictionary_reference_exists
      FROM information_schema.compression_dictionary_tables
      WHERE TABLE_SCHEMA='test' AND TABLE_NAME='$modified_table_name'
      AND COLUMN_NAME = '$modified_column_name';

      SELECT @modified_dictionary_reference_exists AS modified_dictionary_reference_exists;
      let $modified_dictionary_reference_exists = `SELECT @modified_dictionary_reference_exists`;

    if ($modified_dictionary_reference_exists)
    {
      eval SELECT DICT_NAME = '$modified_dict_name' AS modified_dictionary_reference_matches
        FROM information_schema.compression_dictionary_tables
      WHERE TABLE_SCHEMA='test' AND TABLE_NAME='$modified_table_name'
      AND COLUMN_NAME = '$modified_column_name';
    }
  }

  if (!$modified_dict_name_defined)
  {
    eval SELECT COUNT(*) = 0 AS modified_dictionary_reference_does_not_exist
      FROM information_schema.compression_dictionary_tables
      WHERE TABLE_SCHEMA='test' AND TABLE_NAME='$modified_table_name'
      AND COLUMN_NAME = '$modified_column_name';
  }
}

--eval DROP TABLE $modified_table_name

eval SELECT COUNT(*) = 0 AS original_dictionary_references_cleaned
  FROM information_schema.compression_dictionary_tables
  WHERE TABLE_SCHEMA='test' AND TABLE_NAME='$original_table_name';

if (!$storage_engine_changed)
{
  eval SELECT COUNT(*) = 0 AS modified_dictionary_references_cleaned
    FROM information_schema.compression_dictionary_tables
    WHERE TABLE_SCHEMA='test' AND TABLE_NAME='$modified_table_name';
}

--enable_query_log

--echo
