if (`select count(*) = 0 from performance_schema.session_variables where variable_name = 'rocksdb_enable_native_partition' and variable_value = 'on';`) {
  --skip Test requires native partitioning enabled
}
