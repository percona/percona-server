# This file will be executed at the start of mysqldump

SET DEBUG='+d,catch_show_gtid_mode';
SET DEBUG_SYNC='before_show_gtid_executed SIGNAL before_show_gtid_executed.reached WAIT_FOR before_show_gtid_executed.continue';
