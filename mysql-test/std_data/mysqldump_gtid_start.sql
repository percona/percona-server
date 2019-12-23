# This file will be executed at the start of mysqldump

SET DEBUG_SYNC='gtid_executed_before_get_value SIGNAL gtid_executed.started WAIT_FOR gtid_executed.continue';
