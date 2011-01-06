#!/usr/bin/perl

# vim:sw=2:ai

# test for bugfix: commit/c88efe637f6a184b55d2bd8d060bda3e556572d8
# (some trailing bytes were dropped for varlen or nullable key fields)

BEGIN {
	push @INC, "../common/";
};

use strict;
use warnings;
use hstest;

my $dbh = hstest::init_testdb();
my $table = 'hstesttbl';
my $tablesize = 50;
$dbh->do(
  "create table $table (" .
  "k int primary key, " .
  "v1 varchar(30), " .
  "v2 varchar(30), " .
  "index i1(v1), index i2(v2, v1)) " .
  "engine = myisam default charset = binary");
srand(999);

my %valmap = ();

my $sth = $dbh->prepare("insert into $table values (?,?,?)");
for (my $i = 0; $i < $tablesize; ++$i) {
  my $k = $i;
  my ($s1, $s2) = ("", "");
  for (my $j = 0; $j < $i; ++$j) {
    $s1 .= chr(48 + $j % 10);
    $s2 .= chr(65 + $j % 10);
  }
  my $v1 = $s1;
  my $v2 = $s2;
  $sth->execute($k, $v1, $v2);
  $valmap{$k} = [ $v1, $v2 ];
}

dump_table();

my $hs = hstest::get_hs_connection(undef, 9999);
my $dbname = $hstest::conf{dbname};
$hs->open_index(1, $dbname, $table, '', 'k,v1,v2');
$hs->open_index(2, $dbname, $table, 'i1', 'k,v1,v2');
$hs->open_index(3, $dbname, $table, 'i2', 'k,v1,v2');

for (my $i = 0; $i <= 30; ++$i) {
  my ($v1, $v2) = @{$valmap{$i}};
  my ($rk, $rv1, $rv2);
  my $r = $hs->execute_single(1, '=', [ $i ], 1, 0);
  shift(@$r);
  ($rk, $rv1, $rv2) = @$r;
  print "PK $rk $rv1 $rv2\n";
  my $r = $hs->execute_single(2, '=', [ $v1 ], 1, 0);
  shift(@$r);
  ($rk, $rv1, $rv2) = @$r;
  print "I1 $rk $rv1 $rv2\n";
  my $r = $hs->execute_single(3, '=', [ $v2, $v1 ], 1, 0);
  shift(@$r);
  ($rk, $rv1, $rv2) = @$r;
  print "I2 $rk $rv1 $rv2\n";
}

=pod
my %valmap = ();

print "HSINSERT\n";
my $hs = hstest::get_hs_connection(undef, 9999);
my $dbname = $hstest::conf{dbname};
$hs->open_index(1, $dbname, $table, '', 'k,v1,v2');
# inserts with auto_increment
for (my $i = 0; $i < $tablesize; ++$i) {
  my $k = 0;
  my $v1 = "v1hs_" . $i;
  my $v2 = "v2hs_" . $i;
  my $r = $hs->execute_insert(1, [ $k, $v1, $v2 ]);
  my $err = $r->[0];
  if ($err != 0) {
    my $err_str = $r->[1];
    print "$err $err_str\n";
  }
}
# make sure that it works even when inserts are pipelined. these requests
# are possibly executed in a single transaction.
for (my $i = 0; $i < $tablesize; ++$i) {
  my $k = 0;
  my $v1 = "v1hs3_" . $i;
  my $v2 = "v2hs3_" . $i;
  my $r = $hs->execute_multi([
  	[ 1, '+', [$k, $v1, $v2] ],
  	[ 1, '+', [$k, $v1, $v2] ],
  	[ 1, '+', [$k, $v1, $v2] ],
	]);
  for (my $i = 0; $i < 3; ++$i) {
    my $err = $r->[$i]->[0];
    if ($err != 0) {
      my $err_str = $r->[1];
      print "$err $err_str\n";
    }
  }
}
undef $hs;
=cut

sub dump_table {
  print "DUMP_TABLE\n";
  my $aref = $dbh->selectall_arrayref("select k,v1,v2 from $table order by k");
  for my $row (@$aref) {
    my ($k, $v1, $v2) = @$row;
    $v1 = "[null]" if !defined($v1);
    $v2 = "[null]" if !defined($v2);
    print "$k $v1 $v2\n";
    # print "MISMATCH\n" if ($valmap{$k} ne $v);
  }
}

