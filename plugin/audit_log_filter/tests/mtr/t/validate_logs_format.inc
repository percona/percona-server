# ==== Purpose ====
#
# Check if audit logs formatting is valid.
#
# ==== Usage ====
#
# --let $audit_filter_log_path = audit logs path
# --let $audit_filter_log_name = audit logs base name
# --let $audit_filter_log_format = audit logs format
# --let $audit_filter_log_check_one = log file name to check
# --source validate_logs_format.inc
#
# Parameters:
#
# $audit_filter_log_path
#   The audit logs path.
# $audit_filter_log_name
#   The audit log file base name.
# $audit_filter_log_format
#   The audit logs format, 'xml' or 'json'.
# $audit_filter_log_check_one
#   Check only this one log file, if provided.

--let audit_filter_log_path = $audit_filter_log_path
--let audit_filter_log_name = $audit_filter_log_name
--let audit_filter_log_format = $audit_filter_log_format
--let audit_filter_log_check_one = $audit_filter_log_check_one

perl;
  use strict;
  use warnings;
  use JSON;
  use XML::Parser;

  my $dir_name = $ENV{'audit_filter_log_path'} or die "Missing audit_filter_log_path\n";
  my $base_file_name = $ENV{'audit_filter_log_name'} =~ s/\..*$//r;
  my $format_name = $ENV{'audit_filter_log_format'} or die "Missing audit_filter_log_format\n";
  my $check_one_log_name = $ENV{'audit_filter_log_check_one'};

  die "Bad log format: $format_name\n" if ($format_name !~ m/^xml$|^json$/);

  if ($check_one_log_name) {
    if ($format_name eq "xml") {
      validate_xml("$dir_name$check_one_log_name");
    }
    else {
      validate_json("$dir_name$check_one_log_name");
    }
    print "File format Ok\n";
  }
  else {
    die "Missing audit_filter_log_name\n" if (!$base_file_name);

    my $logs_counter = 0;

    opendir(my $dh, $dir_name) or die "Can't opendir $dir_name: $!";

    while (readdir $dh) {
      if ($_ =~ /$base_file_name\.\d{8}T\d{6}/) {
        $logs_counter += 1;
        if ($format_name eq "xml") {
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
