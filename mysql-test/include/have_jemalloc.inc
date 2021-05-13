# ==== Purpose ====
# 
# Ensure that the server is running with Jemalloc loaded and profiling enabled
#
# ==== Usage ====
#
# source include/have_jemalloc.inc;

--let $_have_jemalloc= query_get_value(SHOW VARIABLES LIKE 'jemalloc_detected', Value, 1)
if ($_have_jemalloc != ON)
{
  --skip Test requires Jemalloc library loaded and profiling enabled
}
