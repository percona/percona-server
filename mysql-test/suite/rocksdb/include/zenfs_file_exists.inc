# Expected arguments:
# $file_exists_file_name - file name to check
# $file_exists_inverted - if non-zero, $file_exists_file_name must not exist
#

--source parse_rocksdb_fs_uri.inc
if ($rocksdb_zenfs_disabled)
{

  --let $ROCKSDB_DATADIR = `SELECT CONCAT(@@datadir, '/', @@rocksdb_datadir)`
  if ($file_exists_inverted)
  {
    --error 1
  }
  --file_exists $ROCKSDB_DATADIR/$file_exists_file_name
}
if (!$rocksdb_zenfs_disabled)
{
  --file_exists $MYSQL_ZENFS

  --let $zenfs_data_dir_prefix = ./.rocksdb

  --let $extracted_zenfs_file_name = `SELECT SUBSTRING_INDEX('$file_exists_file_name', '/', -1)`
  --let $extracted_zenfs_path = `SELECT LEFT('$file_exists_file_name', LENGTH('$file_exists_file_name') - LENGTH('$extracted_zenfs_file_name') - 1)`

  --let $zenfs_file_exists_zenfs_result_file = $MYSQL_TMP_DIR/zfe_zenfs_result.txt
  --let $zenfs_file_exists_awk_result_file = $MYSQL_TMP_DIR/zfe_awk_result.txt
  --let $zenfs_file_exists_grep_result_file = $MYSQL_TMP_DIR/zfe_grep_result.txt

  --let $zenfs_cmd = $MYSQL_ZENFS list --zbd=$extracted_zenfs_device --path=$zenfs_data_dir_prefix$extracted_zenfs_path > $zenfs_file_exists_zenfs_result_file
  --exec $zenfs_cmd

  --let $awk_cmd = awk '{ print \$6 }' $zenfs_file_exists_zenfs_result_file > $zenfs_file_exists_awk_result_file
  --exec $awk_cmd

  --exec echo -n '--let ZENFS_GREP_MATCHED_LINES = ' > $zenfs_file_exists_grep_result_file
  --let $grep_cmd = grep --fixed-strings --line-regexp --count $extracted_zenfs_file_name $zenfs_file_exists_awk_result_file >> $zenfs_file_exists_grep_result_file
  --error 0,1
  --exec $grep_cmd
  --source $zenfs_file_exists_grep_result_file

  --remove_file $zenfs_file_exists_grep_result_file
  --remove_file $zenfs_file_exists_awk_result_file
  --remove_file $zenfs_file_exists_zenfs_result_file

  if ($file_exists_inverted)
  {
    if($ZENFS_GREP_MATCHED_LINES != 0)
    {
      --echo '$file_exists_file_name' exists
      --die zenfs_file_exists.inc assertion failed
    }
  }
  if (!$file_exists_inverted)
  {
    if($ZENFS_GREP_MATCHED_LINES != 1)
    {
      --echo '$file_exists_file_name' does not exist
      --die zenfs_file_exists.inc assertion failed
    }
  }
}
