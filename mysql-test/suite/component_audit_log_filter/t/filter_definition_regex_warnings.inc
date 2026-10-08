# ==== Purpose ====
#
# Count server error log lines containing a given text written since
# filter_definition_regex_warnings_mark.inc was sourced. A run lasting at
# least 60 seconds since the mark may get further rate limited warnings,
# $rx_warning_slow_max is then the highest accepted count.
#
# ==== Usage ====
#
# --let rx_warning_text = literal text searched for in each line
# --let rx_warning_count = expected number of lines
# [--let rx_warning_slow_max = highest number accepted on slow runs]
# [--let rx_warning_keep_mark = 1, keep the mark for another check]
# --let rx_warning_label = description printed into the result
# --source filter_definition_regex_warnings.inc

--echo # Warnings: $rx_warning_label
perl;
  use strict;
  use warnings;
  use Time::HiRes qw(clock_gettime CLOCK_MONOTONIC);

  open my $m, '<', "$ENV{MYSQLTEST_VARDIR}/tmp/rx_warnings.mark" or die $!;
  my ($offset, $start) = split ' ', <$m>;
  close $m;
  unlink "$ENV{MYSQLTEST_VARDIR}/tmp/rx_warnings.mark"
    unless $ENV{rx_warning_keep_mark};
  my $elapsed = clock_gettime(CLOCK_MONOTONIC) - $start;

  my $log = "$ENV{MYSQLTEST_VARDIR}/log/mysqld.1.err";
  open my $f, '<:raw', $log or die "$log: $!";
  seek $f, $offset, 0 or die $!;
  my $count = 0;
  while (my $line = <$f>) {
    $count++ if index($line, $ENV{rx_warning_text}) >= 0;
  }
  close $f;

  my $expected = $ENV{rx_warning_count};
  my $max = $expected;
  $max = $ENV{rx_warning_slow_max}
    if $elapsed >= 60 && ($ENV{rx_warning_slow_max} // '') ne '';
  die "Expected $expected" . ($max != $expected ? "..$max" : '') .
      " lines containing '$ENV{rx_warning_text}' in ${elapsed}s, found $count"
    unless $count >= $expected && $count <= $max;
EOF
--let rx_warning_slow_max =
