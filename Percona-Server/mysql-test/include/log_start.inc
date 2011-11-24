--let slow_query_log_file_old=`SELECT Variable_value FROM INFORMATION_SCHEMA.GLOBAL_VARIABLES WHERE Variable_name = 'slow_query_log_file';`
--let slow_query_log_old=`SELECT Variable_value FROM INFORMATION_SCHEMA.GLOBAL_VARIABLES WHERE Variable_name = 'slow_query_log';`
--disable_query_log
--let log_file_full_path = $MYSQLTEST_VARDIR/$log_file
SET GLOBAL slow_query_log=0;
perl;
  $log_file_full_path= $ENV{'log_file_full_path'};
  unlink $log_file_full_path;
  open(FILE, '>', $log_file_full_path)
    or die "Cannot create log file $log_file_full_path, reason: $!";
  close(FILE);
EOF
--echo [log_start.inc] $log_file
EVAL SET GLOBAL slow_query_log_file="$log_file_full_path";
SET GLOBAL slow_query_log=1;
--enable_query_log
