# Use only after every donor, receiver and writer transaction has ended.
# A fresh explicit read view must have equal visibility limits, because
# no ordinary or reserved transaction id should remain active.

START TRANSACTION WITH CONSISTENT SNAPSHOT;
--let CLONE_SNAPSHOT_STATUS= query_get_value(SHOW ENGINE INNODB STATUS, Status, 1)
perl;
  use strict;
  use warnings;

  my $status = $ENV{'CLONE_SNAPSHOT_STATUS'};
  my @limits = $status =~
    /^Trx read view will not see trx with id >= (\d+), sees < (\d+)$/gm;
  die "Expected exactly one open read view\n$status" unless @limits == 2;
  die "Read view still contains active transaction ids\n$status"
    unless $limits[0] == $limits[1];
  print "Read view has no active transaction ids\n";
EOF
COMMIT;
