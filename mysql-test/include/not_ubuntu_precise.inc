#
# Skip the testcase if running under Ubuntu Precise (12.04)
#

--let check_result_file=$MYSQL_TMP_DIR/ubuntu_precise_check.tmp

--perl
use strict;
use warnings;
my $ubuntu_precise = 0;
if (open(my $FILE, '<', "/etc/lsb-release")) {
  $ubuntu_precise = scalar grep{/DISTRIB_RELEASE=12\.04/} <$FILE>;
  close $FILE;
}

my $result_fn = $ENV{'check_result_file'} or die "check_result_file not set";
open(my $RESULT, ">", $result_fn) or die "Could not open $result_fn for writing";
print $RESULT '--let $is_ubuntu_precise = ' . $ubuntu_precise . "\n";
close $RESULT;
EOF

--source $check_result_file
--remove_file $check_result_file

if ($is_ubuntu_precise)
{
   --skip Ubuntu Precise (12.04) is not supported
}
