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
--disconnect connection_one
--disconnect connection_two
--disconnect connection_three
--source include/wait_until_count_sessions.inc
# we disable slow query log AFTER disconnects here to make sure that all
# "SELECT 'connection_xxx'" statements are 100% processed (including logging
# slow queries) and we do not close slow query log file in one connection
# while other ones, althought already send the result back to the client,
# may still need to finish remaining tasks (like writing
# to slog)
--source include/log_stop.inc
--let log_slow_rate_test=1
--source include/log_grep.inc
