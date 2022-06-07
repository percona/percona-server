#
# Check if server has support for loading plugins
#
if (`SELECT @@have_dynamic_loading != 'YES'`) {
  --skip RocksDB requires dynamic loading
}

if (!$ROCKSDB) {
  --skip RocksDB requires the environment variable \$ROCKSDB to be set (normally done by mtr)
}

#
# Check if --plugin-dir was setup for ROCKSDB
#
if (`SELECT CONCAT('--plugin-dir=', REPLACE(@@plugin_dir, '\\\\', '/')) != '$ROCKSDB_OPT/'`) {
  --skip RocksDB requires that --plugin-dir is set to the RocksDB plugin dir (either the .opt file does not contain \$ROCKSDB_OPT or another plugin is in use)
}
