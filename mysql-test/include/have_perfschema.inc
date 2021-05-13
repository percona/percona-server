# Performance Schema is enabled by default but can be deactivated on startup

--let $_have_jemalloc= query_get_value(SHOW VARIABLES LIKE 'performance_schema', Value, 1)
if ($_have_jemalloc != ON)
{
  --skip Need performance schema on
}
