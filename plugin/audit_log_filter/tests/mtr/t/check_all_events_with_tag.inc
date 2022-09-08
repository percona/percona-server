# ==== Purpose ====
#
# Check if all audit events in a audit filter log contain provided tag.
#
# ==== Usage ====
#
# --let $audit_filter_log_path = path to dir containing audit filter log
# --let $audit_filter_log_name = audit filter log file name
# --let $audit_filter_log_format = [xml|json] audit filter log format
# --let $search_tag = text to search in audit event record
# --source check_all_events_with_tag.inc
#
# Parameters:
#
# $audit_filter_log_path
#   Path to dir containing audit filter log.
# $audit_filter_log_name
#   Audit filter log file name.
# $audit_filter_log_format
#   Audit filter log format. One of: xml, json. Defaults to xml.
# $search_tag
#   Text to search in audit event record. For example, set it to
#   '<TABLE>table1</TABLE>' in order to make sure each record in the
#   audit log contains table name 'table1'.
#   Add '!' symbol in front of the string to inverse check.
#   For example, '!<TABLE>table1</TABLE>' will make sure not any of audit
#   records contains table name 'table1'.

--let audit_filter_log_path = $audit_filter_log_path
--let audit_filter_log_name = $audit_filter_log_name
--let audit_filter_log_format = $audit_filter_log_format
--let search_tag = $search_tag

perl;
  die "Log file path is not set" if (!$ENV{'audit_filter_log_path'});
  die "Log file name is not set" if (!$ENV{'audit_filter_log_name'});
  die "Search tag is not set" if (!$ENV{'search_tag'});

  my $full_log_name = "$ENV{audit_filter_log_path}$ENV{audit_filter_log_name}";
  my $search_tag = $ENV{'search_tag'};
  my $log_format = $ENV{'audit_filter_log_format'} || "xml";
  my $search_neg = 0;

  if ($search_tag =~ /^!/) {
    $search_neg = 1;
    $search_tag =~ s/^.//s;
  }

  open(my $fh, '<:encoding(UTF-8)', $full_log_name)
    or die "Could not open file '$full_log_name' $!";

  while (my $event = read_event($fh, $log_format)) {
    if ($search_neg) {
      if ($event =~ /$search_tag/) {
        die "Tag '$search_tag' is not expected in event \n'$event'\n";
      }
    }
    else {
      if ($event !~ /$search_tag/) {
        die "Missing '$search_tag' in event \n'$event'\n";
      }
    }
  }

  close($fh);

  print $search_neg ? "No tag" : "Tag";
  print " $search_tag Ok\n";

  sub read_event {
    my ($fh, $format) = @_;
    my $result = "";

    if ($format eq 'xml') {
      my $event_start_found = 0;
      my $event_end_found = 0;

      while (my $row = <$fh>) {
        $event_start_found = 1 if ($row =~ /<AUDIT_RECORD>/);
        $event_end_found = 1 if ($row =~ /<\/AUDIT_RECORD>/);

        if ($event_start_found && !$event_end_found) {
          $result .= $row;
        }

        last if ($event_start_found && $event_end_found);
      }
    }
    elsif ($format eq 'json') {
      my $num_opening_tags = 0;
      my $num_closing_tags = 0;

      while (my $row = <$fh>) {
        next if ($row =~ /\{.*\}/);

        $num_opening_tags++ if ($row =~ /\{/);
        $num_closing_tags++ if ($row =~ /\}/);

        if ($num_opening_tags > 0) {
          $result .= $row;
        }

        last if ($num_opening_tags > 0 && $num_closing_tags > 0 &&
                 $num_opening_tags == $num_closing_tags);
      }
    }
    else {
      die "Unknown log format '$format'\n";
    }

    $result =~ s/\R//g;

    return $result;
  }

EOF
