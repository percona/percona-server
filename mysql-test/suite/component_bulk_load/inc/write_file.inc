--let csv_file_path = $csv_file_path
--let csv_file_data = $csv_file_data

--perl
  use strict;
  use warnings;
  my $file_name = $ENV{csv_file_path} or die "Missing csv_file_path";
  my $file_data = $ENV{csv_file_data} or die "Empty csv_file_data";
  open(my $fh, '>>', $file_name) || die "Cannot open $file_name: $!";
  eval 'print $fh "' . $file_data . '";';
  close($fh);
EOF
