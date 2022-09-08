# ==== Purpose ====
#
# Check if JSON file formatting is valid.
#
# ==== Usage ====
#
# --let $audit_filter_log_path = JSON file path
# --source validate_json_file.inc
#
# Parameters:
#
# $audit_filter_log_path
#   The JSON file path.
# $audit_filter_log_name
#   The JSON file name, optional.

--let audit_filter_log_path = $audit_filter_log_path
--let audit_filter_log_name = $audit_filter_log_name

perl;
  use JSON;

  if ($ENV{'audit_filter_log_name'}) {
    validate_json("$ENV{'audit_filter_log_path'}$ENV{'audit_filter_log_name'}");
    print "File format Ok\n";
  }
  else {
    opendir(my $dh, $ENV{'audit_filter_log_path'}) or die
        "Can't opendir $ENV{'audit_filter_log_path'}: $!";

    while (readdir $dh) {
      if ($_ =~ /test_audit_filter\.\d{8}T\d{6}\.log/) {
        $logs_counter += 1;
        validate_json("$ENV{'audit_filter_log_path'}$_");
      }
    }

    closedir $dh;

    if ($logs_counter > 0) {
      print "Rotated files Ok\n";
    } else {
      print "Rotation doesn't work, logs_counter: $logs_counter\n";
    }
  }

  sub validate_json {
    my ($filename) = @_;

    open(my $fh, "<:encoding(UTF-8)", $filename) or
        die("Can't open '$filename': $!\n");
    local $ / ;
    my $json_text = <$fh>;
    close($fh);

    my $json = JSON->new();
    my $data = $json->decode($json_text);
  };
EOF
