# ==== Purpose ====
#
# Check if XML file formatting is valid.
#
# ==== Usage ====
#
# --let $audit_filter_log_path = XML file path
# --source validate_xml_file.inc
#
# Parameters:
#
# $audit_filter_log_path
#   The XML file path.
# $audit_filter_log_name
#   The XML file name, optional.

--let audit_filter_log_path = $audit_filter_log_path
--let audit_filter_log_name = $audit_filter_log_name

perl;
  eval "use XML::Parser; 1" or exit 0;
  my $p = new XML::Parser;

  if ($ENV{'audit_filter_log_name'}) {
    $p->parsefile("$ENV{'audit_filter_log_path'}$ENV{'audit_filter_log_name'}");
    print "File format Ok\n";
  }
  else {
    my $logs_counter = 0;

    opendir(my $dh, $ENV{'audit_filter_log_path'}) or die
        "Can't opendir $ENV{'audit_filter_log_path'}: $!";

    while (readdir $dh) {
      if ($_ =~ /test_audit_filter\.\d{8}T\d{6}\.log/) {
        $logs_counter += 1;
        $p->parsefile("$ENV{'audit_filter_log_path'}$_");
      }
    }

    closedir $dh;

    if ($logs_counter > 0) {
      print "Rotated files Ok\n";
    } else {
      print "Rotation doesn't work, logs_counter: $logs_counter\n";
    }
  }
EOF
