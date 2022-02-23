# Usage:
# let $checkpoint = <value to set the checkpoint>;
# let $succeeds = <1 if checkpoint creation should succeed, 0 otherwise>;
# --source suite/rocksdb/include/set_checkpoint.inc


if ($succeeds)
{
  # Create checkpoint
  --replace_result '$checkpoint' [CHECKPOINT]
  eval SET GLOBAL ROCKSDB_CREATE_CHECKPOINT = '$checkpoint';

  # Check checkpoint
  --let $file_exists_file_name = $checkpoint/CURRENT
  --let $file_exists_inverted = 0
  --source zenfs_file_exists.inc

  # Cleanup
  if (!$rocksdb_zenfs_disabled)
  {
    --source include/shutdown_mysqld.inc
  }
  --let $directory_to_force_delete = $checkpoint
  --source zenfs_force_rmdir.inc
  if ($rocksdb_zenfs_disabled)
  {
    # fake "# restart" message written to the test log to get identical results in ZenFS / non-ZenFS modes
    --echo # restart
  }
  if (!$rocksdb_zenfs_disabled)
  {
    --source include/start_mysqld.inc
  }
}
if (!$succeeds)
{
  --disable_result_log
  --disable_query_log
  --error ER_GET_ERRMSG
  eval SET GLOBAL ROCKSDB_CREATE_CHECKPOINT = '$checkpoint';
  --enable_query_log
  --enable_result_log
}
