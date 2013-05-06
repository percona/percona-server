--let wait_condition=SELECT COUNT(*) = 1 FROM INFORMATION_SCHEMA.PROCESSLIST;
--connection default
--source include/log_start.inc

--connect (connection_one,localhost,root,,)
--connect (connection_two,localhost,root,,)
--connect (connection_three,localhost,root,,)

--let i=2
--let k=1

while($i)
{
--connection connection_one
SET SESSION query_exec_time=1.1;
eval SET SESSION query_exec_id=$k;
inc $k;
SELECT 'connection_one';

--connection connection_two
SET SESSION query_exec_time=1.1;
eval SET SESSION query_exec_id=$k;
inc $k;
SELECT 'connection_two';

--connection connection_three
SET SESSION query_exec_time=1.1;
eval SET SESSION query_exec_id=$k;
inc $k;
SELECT 'connection_three';

dec $i;
}

--connection default
--disconnect connection_one
--disconnect connection_two
--disconnect connection_three
--source include/wait_condition.inc
--source include/log_stop.inc
--let log_slow_rate_test=1
--source include/log_grep.inc
