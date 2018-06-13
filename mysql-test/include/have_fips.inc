# This inc file detects FIPS only on centOS/RHEL

--source include/not_windows.inc

--let OUTFILE= $MYSQLTEST_VARDIR/tmp/fips.out

perl;
use strict;
# This file contents should only be either 0 or 1
my $fips_file = "/proc/sys/crypto/fips_enabled";
my $fips = 0;
if ((-e $fips_file) && (-r $fips_file))
{
  local $/=undef;
  open FILE, $fips_file or die "Couldn't open file: $!";
  $fips = <FILE>;
  close FILE;
}

open(FILE, ">$ENV{OUTFILE}") or die;
print FILE "let \$fips= $fips;\n";
close(FILE);
EOF

--source $OUTFILE
--remove_file $OUTFILE

if (!$fips) {
  --skip Test requires FIPS enabled environment like centOS/RHEL
}
