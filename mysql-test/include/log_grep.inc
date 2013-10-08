--disable_query_log
--echo [log_grep.inc] file: $log_file pattern: $grep_pattern
perl;
  $log_file=           $ENV{'log_file'};
  $log_file_full_path= $ENV{'log_file_full_path'};
  $grep_pattern=       $ENV{'grep_pattern'};

  open(FILE, "$log_file_full_path") 
    or die("Cannot open file $log_file_full_path: $!\n");

  $lines = 0;
  while(<FILE>) {
    $lines++ if (/$grep_pattern/);
  }
  close(FILE);
  print "[log_grep.inc] lines:   $lines\n";
EOF
