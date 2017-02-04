SET @userstat_old= @@userstat;
SET GLOBAL userstat=ON;

CREATE USER mysqltest_1@localhost;
CREATE USER mysqltest_2@localhost;

# Make a single connection for mysqltest_1 user
connect (conn1,localhost,mysqltest_1,,);
SELECT 1;
connection default;

--let $assert_text= I_S.USER_STATISTICS CONCURRENT_CONNECTIONS: single connection
--let $query_result= query_get_value(SELECT CONCURRENT_CONNECTIONS FROM INFORMATION_SCHEMA.USER_STATISTICS WHERE USER LIKE 'mysqltest_1', CONCURRENT_CONNECTIONS, 1)
--let $assert_cond= "$query_result" = 1
--source include/assert.inc

# Make two connections for mysqltest_1 user
connect (conn2,localhost,mysqltest_1,,);
SELECT 1;
connection default;

--let $assert_text= I_S.USER_STATISTICS CONCURRENT_CONNECTIONS: two connections
--let $query_result= query_get_value(SELECT CONCURRENT_CONNECTIONS FROM INFORMATION_SCHEMA.USER_STATISTICS WHERE USER LIKE 'mysqltest_1', CONCURRENT_CONNECTIONS, 1)
--let $assert_cond= "$query_result" = 2
--source include/assert.inc

# Check if number of concurrent connections decreased
disconnect conn2;
let $count_sessions= 2;
--source include/wait_until_count_sessions.inc

--let $assert_text= I_S.USER_STATISTICS CONCURRENT_CONNECTIONS: handle disconnects
--let $query_result= query_get_value(SELECT CONCURRENT_CONNECTIONS FROM INFORMATION_SCHEMA.USER_STATISTICS WHERE USER LIKE 'mysqltest_1', CONCURRENT_CONNECTIONS, 1)
--let $assert_cond= "$query_result" = 1
--source include/assert.inc

# Number of concurrent connections shouldn't be affected by different users
connect (conn3,localhost,mysqltest_2,,);
SELECT 1;
connection default;

--let $assert_text= I_S.USER_STATISTICS CONCURRENT_CONNECTIONS: connections from other users
--let $query_result= query_get_value(SELECT CONCURRENT_CONNECTIONS FROM INFORMATION_SCHEMA.USER_STATISTICS WHERE USER LIKE 'mysqltest_1', CONCURRENT_CONNECTIONS, 1)
--let $assert_cond= "$query_result" = 1
--source include/assert.inc


# if there is no connections, concurrent connections counter should be zero
disconnect conn1;
let $count_sessions= 2;
--source include/wait_until_count_sessions.inc

--let $assert_text= I_S.USER_STATISTICS CONCURRENT_CONNECTIONS: no connections - zero concurrency
--let $query_result= query_get_value(SELECT CONCURRENT_CONNECTIONS FROM INFORMATION_SCHEMA.USER_STATISTICS WHERE USER LIKE 'mysqltest_1', CONCURRENT_CONNECTIONS, 1)
--let $assert_cond= "$query_result" = 0
--source include/assert.inc

disconnect conn3;
let $count_sessions= 1;
--source include/wait_until_count_sessions.inc

# Unknown users are counted in USER_STATISTICS, concurrent connections should be positive
--disable_query_log
--error ER_ACCESS_DENIED_ERROR
connect (conn4,localhost,mysqltest_unknownuser,,);
--enable_query_log
connection default;

--let $assert_text= I_S.USER_STATISTICS CONCURRENT_CONNECTIONS: unknown user connection
--let $query_result= query_get_value(SELECT CONCURRENT_CONNECTIONS FROM INFORMATION_SCHEMA.USER_STATISTICS WHERE USER LIKE 'mysqltest_unknownuser', CONCURRENT_CONNECTIONS, 1)
--let $assert_cond= "$query_result" = 0
--source include/assert.inc


DROP USER mysqltest_1@localhost;
DROP USER mysqltest_2@localhost;

SET GLOBAL userstat= @userstat_old;
