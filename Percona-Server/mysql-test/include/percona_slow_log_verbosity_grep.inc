# Common extensions to the slow query log
--let grep_pattern = ^# Thread_id: \d+  Schema: .+  Last_errno: \d+  Killed: \d+\$
--source include/log_grep.inc
--let grep_pattern = ^#.*Rows_affected: \d+  Rows_read: \d+\$
--source include/log_grep.inc
--let grep_pattern = ^# Bytes_sent: \d+.*\$
--source include/log_grep.inc
# InnoDB
--let grep_pattern = ^# InnoDB_trx_id: \w+\$
--source include/log_grep.inc
# Query plan
--let grep_pattern = ^# Bytes_sent: \d+  Tmp_tables: \d+  Tmp_disk_tables: \d+  Tmp_table_sizes: \d+\$
--source include/log_grep.inc
--let grep_pattern = ^# QC_Hit: (Yes|No)  Full_scan: (Yes|No)  Full_join: (Yes|No)  Tmp_table: (Yes|No)  Tmp_table_on_disk: (Yes|No)\$
--source include/log_grep.inc
--let grep_pattern = ^# Filesort: (Yes|No)  Filesort_on_disk: (Yes|No)  Merge_passes: \d+\$
--source include/log_grep.inc
# InnoDB
--let grep_pattern = ^#   InnoDB_IO_r_ops: \d+  InnoDB_IO_r_bytes: \d+  InnoDB_IO_r_wait: \d*\.\d*\$
--source include/log_grep.inc
--let grep_pattern = ^#   InnoDB_rec_lock_wait: \d*\.\d*  InnoDB_queue_wait: \d*\.\d*\$
--source include/log_grep.inc
--let grep_pattern = ^#   InnoDB_pages_distinct: \d+\$
--source include/log_grep.inc
--let grep_pattern = ^# No InnoDB statistics available for this query\$
--source include/log_grep.inc
