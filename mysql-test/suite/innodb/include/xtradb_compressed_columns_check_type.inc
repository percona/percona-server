# the following variables must be defined:
# - $row_format - DYNAMIC | COMPRESSED | REDUNDANT | COMPACT
# - $compressed_attribute - COLUMN_FORMAT COMPRESSED | COLUMN_FORMAT COMPRESSED WITH COMPRESSION_DICTIONARY <dict> | empty string
# - define either $full_field_type and $type_length_limit or $base_field_type and $field_length
#   for instance,
#   ($full_field_type = BLOB and $type_length_limit = 65535) means BLOB (with lax length 65535)
#   or
#   ($base_field_type = VARCHAR and $field_length = 128) means VARCHAR(128)

if(!$full_field_type)
{
  --let $local_full_field_type= $base_field_type($field_length) $compressed_attribute

  # NATIONAL VARCHAR(N) where N >= 32766 is silently converted
  # to TEXT - adjusting $local_type_length_limit correspondingly
  if($base_field_type != 'NATIONAL VARCHAR')
  {
    --let $local_type_length_limit= $field_length
  }
  if($base_field_type == 'NATIONAL VARCHAR')
  {
    if($field_length >= 32766)
    {
      --let $local_type_length_limit= 65535
    }
    if($field_length < 32766)
    {
      --let $local_type_length_limit= $field_length
    }
  }
}
if($full_field_type)
{
  --let $local_full_field_type= $full_field_type $compressed_attribute
  --let $local_type_length_limit= $type_length_limit
}

--disable_query_log
eval CREATE TABLE t1(
  a $local_full_field_type
) ROW_FORMAT = $row_format ENGINE=InnoDB;

if(!$full_field_type)
{
  --let $current_value_length= `SELECT IF($field_length > 3, $field_length - 3, 1)`
  --let $max_value_length= `SELECT $field_length + 3`
}
if($full_field_type)
{
  --let $current_value_length= `SELECT IF($field_length > 0, $field_length, 1)`
  --let $max_value_length= $field_length
}

INSERT INTO t1 VALUES(NULL);
--let $include_silent= 1
--let $assert_text= null value must match ($local_full_field_type, ROW_RORMAT = $row_format, length = $field_length)
--let $assert_cond= [SELECT COUNT(*) FROM t1 WHERE a IS NULL] = 1
--source include/assert.inc
--let $include_silent= 0
DELETE FROM t1;

INSERT INTO t1 VALUES('');
--let $include_silent= 1
--let $assert_text= empty value must match ($local_full_field_type, ROW_RORMAT = $row_format, length = $field_length)
--let $assert_cond= [SELECT COUNT(*) FROM t1 WHERE a = ""] = 1
--source include/assert.inc
--let $include_silent= 0
DELETE FROM t1;

while($current_value_length <= $max_value_length)
{
  eval SET @inserted_value = REPEAT('z', $current_value_length);
  --disable_warnings
  INSERT INTO t1 VALUES(@inserted_value);
  --enable_warnings
  --let $include_silent= 1
  --let $assert_text= value of $current_value_length byte(s) must match ($local_full_field_type, ROW_RORMAT = $row_format, length = $field_length, current = $current_value_length)
  --let $assert_cond= [SELECT COUNT(*) FROM t1 WHERE a = LEFT(@inserted_value, $local_type_length_limit)] = 1
  --source include/assert.inc
  --let $include_silent= 0
  DELETE FROM t1;
  --inc $current_value_length
}

DROP TABLE t1;
--enable_query_log
