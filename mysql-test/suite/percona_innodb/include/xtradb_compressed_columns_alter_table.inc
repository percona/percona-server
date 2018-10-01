# include file for xtradb_compressed_columns_alter_table.test

# define the following variables before inclusion:
#   $original_table_name   - table name for testing alterations, e.g. 't1'
#   $original_column_name  - original column name inside $original_table_name, e.g. 'a'
#   $original_column_type  - original type of the $original_column_name column, e.g. 'BLOB'
#   $modified_table_name   - table name after the alteration, can also be $original_table_name
#   $modified_column_name  - column name to rename $original_column_name to, e.g. 'b', can also be $original_column_name
#   $modified_column_type  - column type to change $original_column_type to, e.g. 'TEXT', can also be $original_column_type
#   $alter_table_algorithm - algoritm to use for 'ALTER TABLE' operation, can be 'INPLACE' / 'COPY' / 'DEFAULT'
#   $original_dict_name    - compression dictionary associated with $original_column_type
#   $modified_dict_name    - compression dictionary associated with $modified_column_type
# $original_dict_name and $modified_dict_name can take the following values:
#   '' (empty string)          - meaning full column type will be just '$column_type'
#   '*'                        - meaning full column type will be just '$column_type COLUMN_FORMAT COMPRESSED'
#   '<dict>'(anything but '*') - meaning full column type will be just '$column_type COLUMN_FORMAT COMPRESSED WITH COMPRESSION_DICTIONARY <dict>'
#   'unknown'(literally)       - has the same meaning as '<dict>' but an error is expected

--source ../include/xtradb_compressed_columns_alter_table_pre.inc

--let $expected_error =
if ($alter_table_algorithm == 'INPLACE')
{
  if ($original_column_definition != $modified_column_definition)
  {
    --let $expected_error = ER_ALTER_OPERATION_NOT_SUPPORTED_REASON
  }
}

if ($modified_dict_name == 'unknown')
{
  --let $expected_error = ER_COMPRESSION_DICTIONARY_DOES_NOT_EXIST
}

let $alter_table_statement = ALTER TABLE $original_table_name
  CHANGE COLUMN $original_column_name $modified_column_name $modified_column_definition,
  ALGORITHM = $alter_table_algorithm;
if ($expected_error)
{
  --echo error is expected
  --error $expected_error
  --eval $alter_table_statement
}
if (!$expected_error)
{
  --echo successful execution is expected
  --eval $alter_table_statement
}


# here, if error occurs, we make xtradb_compressed_columns_alter_table_post.inc
# think that table hasn't been changed by reassigning control variables
if ($expected_error)
{
  --let $saved_modified_column_name       = $modified_column_name
  --let $saved_modified_column_type       = $modified_column_type
  --let $saved_modified_dict_name         = $modified_dict_name
  --let $saved_modified_dict_name_defined = $modified_dict_name_defined
  --let $saved_modified_column_definition = $modified_column_definition

  --let $modified_column_name       = $original_column_name
  --let $modified_column_type       = $original_column_type
  --let $modified_dict_name         = $original_dict_name
  --let $modified_dict_name_defined = $original_dict_name_defined
  --let $modified_column_definition = $original_column_definition
}

--source ../include/xtradb_compressed_columns_alter_table_post.inc

if ($expected_error)
{
  --let $modified_column_name       = $saved_modified_column_name
  --let $modified_column_type       = $saved_modified_column_type
  --let $modified_dict_name         = $saved_modified_dict_name
  --let $modified_dict_name_defined = $saved_modified_dict_name_defined
  --let $modified_column_definition = $saved_modified_column_definition
}
