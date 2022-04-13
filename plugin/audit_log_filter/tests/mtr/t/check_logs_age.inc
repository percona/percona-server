# ==== Purpose ====
#
# Check audit filter log files age doesn't exceed expected value.
#
# ==== Usage ====
#
# --let $log_path = path to dir containing audit filter log
# --let $log_name = audit filter log file name
# --let $prune_seconds = number of seconds after which audit filter log files
#                        become subject to pruning
# --source check_logs_age.inc
#
# Parameters:
#
# $log_path
#   Path to dir containing audit filter log.
# $log_name
#   Audit filter log file name.
# $prune_seconds
#   Number of seconds after which audit filter log files become
#   subject to pruning.

--let log_path = $log_path
--let log_name = $log_name
--let prune_seconds = $prune_seconds

perl;
  use strict;
  use Date::Parse;
  use List::Util qw(min max);

  my $prune_seconds = $ENV{'prune_seconds'};
  my $log_name = $ENV{'log_name'};
  my $log_base_name = $ENV{'log_name'} =~ s/\.log//r;
  my $timestamps = [];

  opendir(my $dh, $ENV{'log_path'}) || die "Can't opendir $ENV{'log_path'}: $!";

  while ((my $filename = readdir($dh))) {
    if ($filename =~ /^$log_base_name/ && $filename =~ /\.(\d{8}T\d{6})\./) {
      push(@$timestamps, str2time($1));
    }
  }

  closedir $dh;

  my $min_time = min(@$timestamps);
  my $max_time = max(@$timestamps);

  if ($max_time - $min_time > $prune_seconds) {
    print "Logs age exceeds expected prune_seconds $prune_seconds\n";
  }
  else {
    print "Logs age doesn't exceed expected prune_seconds $prune_seconds\n";
  }
EOF
