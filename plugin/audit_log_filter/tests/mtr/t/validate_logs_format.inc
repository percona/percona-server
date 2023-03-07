# ==== Purpose ====
#
# Check if audit logs formatting is valid.
#
# ==== Usage ====
#
# --let $audit_filter_log_path = audit logs path
# --source validate_logs_format.inc
#
# Parameters:
#
# $audit_filter_log_path
#   The audit logs path.
# $audit_filter_log_name
#   The audit log file name, optional.

--let audit_filter_log_path = $audit_filter_log_path
--let audit_filter_log_name = $audit_filter_log_name

perl;
  use JSON;
  use XML::Parser;

  my $file_name = $ENV{'audit_filter_log_name'};
  my $dir_name = $ENV{'audit_filter_log_path'};

  if ($file_name) {
    if ($file_name =~ m/\.xml\./) {
      validate_xml("$dir_name$file_name");
    }
    else {
      validate_json("$dir_name$file_name");
    }
    print "File format Ok\n";
  }
  else {
    my $logs_counter = 0;

    opendir(my $dh, $dir_name) or die "Can't opendir $dir_name: $!";

    while (readdir $dh) {
      if ($_ =~ /audit_filter\.\d{8}T\d{6}\.(xml|json)\.log/) {
        $logs_counter += 1;
        if ($_ =~ m/\.xml\./) {
          validate_xml("$dir_name$_");
        }
        else {
          validate_json("$dir_name$_");
        }
      }
    }

    closedir $dh;

    if ($logs_counter > 0) {
      print "Rotated files Ok\n";
    } else {
      print "Rotation doesn't work, logs_counter: $logs_counter\n";
    }
  }

  sub validate_xml {
    my ($filename) = @_;
    my $p = new XML::Parser;
    $p->parsefile($filename);
  };

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
