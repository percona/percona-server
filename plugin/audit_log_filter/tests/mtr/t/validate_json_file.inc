# ==== Purpose ====
#
# Check if JSON file formatting is valid.
#
# ==== Usage ====
#
# --let $json_file_path = JSON file path
# --let $json_file_name = JSON file name
# --source validate_json_file.inc
#
# Parameters:
#
# $json_file_path
#   The JSON file path.
# $json_file_name
#   The JSON file name.

--let json_file_path=$json_file_path
--let json_file_name=$json_file_name

perl;
  use JSON;

  my $filename = $ENV{'json_file_path'} . $ENV{'json_file_name'};

  open(my $fh, "<:encoding(UTF-8)", $filename) or die("Can't open '$filename': $!\n");
  local $/;
  my $json_text = <$fh>;
  close($fh);

  my $json = JSON->new();
  my $data = $json->decode($json_text);

  print "JSON audit filtering log is valid\n";
EOF
