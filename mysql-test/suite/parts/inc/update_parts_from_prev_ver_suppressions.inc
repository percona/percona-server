--disable_query_log
call mtr.add_suppression("Resizing redo log from");
call mtr.add_suppression("Upgrading redo log");
call mtr.add_suppression("Starting to delete and rewrite log files");
call mtr.add_suppression("New log files created");
call mtr.add_suppression("You need to use --log-bin to make --binlog-format work");
call mtr.add_suppression("Creating routine without parsing routine body");
call mtr.add_suppression("Storage engine '.*' does not support system tables. \\[mysql.*\\]");
call mtr.add_suppression("Table 'mysql.component' doesn't exist");
call mtr.add_suppression("is expected to be transactional");
call mtr.add_suppression("table is missing or has an incorrect definition");
call mtr.add_suppression("ACL DDLs will not work unless mysql_upgrade is executed");
call mtr.add_suppression(".* Native table .* has the wrong structure");
call mtr.add_suppression("Column count of mysql.* is wrong");
call mtr.add_suppression(".*Missing system table mysql.global_grants.*");
call mtr.add_suppression("ACL table mysql.[a-z_]* missing. Some operations may fail.");
call mtr.add_suppression("Info table is not ready to be used. Table 'mysql.slave_master_info' cannot be opened");
call mtr.add_suppression("Error in checking mysql.slave_master_info repository info type of TABLE");
call mtr.add_suppression("Error creating master info: Error checking repositories.");
call mtr.add_suppression("Slave: Failed to initialize the master info structure for channel");
call mtr.add_suppression("Failed to create or recover replication info repositories.");
call mtr.add_suppression("Table .* has length mismatch in the column name table_name.  Please run mysql_upgrade");
# 5.6 suppresions
call mtr.add_suppression("Column count of performance_schema.* is wrong");
call mtr.add_suppression(".* has no `.*` column at position *");
call mtr.add_suppression("Failed to open optimizer cost constant tables");
call mtr.add_suppression(".* table is not ready to be used. Table '.*' cannot be opened.");
call mtr.add_suppression("InnoDB: Cannot open '.*' for reading: No such file or directory");
--enable_query_log
