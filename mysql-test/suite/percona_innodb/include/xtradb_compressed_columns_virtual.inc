# $table_name - table name
# $source_column_name - source column name
# $source_column_type - source column type (e.g. BLOB | TEXT | VARCHAR(N), etc.)
# $source_dictionary_name - '' (no compression) | '*' (compressed wo dictionary) | '<dictionary>' (compresed with <dictionary>)
# $generated_column_name - generated columns name
# $generated_column_type - the type of the generated columns (e.g. BLOB | TEXT | VARCHAR(N), etc.)
# $generated_expression_length - generated column expression will be 'LEFT($source_column_name, $generated_expression_length)'
# $generated_storage_type - VIRTUAL | STORED
# $generated_dictionary_name - '' (no compression) | '*' (compressed wo dictionary) | '<dictionary>' (compresed with <dictionary>)
# $indexed - non-empty means that generated columns will be indexed

--let $source_column_definition = $source_column_name $source_column_type
if($source_dictionary_name != '')
{
  --let $source_column_definition = $source_column_definition COLUMN_FORMAT COMPRESSED
  if($source_dictionary_name != '*')
  {
    --let $source_column_definition = $source_column_definition WITH COMPRESSION_DICTIONARY $source_dictionary_name
  }
}
--let $generated_column_definition = $generated_column_name $generated_column_type GENERATED ALWAYS AS (LEFT($source_column_name, $generated_expression_length)) $generated_storage_type
if($generated_dictionary_name != '')
{
  --let $generated_column_definition = $generated_column_definition COLUMN_FORMAT COMPRESSED
  if($generated_dictionary_name != '*')
  {
    --let $generated_column_definition = $generated_column_definition WITH COMPRESSION_DICTIONARY $generated_dictionary_name
  }
}

--let $index_definition =
if($indexed)
{
  --let $index_definition = , INDEX($generated_column_name($generated_expression_length))
}

--disable_query_log
eval CREATE TABLE $table_name (
  id INT,
  $source_column_definition,
  $generated_column_definition
  $index_definition
) ENGINE=InnoDB;

SET @short_value = REPEAT('a', 4);
eval INSERT INTO $table_name VALUES(2, @short_value, DEFAULT);
if($generated_expression_length > 4)
{
  SET @long_value = REPEAT('z', 8192);
  eval INSERT INTO $table_name VALUES(1, @long_value, DEFAULT);
}
--enable_query_log

--let $include_silent = 1
--let $assert_text= Short value must not be corrupted (source: "$source_column_definition", generated: "$generated_column_definition", indexed: "indexed")
--let $assert_cond= [SELECT COUNT(*) FROM $table_name WHERE $generated_column_name = LEFT(@short_value, $generated_expression_length)] = 1
--source include/assert.inc
--let $include_silent = 0

if($generated_expression_length > 4)
{
  --let $include_silent = 1
  --let $assert_text= Long value must not be corrupted (source: "$source_column_definition", generated: "$generated_column_definition", indexed: "indexed")
  --let $assert_cond= [SELECT COUNT(*) FROM $table_name WHERE $generated_column_name = LEFT(@long_value, $generated_expression_length)] = 1
  --source include/assert.inc
  --let $include_silent = 0
}

--disable_query_log
eval DROP TABLE $table_name;
--enable_query_log
