# ==== Purpose ====
#
# Check log rotation worked.
#
# ==== Usage ====
#
# --let $log_path = path to dir containing audit filter log
# --source check_rotated_logs.inc
#
# Parameters:
#
# $log_path
#   Path to dir containing audit filter log.

--let log_path = $log_path

perl;
  eval "use XML::Parser; 1" or exit 0;
  my $p = new XML::Parser;
  my $logs_counter = 0;

  opendir(my $dh, $ENV{'log_path'}) || die "Can't opendir $ENV{'log_path'}: $!";

  while (readdir $dh) {
    if ($_ =~ /test_audit_filter/) {
      $logs_counter += 1;
      $p->parsefile("$ENV{'log_path'}$_");
    }
  }

  closedir $dh;

  if ($logs_counter > 1) {
    print "Rotation Ok\n";
  }
  else {
    print "Rotation doesn't work, logs_counter: $logs_counter\n";
  }
EOF
