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
  SELECT COUNT(*) = 0 AS original_dictionary_reference_cleared
    FROM information_schema.xtradb_zip_dict_cols
    WHERE table_id = @original_table_id AND column_pos = @original_column_pos;
}

if (!$storage_engine_changed)
{
  eval SELECT table_id INTO @modified_table_id FROM information_schema.innodb_sys_tables
    WHERE name=CONCAT(DATABASE(), '/', '$modified_table_name');
  eval SELECT pos INTO @modified_column_pos FROM information_schema.innodb_sys_columns
    WHERE table_id = @modified_table_id AND name = '$modified_column_name';

  if ($modified_dict_name_defined)
  {
    eval SELECT id INTO @modified_dict_id FROM information_schema.xtradb_zip_dict
      WHERE name = '$modified_dict_name';
    let $modified_dictionary_reference_exists = `SELECT COUNT(*) = 1
      FROM information_schema.xtradb_zip_dict_cols
      WHERE table_id = @modified_table_id AND column_pos = @modified_column_pos`;
    --eval SELECT $modified_dictionary_reference_exists AS modified_dictionary_reference_exists
    if ($modified_dictionary_reference_exists)
    {
      SELECT dict_id = @modified_dict_id AS modified_dictionary_reference_matches
        FROM information_schema.xtradb_zip_dict_cols
        WHERE table_id = @modified_table_id AND column_pos = @modified_column_pos;
    }
  }
  if (!$modified_dict_name_defined)
  {
    SELECT COUNT(*) = 0 AS modified_dictionary_reference_does_not_exist
      FROM information_schema.xtradb_zip_dict_cols
      WHERE table_id = @modified_table_id AND column_pos = @modified_column_pos;
  }
}

--eval DROP TABLE $modified_table_name

SELECT COUNT(*) = 0 AS original_dictionary_references_cleaned
  FROM information_schema.xtradb_zip_dict_cols
  WHERE table_id = @original_table_id;

if (!$storage_engine_changed)
{
  SELECT COUNT(*) = 0 AS modified_dictionary_references_cleaned
    FROM information_schema.xtradb_zip_dict_cols
    WHERE table_id = @modified_table_id;
}

--enable_query_log

--echo
