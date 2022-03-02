# Output parameters:
# $rocksdb_zenfs_disabled - 1 if ZenFS is disabled, 0 if ZenFS is enabled
# $extracted_zenfs_device - ZenFS zoned block device name is put here when $rocksdb_zenfs_disabled == 0,
#                           e.g. 'nullb0'
#

--let $rocksdb_zenfs_disabled = `SELECT @@rocksdb_fs_uri IS NULL OR @@rocksdb_fs_uri = ''`
if (!$rocksdb_zenfs_disabled)
{
  --let $zenfs_fs_uri_prefix = zenfs://dev:
  if (`SELECT LOCATE('$zenfs_fs_uri_prefix', @@rocksdb_fs_uri) != 1`)
  {
    --echo @@rocksdb_fs_uri must start with '$zenfs_fs_uri_prefix' prefix
    --die @@rocksdb_fs_uri starts with invalid prefix
  }
  --let $extracted_zenfs_device = `SELECT RIGHT(@@rocksdb_fs_uri, LENGTH(@@rocksdb_fs_uri) - LENGTH('$zenfs_fs_uri_prefix'))`
}
