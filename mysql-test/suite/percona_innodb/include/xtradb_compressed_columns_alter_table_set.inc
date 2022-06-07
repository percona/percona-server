--echo # from 'BLOB' to 'xxx'
--let $original_dict_name =
--let $modified_dict_name =
--source ../include/xtradb_compressed_columns_alter_table.inc

--let $modified_dict_name = *
--source ../include/xtradb_compressed_columns_alter_table.inc

--let $modified_dict_name = $dict1_name
--source ../include/xtradb_compressed_columns_alter_table.inc

--let $modified_dict_name = $unknown_dict_name
--source ../include/xtradb_compressed_columns_alter_table.inc


--echo # from 'BLOB COLUMN_FORMAT COMPRESSED' to 'xxx'
--let $original_dict_name = *
--let $modified_dict_name =
--source ../include/xtradb_compressed_columns_alter_table.inc

--let $modified_dict_name = *
--source ../include/xtradb_compressed_columns_alter_table.inc

--let $modified_dict_name = $dict1_name
--source ../include/xtradb_compressed_columns_alter_table.inc

--let $modified_dict_name = $unknown_dict_name
--source ../include/xtradb_compressed_columns_alter_table.inc


--echo # from 'BLOB COLUMN_FORMAT COMPRESSED WITH COMPRESSION_DICTIONARY <dict>' to 'xxx'
--let $original_dict_name = $dict1_name
--let $modified_dict_name =
--source ../include/xtradb_compressed_columns_alter_table.inc

--let $modified_dict_name = *
--source ../include/xtradb_compressed_columns_alter_table.inc

--let $modified_dict_name = $dict1_name
--source ../include/xtradb_compressed_columns_alter_table.inc

--let $modified_dict_name = $unknown_dict_name
--source ../include/xtradb_compressed_columns_alter_table.inc

# extra check for changing compression dictionary from '<dict1>' to '<dict2>'
--let $modified_dict_name = $dict2_name
--source ../include/xtradb_compressed_columns_alter_table.inc


# save the value of external variables we need to modify
--let $local_modified_table_name  = $modified_table_name
--let $local_modified_column_name = $modified_column_name


--echo # check for adding a column with associated compression dictionary at the first position
--let $modified_column_name = b
--let $original_dict_name =
--let $modified_dict_name = $dict1_name
--source ../include/xtradb_compressed_columns_alter_table_pre.inc
eval ALTER TABLE $original_table_name ADD COLUMN $modified_column_name $modified_column_definition FIRST,
  ALGORITHM = $alter_table_algorithm;
eval UPDATE $original_table_name SET $modified_column_name = $original_column_name;
--source ../include/xtradb_compressed_columns_alter_table_post.inc

--echo # check for adding a column with associated compression dictionary between existing columns
--source ../include/xtradb_compressed_columns_alter_table_pre.inc
eval ALTER TABLE $original_table_name ADD COLUMN $modified_column_name $modified_column_definition AFTER id,
  ALGORITHM = $alter_table_algorithm;
eval UPDATE $original_table_name SET $modified_column_name = $original_column_name;
--source ../include/xtradb_compressed_columns_alter_table_post.inc

--echo # check for adding a column with associated compression dictionary at the last position
--source ../include/xtradb_compressed_columns_alter_table_pre.inc
eval ALTER TABLE $original_table_name ADD COLUMN $modified_column_name $modified_column_definition AFTER last,
  ALGORITHM = $alter_table_algorithm;
eval UPDATE $original_table_name SET $modified_column_name = $original_column_name;
--source ../include/xtradb_compressed_columns_alter_table_post.inc


--echo # check for removing a column with associated compression dictionary
--let $original_dict_name = $dict1_name
--let $modified_dict_name =
--source ../include/xtradb_compressed_columns_alter_table_pre.inc
eval ALTER TABLE $original_table_name ADD COLUMN $modified_column_name $modified_column_definition AFTER last,
  ALGORITHM = $alter_table_algorithm;
eval UPDATE $original_table_name SET $modified_column_name = $original_column_name;
eval ALTER TABLE $original_table_name DROP COLUMN $original_column_name,
  ALGORITHM = $alter_table_algorithm;
--source ../include/xtradb_compressed_columns_alter_table_post.inc


--echo # add a new column to an existing table so that a column with associated compression dictionary would get shifted
--let $modified_column_name = $local_modified_column_name
--let $original_dict_name = $dict1_name
--let $modified_dict_name = $dict1_name
--source ../include/xtradb_compressed_columns_alter_table_pre.inc
eval ALTER TABLE $original_table_name ADD COLUMN custom INT AFTER id,
  ALGORITHM = $alter_table_algorithm;
--source ../include/xtradb_compressed_columns_alter_table_post.inc


--echo # remove a column from an existing table so that a column with associated compression dictionary would get shifted
--source ../include/xtradb_compressed_columns_alter_table_pre.inc
eval ALTER TABLE $original_table_name DROP COLUMN dummy,
  ALGORITHM = $alter_table_algorithm;
--source ../include/xtradb_compressed_columns_alter_table_post.inc


--echo # explicitly move a column with associated compression dictionary to a new position (forward)
--source ../include/xtradb_compressed_columns_alter_table_pre.inc
eval ALTER TABLE $original_table_name CHANGE COLUMN $original_column_name $modified_column_name $modified_column_definition AFTER last,
  ALGORITHM = $alter_table_algorithm;
--source ../include/xtradb_compressed_columns_alter_table_post.inc

--echo # explicitly move a column with associated compression dictionary to a new position (backward)
--source ../include/xtradb_compressed_columns_alter_table_pre.inc
eval ALTER TABLE $original_table_name CHANGE COLUMN $original_column_name $modified_column_name $modified_column_definition AFTER id,
  ALGORITHM = $alter_table_algorithm;
--source ../include/xtradb_compressed_columns_alter_table_post.inc


--echo # rename a table which has a column with associated compression dictionary
--let $modified_table_name = t2
--source ../include/xtradb_compressed_columns_alter_table_pre.inc
eval ALTER TABLE $original_table_name RENAME TO $modified_table_name,
  ALGORITHM=$alter_table_algorithm;
--source ../include/xtradb_compressed_columns_alter_table_post.inc

# restore the value of the $modified_column_name variable
--let $modified_table_name  = $local_modified_table_name
--let $modified_column_name = $local_modified_column_name


if ($alter_table_algorithm != 'INPLACE')
{
  --echo # change storage engine for a table without compressed columns
  --let $original_dict_name =
  --let $modified_dict_name =
  --let storage_engine_changed = 1
  --source ../include/xtradb_compressed_columns_alter_table_pre.inc
  eval ALTER TABLE $original_table_name ENGINE=MyISAM,
    ALGORITHM=$alter_table_algorithm;
  --source ../include/xtradb_compressed_columns_alter_table_post.inc
  --let storage_engine_changed =

  --echo # change storage engine for a table which has a compressed column
  --let $original_dict_name = *
  --let $modified_dict_name = $original_dict_name
  --source ../include/xtradb_compressed_columns_alter_table_pre.inc
  --echo error is expected
  --error ER_ILLEGAL_HA_CREATE_OPTION
  eval ALTER TABLE $original_table_name ENGINE=MyISAM,
    ALGORITHM=$alter_table_algorithm;
  --source ../include/xtradb_compressed_columns_alter_table_post.inc

  --echo # change storage engine for a table which has a column with associated compression dictionary
  --let $original_dict_name = $dict1_name
  --let $modified_dict_name = $original_dict_name
  --source ../include/xtradb_compressed_columns_alter_table_pre.inc
  --echo error is expected
  --error ER_ILLEGAL_HA_CREATE_OPTION
  eval ALTER TABLE $original_table_name ENGINE=MyISAM,
    ALGORITHM=$alter_table_algorithm;
  --source ../include/xtradb_compressed_columns_alter_table_post.inc
}
