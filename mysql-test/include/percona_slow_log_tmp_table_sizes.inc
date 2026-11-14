# Extract the "# Tmp_tables: N  Tmp_disk_tables: N  Tmp_table_sizes: N
# Tmp_disk_table_sizes: N" line written by the query_plan slow log verbosity
# level, and record it in a report file for manual inspection.
#
# The *last* matching line in the log is used: include/log_start.inc turns the
# slow query log on with a SET statement that is itself logged (with all
# counters at zero), so the line of interest is the one for the query that ran
# after it.
#
# Arguments (all environment-style lets, so that perl can see them):
# - log_file_full_path: set by include/log_start.inc
# - tts_case:           label used in the report file
# - tts_expected:       free-form note on what the value should look like
#
# Sets:
# - $tmp_tables, $tmp_disk_tables, $tmp_table_sizes, $tmp_disk_table_sizes:
#   values from the log, or -1 each if no matching line was found at all
#
# Appends one tab-separated line per case to
# $MYSQL_TMP_DIR/tmp_table_sizes_report.tsv

--let TTS_PERL_RESULT = $MYSQL_TMP_DIR/tmp_table_sizes_result.test
# The caller is responsible for removing this again before the test ends:
# include/check-testcase.test lists $MYSQL_TMP_DIR, so a file left behind is
# reported as leaked state.
--let TTS_REPORT = $MYSQL_TMP_DIR/tmp_table_sizes_report.tsv

perl;
  my $out_path = $ENV{'TTS_PERL_RESULT'};
  my $report_path = $ENV{'TTS_REPORT'};
  my $log_path = $ENV{'log_file_full_path'};

  open(my $log, "<", $log_path)
    or die("Cannot open file $log_path: $!\n");
  my ($used, $disk, $size, $disk_size) = (-1, -1, -1, -1);
  while (<$log>) {
    if (/^# Tmp_tables: (\d+)  Tmp_disk_tables: (\d+)  Tmp_table_sizes: (\d+)  Tmp_disk_table_sizes: (\d+)$/) {
      ($used, $disk, $size, $disk_size) = ($1, $2, $3, $4);
    }
  }
  close($log);

  open(my $out, ">", $out_path) or die("Cannot create file $out_path: $!\n");
  print $out "--let \$tmp_tables= $used\n";
  print $out "--let \$tmp_disk_tables= $disk\n";
  print $out "--let \$tmp_table_sizes= $size\n";
  print $out "--let \$tmp_disk_table_sizes= $disk_size\n";
  close($out);

  my $is_new = !-e $report_path;
  open(my $report, ">>", $report_path)
    or die("Cannot append to $report_path: $!\n");
  print $report "case\tTmp_tables\tTmp_disk_tables\tTmp_table_sizes\tTmp_disk_table_sizes\texpected\n"
    if $is_new;
  print $report "$ENV{'tts_case'}\t$used\t$disk\t$size\t$disk_size\t$ENV{'tts_expected'}\n";
  close($report);
EOF

--source $TTS_PERL_RESULT
--remove_file $TTS_PERL_RESULT
