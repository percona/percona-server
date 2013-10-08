--connection default

--source include/count_sessions.inc
--connect (connection_one,localhost,root,,)
--connect (connection_two,localhost,root,,)
--connect (connection_three,localhost,root,,)

--source include/log_start.inc

--let i=2
--let k=1

while($i)
{
--connection connection_one
inc $k;
SELECT 'connection_one';

--connection connection_two
inc $k;
SELECT 'connection_two';

--connection connection_three
inc $k;
SELECT 'connection_three';

dec $i;
}

--connection default
--source include/log_stop.inc
--disconnect connection_one
--disconnect connection_two
--disconnect connection_three
--source include/wait_until_count_sessions.inc
--let log_slow_rate_test=1
--source include/log_grep.inc
