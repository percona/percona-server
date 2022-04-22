if (`select count(*) = 0 from performance_schema.global_variables where variable_name = 'rocksdb_enable_delete_range_for_drop_index' and variable_value = "OFF";`) {
  --skip Test requires rocksdb_enable_delete_range_for_drop_index to OFF
}
