#
# Collect information about archived log files
#

perl;
use Data::Dumper;
sub fn {
  my $filepath = shift;
  my ($dirname, $filename) = $filepath =~ m|^(.*[/\\])([^/\\]+?)$|;

  return $filename;
}

sub get_file_stat {
  my $filepath = shift;
  my $prefix = shift;
  my $filename = fn($filepath);

  my @array = stat($filepath);
  my $filemodifytime = $array[9];
  my @t = localtime $filemodifytime;
  my $modifytime = sprintf "%04u-%02u-%02u %02u:%02u:%02u",$t[5]+1900,$t[4]+1,$t[3],$t[2],$t[1],$t[0];
  my ($lsn) = $filename =~ m/ib_log_archive_[0]*([1-9][0-9]+)$/;

  my $f = {
    'path' => $filepath,
    'name' => $filename,
    't' => $filemodifytime,
    'modifytime' => $modifytime,
    'lsn' => $lsn
  };

  return $f;
}

sub print_file_stat {
  my %stat = %{$_[0]};
  my $prefix = $_[1];

  open F, ">>".$ENV{'MYSQLTEST_VARDIR'}.'/tmp/percona_archived_logs.tmp' or die "Cannot open file percona_archived_logs.tmp for write";
  printf F ("let \$${prefix}_log_file = %s;\n", $stat{'name'});
  printf F ("let \$${prefix}_log_file_modify = %s;\n", $stat{'modifytime'});
  printf F ("let \$${prefix}_log_file_lsn = %s;\n", $stat{'lsn'});
  close F;
}

@files = glob($ENV{'MYSQLD_ARCHIVEDIR'}.'/ib_log_archive_*');

my @files_stat = ();

foreach my $file (@files) {
  push(@files_stat, get_file_stat($file));
}

$count = scalar(@files_stat);
$mid_n = 1;
$t = $files_stat[$mid_n]{'t'};
while (($files_stat[$mid_n]{'t'} == $t) && ($mid_n < $count - 1)) {$mid_n++};
print_file_stat(@files_stat[0], 'first');
print_file_stat(@files_stat[-1], 'last');
print_file_stat(@files_stat[$mid_n - 1], 'mid');

open F, ">>".$ENV{'MYSQLTEST_VARDIR'}.'/tmp/percona_archived_logs.tmp' or die "Cannot open file percona_archived_logs.tmp for write";
printf F ("let \$archived_logs_count = %s;\n", $count);
printf F ("let \$archived_logs_mid = %s;\n", $mid_n);
close F;

EOF

--source $MYSQLTEST_VARDIR/tmp/percona_archived_logs.tmp
remove_file $MYSQLTEST_VARDIR/tmp/percona_archived_logs.tmp;

