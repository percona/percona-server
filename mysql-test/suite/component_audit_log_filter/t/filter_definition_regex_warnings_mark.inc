# ==== Purpose ====
#
# Remember the current end of the server error log and the current
# monotonic time, used by filter_definition_regex_warnings.inc.
#
# ==== Usage ====
#
# --source filter_definition_regex_warnings_mark.inc

perl;
  use strict;
  use warnings;
  use Time::HiRes qw(clock_gettime CLOCK_MONOTONIC);

  my $log = "$ENV{MYSQLTEST_VARDIR}/log/mysqld.1.err";
  my $size = -s $log // 0;
  open my $f, '>', "$ENV{MYSQLTEST_VARDIR}/tmp/rx_warnings.mark" or die $!;
  print $f "$size " . clock_gettime(CLOCK_MONOTONIC) . "\n";
  close $f;
EOF
