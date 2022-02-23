# Expected arguments:
# $directory_to_force_delete - Directory that should get force deleted
#

if ($directory_to_force_delete == '')
{
  --echo '$directory_to_force_delete' is empty
  --die zenfs_force_rmdir.inc assertion failed
}

--source parse_rocksdb_fs_uri.inc

--source include/stop_mysqld.inc
if ($rocksdb_zenfs_disabled)
{
  --force-rmdir $directory_to_force_delete
}
if (!$rocksdb_zenfs_disabled)
{
  --let $zenfs_cmd = $MYSQL_ZENFS rmdir --zbd=$extracted_zenfs_device --path=$directory_to_force_delete --force > /dev/null

  --exec $zenfs_cmd
}

--source include/start_mysqld.inc
