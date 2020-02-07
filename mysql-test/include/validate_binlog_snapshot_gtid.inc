--let $snapshot_gtid_executed = query_get_value(SHOW STATUS LIKE "Binlog_snapshot_gtid_executed", Value, 1)
--let $gtid_executed = `SELECT @@GLOBAL.GTID_EXECUTED`

--let _FF_SNAPSHOT_GTID_EXECUTED = $snapshot_gtid_executed
--let _FF_SNAPSHOT_GTID_EXECUTED_EXP = $expected_snapshot_gtid_executed
--let _FF_GTID_EXECUTED = $gtid_executed
--let _FF_GTID_EXECUTED_EXP = $expected_gtid_executed

--perl
  use strict;
  my $snapshot_gtid_executed = $ENV{'_FF_SNAPSHOT_GTID_EXECUTED'};
  my $snapshot_gtid_executed_exp = $ENV{'_FF_SNAPSHOT_GTID_EXECUTED_EXP'};
  my $gtid_executed = $ENV{'_FF_GTID_EXECUTED'};
  my $gtid_executed_exp = $ENV{'_FF_GTID_EXECUTED_EXP'};

  my @snapshot_gtid_executed_exp_parts = split(',', $snapshot_gtid_executed_exp);
  my @gtid_executed_exp_parts = split(',', $gtid_executed_exp);

  foreach my $val (@snapshot_gtid_executed_exp_parts) {
    if (index(lc($snapshot_gtid_executed), lc($val)) == -1) {
      print "Expected snapshot_gtid_executed value not found.\n";
      print "Expected: $val, snapshot_gtid_executed: $snapshot_gtid_executed\n";
    }
  }

  foreach my $val (@gtid_executed_exp_parts) {
    if (index(lc($gtid_executed), lc($val)) == -1) {
      print "Expected gtid_executed value not found.\n";
      print "Expected $val, gtid_executed: $gtid_executed\n";
    }
  }
EOF

