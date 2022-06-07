if (`select count(*) = 0 from performance_schema.global_variables where lower(variable_name) = 'innodb_buffer_pool_instances' and variable_value >= 2`)
{
  --skip Test requires InnoDB with at least two buffer pool instances
}
