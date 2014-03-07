#
# Include this file in a session after fake changes workload.  It will disable the fake 
# changes and assert that the relevant server state has not changed from the one saved
# by a previous start_fake_changes.inc.
#
SET innodb_fake_changes=0;

eval CHECK TABLE $fake_changes_table;

let $t_checksum_2= `CHECKSUM TABLE $fake_changes_table EXTENDED`;
SELECT VARIABLE_VALUE INTO @innodb_rows_inserted_2 FROM INFORMATION_SCHEMA.GLOBAL_STATUS WHERE VARIABLE_NAME = 'Innodb_rows_inserted';
SELECT VARIABLE_VALUE INTO @innodb_rows_deleted_2 FROM INFORMATION_SCHEMA.GLOBAL_STATUS WHERE VARIABLE_NAME='Innodb_rows_deleted';
SELECT VARIABLE_VALUE INTO @innodb_rows_updated_2 FROM INFORMATION_SCHEMA.GLOBAL_STATUS WHERE VARIABLE_NAME='Innodb_rows_updated';
eval SELECT ROWS INTO @table_rows_estimate_2 FROM INFORMATION_SCHEMA.INNODB_TABLE_STATS WHERE TABLE_SCHEMA LIKE 'test' AND TABLE_NAME LIKE '$fake_changes_table';
eval SELECT ROWS_CHANGED INTO @table_rows_changed_2 FROM INFORMATION_SCHEMA.TABLE_STATISTICS WHERE TABLE_SCHEMA LIKE 'test' AND TABLE_NAME LIKE '$fake_changes_table';
eval SELECT ROWS_CHANGED_X_INDEXES INTO @table_rows_changed_x_indexes_2 FROM INFORMATION_SCHEMA.TABLE_STATISTICS WHERE TABLE_SCHEMA LIKE 'test' AND TABLE_NAME LIKE '$fake_changes_table';

--disable_query_log
eval SELECT "$t_checksum_1" LIKE "$t_checksum_2" AS should_be_1;
--enable_query_log
SELECT @innodb_rows_inserted_2 - @innodb_rows_inserted_1 AS should_be_0;
SELECT @innodb_rows_deleted_2 - @innodb_rows_deleted_1 AS should_be_0;
SELECT @innodb_rows_updated_2 - @innodb_rows_updated_1 AS should_be_0;
SELECT @table_rows_estimate_2 - @table_rows_estimate_1 AS should_be_0;
SELECT @table_rows_changed_2 - @table_rows_changed_1 AS should_be_0;
SELECT @table_rows_changed_x_indexes_2 - @table_rows_changed_x_indexes_1 AS should_be_0;

# Disabled until https://bugs.launchpad.net/percona-server/+bug/1201802 is fixed.
# 
# #
# # Index stats should not change, but it's only possible to inspect them 
# # manually.
# #
# eval SHOW INDEXES IN $fake_changes_table;
