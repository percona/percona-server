
perl;
@files = glob($ENV{'MYSQLD_ARCHIVEDIR'}.'/ib_log_archive_*');
my $filepath, $filename, $dirname;
foreach my $file (@files) {
  ($dirname, $filename) = $file =~ m|^(.*[/\\])([^/\\]+?)$|;
  $filepath = $file;
}

my @array = stat($filepath);
my $filemodifytime = $array[9];
my @t = localtime $filemodifytime;
my $modifytime = sprintf "%04u-%02u-%02u %02u:%02u:%02u",$t[5]+1900,$t[4]+1,$t[3],$t[2],$t[1],$t[0];
my $lsn = $filename =~ m/ib_log_archive_0*([1-9][0-9]+)$/;
open F, ">".$ENV{'MYSQLTEST_VARDIR'}.'/tmp/percona_archived_logs.tmp' or die "Cannot open file percona_archived_logs.tmp for write";
printf F ("let \$last_log_file = %s;\n", $filename);
printf F ("let \$last_log_file_modify = %s;\n", $modifytime);
printf F ("let \$last_log_file_lsn = %s;\n", $lsn);
close F;
EOF

--source $MYSQLTEST_VARDIR/tmp/percona_archived_logs.tmp
# remove_file $MYSQLTEST_VARDIR/tmp/percona_archived_logs.tmp;

