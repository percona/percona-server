SELECT * from INFORMATION_SCHEMA.SESSION_VARIABLES WHERE
(Variable_name =  'log_slow_slave_statements')
or (Variable_name = 'long_query_time')
or (Variable_name = 'use_global_log_slow_control')
or (Variable_name = 'use_global_long_query_time')
ORDER BY Variable_name;
SELECT * from INFORMATION_SCHEMA.GLOBAL_VARIABLES WHERE
(Variable_name =  'log_slow_slave_statements')
or (Variable_name = 'long_query_time')
or (Variable_name = 'use_global_log_slow_control')
or (Variable_name = 'use_global_long_query_time')
ORDER BY Variable_name;
