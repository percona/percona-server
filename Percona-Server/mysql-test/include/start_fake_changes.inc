#
# Include this file in a session before fake changes workload.  It will enable 
# the fake changes for the current session and save the relevant server state 
# to variables that will be used by stop_fake_changes.inc that it didn't change.
#
# Since some of the state checked is global InnoDB counters, do not issue a 
# non-fake-changes workload in parallel.
#

# Requires userstat
SELECT @@global.userstat = 1 AS should_be_1;

# Have up-to-date stats for the fake changes table, so that, if there is a
# non-fake-changes session running in parallel, it does not trigger an auto
# stat recalc which then fails these checks.

# Requires large index dive sample pages values
SELECT @@global.innodb_stats_transient_sample_pages = 30000 AS should_be_1;

eval ANALYZE TABLE $fake_changes_table;

let $t_checksum_1= `CHECKSUM TABLE $fake_changes_table EXTENDED`;
SELECT VARIABLE_VALUE INTO @innodb_rows_inserted_1 FROM INFORMATION_SCHEMA.GLOBAL_STATUS WHERE VARIABLE_NAME = 'Innodb_rows_inserted';
SELECT VARIABLE_VALUE INTO @innodb_rows_deleted_1 FROM INFORMATION_SCHEMA.GLOBAL_STATUS WHERE VARIABLE_NAME = 'Innodb_rows_deleted';
SELECT VARIABLE_VALUE INTO @innodb_rows_updated_1 FROM INFORMATION_SCHEMA.GLOBAL_STATUS WHERE VARIABLE_NAME = 'Innodb_rows_updated';
eval SELECT NUM_ROWS INTO @table_rows_estimate_1 FROM INFORMATION_SCHEMA.INNODB_SYS_TABLESTATS WHERE NAME LIKE 'test/$fake_changes_table';
eval SELECT ROWS_CHANGED INTO @table_rows_changed_1 FROM INFORMATION_SCHEMA.TABLE_STATISTICS WHERE TABLE_SCHEMA LIKE 'test' AND TABLE_NAME LIKE '$fake_changes_table';
eval SELECT ROWS_CHANGED_X_INDEXES INTO @table_rows_changed_x_indexes_1 FROM INFORMATION_SCHEMA.TABLE_STATISTICS WHERE TABLE_SCHEMA LIKE 'test' AND TABLE_NAME LIKE '$fake_changes_table';
eval SELECT MODIFIED_COUNTER INTO @table_dml_counter_1 FROM INFORMATION_SCHEMA.INNODB_SYS_TABLESTATS WHERE NAME LIKE 'test/$fake_changes_table';

#
# Index stats should not change, but it's only possible to inspect them 
# manually.
#
eval SHOW INDEXES IN $fake_changes_table;

SET innodb_fake_changes=1;
