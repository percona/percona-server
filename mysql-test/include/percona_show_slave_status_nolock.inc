--echo
--disable_result_log
connection slave_lock;
--echo [slave_lock]
send SHOW SLAVE STATUS;

connection slave;
--let $condition= 'SHOW SLAVE STATUS'
--source include/wait_show_condition.inc

--disable_warnings
SET DEBUG_SYNC='now WAIT_FOR signal.after_show_slave_status TIMEOUT 1';
--enable_warnings

--let current=`SELECT SUBSTR(Variable_value FROM 22) FROM INFORMATION_SCHEMA.SESSION_VARIABLES WHERE Variable_name = 'DEBUG_SYNC'`
--echo SIGNAL after SHOW SLAVE STATUS is $current

connection slave;
--echo [slave]
SET DEBUG_SYNC='now SIGNAL signal.empty';

connection slave_nolock;
--echo [slave_nolock]
send SHOW SLAVE STATUS NONBLOCKING;

connection slave;
--let $condition= 'SHOW SLAVE STATUS NONBLOCKING'
--source include/wait_show_condition.inc

--disable_warnings
SET DEBUG_SYNC='now WAIT_FOR signal.after_show_slave_status TIMEOUT 1';
--enable_warnings

--echo # should be 'signal.after_show_slave_status'
--let current=`SELECT SUBSTR(Variable_value FROM 22) FROM INFORMATION_SCHEMA.SESSION_VARIABLES WHERE Variable_name = 'DEBUG_SYNC'`
--echo SIGNAL after SHOW SLAVE STATUS NONBLOCKING is $current

connection slave;
--echo [slave]
SET DEBUG_SYNC='now SIGNAL signal.continue';

connection slave_lock;
--disable_result_log
reap;
--enable_result_log

connection slave_nolock;
--disable_result_log
reap;
--enable_result_log

connection slave;
--echo [slave]
SET DEBUG_SYNC='now SIGNAL signal.empty';
--enable_result_log
--echo
