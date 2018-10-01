# compose full column definitions based on column name, column type and dict name for
# both original and modified columns
# define $original_dict_name_defined / $modified_dict_name_defined if corresponding
# definition uses a compression dictionary
--let $original_dict_name_defined = 0
--let $original_column_definition = $original_column_type
if ($original_dict_name)
{
  --let $original_column_definition = $original_column_definition COLUMN_FORMAT COMPRESSED
  if ($original_dict_name != '*')
  {
    --let $original_column_definition = $original_column_definition WITH COMPRESSION_DICTIONARY $original_dict_name
    --let $original_dict_name_defined = 1
  }
}

--let $modified_dict_name_defined = 0
--let $modified_column_definition = $modified_column_type
if ($modified_dict_name)
{
  --let $modified_column_definition = $modified_column_definition COLUMN_FORMAT COMPRESSED
  if ($modified_dict_name != '*')
  {
    --let $modified_column_definition = $modified_column_definition WITH COMPRESSION_DICTIONARY $modified_dict_name
    --let $modified_dict_name_defined = 1
  }
}

--echo *** "$original_column_name $original_column_definition" -> "$modified_column_name $modified_column_definition", ALGORITHM=$alter_table_algorithm

--disable_query_log

# create original table
eval CREATE TABLE $original_table_name(
  id INT,
  dummy INT,
  $original_column_name $original_column_definition,
  last INT
) ENGINE=InnoDB;

# insert two records in the table: one with a long value (> innodb_column_zip_threshold bytes)
# and another one with a short value (< innodb_column_zip_threshold bytes, for which compression
# won't be used)
--eval INSERT INTO $original_table_name VALUES(1, 0, REPEAT('foo', 128), 0)
--eval INSERT INTO $original_table_name VALUES(2, 0, 'bar', 0)

eval SELECT MD5($original_column_name) INTO @original_long_value_checksum FROM $original_table_name
  WHERE id = 1;
eval SELECT MD5($original_column_name) INTO @original_short_value_checksum FROM $original_table_name
  WHERE id = 2;

eval SELECT table_id INTO @original_table_id FROM information_schema.innodb_tables
  WHERE name=CONCAT(DATABASE(), '/', '$original_table_name');
eval SELECT pos INTO @original_column_pos FROM information_schema.innodb_columns
  WHERE table_id = @original_table_id AND name = '$original_column_name';

if ($original_dict_name_defined)
{
    eval SELECT COUNT(*) = 1 INTO @original_dictionary_reference_exists
    FROM information_schema.compression_dictionary_tables
    WHERE TABLE_SCHEMA='test' AND TABLE_NAME= '$original_table_name'
    AND COLUMN_NAME= '$original_column_name';

    SELECT @original_dictionary_reference_exists AS original_dictionary_reference_exists;
    let $original_dictionary_reference_exists = `SELECT @original_dictionary_reference_exists`;
  if ($original_dictionary_reference_exists)
  {
	  eval SELECT DICT_NAME = '$original_dict_name' AS original_dictionary_reference_matches FROM information_schema.compression_dictionary_tables WHERE TABLE_SCHEMA='test' AND TABLE_NAME= '$original_table_name' AND COLUMN_NAME= '$original_column_name';
  }
}

if (!$original_dict_name_defined)
{
    eval SELECT COUNT(*) = 0 AS original_dictionary_reference_does_not_exist
    FROM information_schema.compression_dictionary_tables
    WHERE TABLE_SCHEMA='test' AND TABLE_NAME= '$original_table_name'
    AND COLUMN_NAME= '$original_column_name';
}
