# ==== Purpose ====
#
# Calculate current log size and overall log size (including rotated files)
# and compare it to expected values.
#
# ==== Usage ====
#
# --let $audit_filter_log_path = path to dir containing audit filter log
# --let $audit_filter_log_name = audit filter log file name
# --let $expected_current_log_size = expected current log size
# --let $expected_total_log_size = expected total logs size
# --source check_current_total_logs_size.inc
#
# Parameters:
#
# $audit_filter_log_path
#   Path to dir containing audit filter log.
# $audit_filter_log_name
#   Audit filter log file name.
# $expected_current_log_size
#   Expected current log size.
# $expected_total_log_size
#   Expected total logs size.

--let audit_filter_log_path = $audit_filter_log_path
--let audit_filter_log_name = $audit_filter_log_name
--let expected_current_log_size = $expected_current_log_size
--let expected_total_log_size = $expected_total_log_size

perl;
  use strict;
  use warnings;
  use File::stat;

  my $current_log_name = $ENV{'audit_filter_log_name'};
  my $log_base_name = $ENV{'audit_filter_log_name'} =~ s/\.log//r;
  my $log_path = $ENV{'audit_filter_log_path'};

  my $expected_current_log_size = $ENV{'expected_current_log_size'};
  my $expected_total_log_size = $ENV{'expected_total_log_size'};

  opendir(my $dh, $log_path) or die "Can't opendir $log_path: $!";

  my $log_current_size = 0;
  my $log_total_size = 0;

  while ((my $filename = readdir($dh))) {
    if ($filename =~ /^$current_log_name/) {
      my $stat = stat("$log_path$filename");
      $log_current_size = $stat->size;
      next;
    }

    if ($filename =~ /^$log_base_name/ && $filename =~ /\.(\d{8}T\d{6})\./) {
      my $stat = stat("$log_path$filename");
      $log_total_size += $stat->size;
    }
  }

  $log_total_size += $log_current_size;

  closedir $dh;

  if ($log_current_size == $expected_current_log_size) {
    print "Audit_log_filter_current_size Ok\n";
  }
  else {
    print "Expected Audit_log_filter_current_size '$expected_current_log_size' doesn't match actual log size '$log_current_size'\n";
  }

  if ($log_total_size == $expected_total_log_size) {
    print "Audit_log_filter_total_size Ok\n";
  }
  else {
    print "Expected Audit_log_filter_total_size '$expected_total_log_size' doesn't match actual log size '$log_total_size'\n";
  }
EOF
