#
# Arguments:
# - $log_accept_any_innodb_io: if 0 (the default), will grep for zero and non-zero InnoDB I/Os
#   separately. If 1, both cases will be handled by the same grep, to be used when I/O happens
#   non-deterministically and it does not matter for the test.
#
# Common extensions to the slow query log
--let grep_pattern = ^# Schema: .+  Last_errno: \d+  Killed: \d+\$
--let log_expected_matches = $log_slow_verbosity_expected_matches
--source include/log_grep.inc
--let grep_pattern = ^#.*Rows_affected: \d+\$
--source include/log_grep.inc
--let grep_pattern = ^# Bytes_sent: \d+.*\$
--source include/log_grep.inc
# InnoDB
--let log_expected_matches = $log_slow_verbosity_innodb_expected_matches
--let grep_pattern = ^# InnoDB_trx_id: [0-9a-fA-F]+\$
--source include/log_grep.inc
# Query plan
--let grep_pattern = ^# Bytes_sent: \d+  Tmp_tables: \d+  Tmp_disk_tables: \d+  Tmp_table_sizes: \d+\$
--source include/log_grep.inc
--let grep_pattern = ^# Full_scan: (Yes|No)  Full_join: (Yes|No)  Tmp_table: (Yes|No)  Tmp_table_on_disk: (Yes|No)\$
--source include/log_grep.inc
--let grep_pattern = ^# Filesort: (Yes|No)  Filesort_on_disk: (Yes|No)  Merge_passes: \d+\$
--source include/log_grep.inc
# InnoDB
if (!$log_accept_any_innodb_io) {
  # I/O read stats have two cases: all-zeros if fully-cached or all non-zeros. A third case is possible
  # with InnoDB_IO_r_wait !=0 and the other two == 0, this can be added when it's tested
  --let grep_pattern = ^#   InnoDB_IO_r_ops: 0  InnoDB_IO_r_bytes: 0  InnoDB_IO_r_wait: 0\.0*\$
  --source include/log_grep.inc
  --let grep_pattern = ^#   InnoDB_IO_r_ops: [1-9]\d*  InnoDB_IO_r_bytes: [1-9]\d*  InnoDB_IO_r_wait: (0\.\d*[1-9]\d*|[1-9]\d*\.\d+)\$
  --source include/log_grep.inc
}
if ($log_accept_any_innodb_io) {
  --let grep_pattern = ^#   InnoDB_IO_r_ops: \d+  InnoDB_IO_r_bytes: \d+  InnoDB_IO_r_wait: \d+\.\d+\$
  --source include/log_grep.inc
}
# Rec lock wait and queue wait: check format
--let grep_pattern = ^#   InnoDB_rec_lock_wait: \d*\.\d*  InnoDB_queue_wait: \d*\.\d*\$
--source include/log_grep.inc
# Rec lock wait: check value, either zero or non-zero
--let grep_pattern = InnoDB_rec_lock_wait: 0\.0*
--source include/log_grep.inc
--let grep_pattern = InnoDB_rec_lock_wait: (0\.\d*[1-9]\d*|[1-9]\d*\.\d+)
--source include/log_grep.inc
# Queue wait: check value, either zero or non-zero
--let grep_pattern = InnoDB_queue_wait: 0\.0*
--source include/log_grep.inc
--let grep_pattern = InnoDB_queue_wait: (0\.\d*[1-9]\d*|[1-9]\d*\.\d+)
--source include/log_grep.inc
# If present, InnoDB distinct pages are always non-zero
--let grep_pattern = ^#   InnoDB_pages_distinct: [1-9]\d*\$
--source include/log_grep.inc
--let grep_pattern = ^# No InnoDB statistics available for this query\$
--source include/log_grep.inc
