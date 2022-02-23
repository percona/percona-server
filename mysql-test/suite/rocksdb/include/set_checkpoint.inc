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
  --let $zenfs_data_dir_prefix =
  --let $rocksdb_data_dir_prefix =
  --source ./zenfs_file_exists.inc

  # Cleanup
  --let $directory_to_force_delete = $checkpoint
  --source ./zenfs_force_rmdir.inc
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
