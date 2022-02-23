# Expected arguments:
# $disk_usage_path - path to check files in
# $disk_usage_mask - pattern to filter files in $disk_usage_path
# $disk_usage_output_file - file name to write calculated files size into
# $disk_usage_output_file_overwrite - if set to non-zero, $disk_usage_output_file will be overwritten, otherwise appended
# $disk_usage_output_prefix - prefix to add to the output size (must be double-quoted, can be empty)
# $disk_usage_output_suffix - suffix to add to the output size (must be double-quoted, can be empty)
#

if ($rocksdb_zenfs_disabled)
{
  --let $datadir = `SELECT @@datadir`
  --let $size_cmd = du -b $datadir$disk_usage_path/*$disk_usage_mask*
}
if (!$rocksdb_zenfs_disabled)
{
  --file_exists $MYSQL_ZENFS
  --let $size_cmd = $MYSQL_ZENFS list --zbd=$extracted_zenfs_device --path=/$disk_usage_path | grep '$disk_usage_mask'
}

--let $size_cmd = $size_cmd | awk '{ sum = sum + \$1 } END { print $disk_usage_output_prefix int(sum / 1024) $disk_usage_output_suffix }'

if ($disk_usage_output_file_overwrite)
{
  --let $size_cmd = $size_cmd >
}
if (!$disk_usage_output_file_overwrite)
{
  --let $size_cmd = $size_cmd >>
}
--let $size_cmd = $size_cmd $disk_usage_output_file
--exec $size_cmd
