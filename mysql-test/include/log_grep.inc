--echo [log_grep.inc] file: $log_file pattern: $grep_pattern
perl;
  $log_file=           $ENV{'log_file'};
  $log_file_full_path= $ENV{'log_file_full_path'};
  $log_slow_rate_test= $ENV{'log_slow_rate_test'};
  open(FILE, "$log_file_full_path") 
    or die("Cannot open file $log_file_full_path: $!\n");

  if ($log_slow_rate_test) {
    $one= 0;
    $two= 0;
    $three= 0;
    while(<FILE>) {
      $one++   if(/'connection_one'/);
      $two++   if(/'connection_two'/);
      $three++ if(/'connection_three'/);
    }
    $sum= $one + $two + $three;
    $zero= 0;
    if ($one == 0) {
      $zero++;
    }
    if ($two == 0) {
      $zero++;
    }
    if ($three == 0) {
      $zero++;
    }
    print "[log_grep.inc] sum:  $sum\n";
    print "[log_grep.inc] zero: $zero\n";
  }
  else {
    $grep_pattern=       $ENV{'grep_pattern'};
    $lines= 0;
    while(<FILE>) {
      $lines++ if (/$grep_pattern/);
    }
    print "[log_grep.inc] lines:   $lines\n";
  }
  close(FILE);
EOF
