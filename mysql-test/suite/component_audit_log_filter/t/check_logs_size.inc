# ==== Purpose ====
#
# Check the overall size of audit filter log files.
#
# ==== Usage ====
#
# --let $log_path = path to dir containing audit filter log
# --let $log_name = audit filter log file name
# --let $log_max_size = maximum expected logs size
# --source check_logs_size.inc
#
# Parameters:
#
# $log_path
#   Path to dir containing audit filter log.
# $log_name
#   Audit filter log file name.
# $log_max_size
#   Maximum expected logs size.

--let log_path = $log_path
--let log_name = $log_name
--let log_max_size = $log_max_size

perl;
  use strict;
  use File::stat;

  my $actual_log_size = 0;
  my $log_max_size = $ENV{'log_max_size'};
  my $log_base_name = $ENV{'log_name'} =~ s/\..*//r;

  opendir(my $dh, $ENV{'log_path'}) || die "Can't opendir $ENV{'log_path'}: $!";

  while ((my $filename = readdir($dh))) {
    if ($filename =~ /^$log_base_name/) {
      my $stat = stat("$ENV{log_path}$filename");
      $actual_log_size += $stat->size;
    }
  }

  closedir $dh;

  if ($actual_log_size > $log_max_size) {
    print "Logs size exceeds expected $log_max_size bytes\n";
  }
  else {
    print "Logs size doesn't exceed expected $log_max_size bytes\n";
  }
EOF
