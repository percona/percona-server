--disable_query_log
--let log_file_full_path = $MYSQLTEST_VARDIR/$log_file
SET @slow_query_log_old=@@slow_query_log;
SET @slow_query_log_file_old= @@slow_query_log_file;
SET GLOBAL slow_query_log=OFF;
perl;
  $log_file_full_path= $ENV{'log_file_full_path'};
  unlink $log_file_full_path;
  open(FILE, '>', $log_file_full_path)
    or die "Cannot create log file $log_file_full_path, reason: $!";
  close(FILE);
EOF
--echo [log_start.inc] $log_file
EVAL SET GLOBAL slow_query_log_file="$log_file_full_path";
SET GLOBAL slow_query_log=ON;
--enable_query_log
