# ==== Purpose ====
#
# Print sorted list of distinct connection types found in current
# audit filter log. Works for NEW, OLD and JSON formats.
#
# ==== Usage ====
#
# --source print_connection_types.inc

--let audit_filter_log_path = `SELECT @@global.datadir`
--let audit_filter_log_name = `SELECT @@global.audit_log_filter.file`

perl;
  my $full_log_name = "$ENV{audit_filter_log_path}$ENV{audit_filter_log_name}";

  open(my $fh, '<:encoding(UTF-8)', $full_log_name)
    or die "Could not open file '$full_log_name' $!";
  my $content = do { local $/; <$fh> };
  close($fh);

  my %types;
  $types{lc($1)} = 1
    while ($content =~ m{(?<!/)connection_type(?:>|=|"\s*:\s*)"?([^"<]*)["<]}gi);

  die "No connection types found in '$full_log_name'\n" if (!%types);

  print "Connection types: " . join(", ", sort keys %types) . "\n";
EOF
