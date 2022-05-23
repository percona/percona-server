# Expected arguments:
# $directory_to_force_delete - Directory that should get force deleted
#

if ($directory_to_force_delete == '')
{
  --echo '$directory_to_force_delete' is empty
  --die zenfs_force_rmdir.inc assertion failed
}

if ($rocksdb_zenfs_disabled)
{
  --force-rmdir $directory_to_force_delete
}
if (!$rocksdb_zenfs_disabled)
{
  --file_exists $MYSQL_ZENFS

  --let $zenfs_cmd = $MYSQL_ZENFS rmdir --zbd=$extracted_zenfs_device --path=$directory_to_force_delete --force > /dev/null 2>&1
  --exec $zenfs_cmd
}
