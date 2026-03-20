# ==== Purpose ====
#
# After generate_audit_events.inc, read the current JSON audit log and assert
# which event classes appear. Uses the same path variables as this suite's
# filter tests (audit_filter_log_path, audit_filter_log_name).
#
# ==== Prerequisites ====
#
# --let audit_filter_log_path = ...
# --let audit_filter_log_name = ...
#
# ==== Parameters (comma-separated class names, no spaces) ====
#
# --let audit_filter_expect_required = general,connection,table_access
#   Each listed class must appear at least once (optional: leave empty).
#   Use names without a leading '$' so mysqltest exports them to the perl
#   process environment.
#
# --let audit_filter_expect_absent = table_access,connection
#   Each listed class must not appear (optional: leave empty).
#
# --let audit_filter_expect_for_suffix = empty filter
#   Used in die messages for missing required classes: "... for <suffix>".
#
# --let audit_filter_expect_ok = OK: empty filter
#   Printed on success (one line, no newline in the value).
#
# ==== Usage ====
#
# --source audit_filter_json_log_expect_classes.inc

perl;
  use strict;
  use warnings;
  use JSON::PP ();

  sub classes_from_env {
    my ($k) = @_;
    my $v = $ENV{$k} // '';
    return grep { length $_ } split /,/, $v;
  }

  my $path = $ENV{audit_filter_log_path} . $ENV{audit_filter_log_name};
  open(my $fh, '<:encoding(UTF-8)', $path) or die "open $path: $!";
  local $/;
  my $t = <$fh>;
  close($fh);
  $t =~ s/\s+$//;
  unless ($t =~ /\]\s*$/) {
    $t =~ s/,\s*$//;
    $t .= "\n]\n";
  }
  my $events = JSON::PP::decode_json($t);
  my %seen;
  for my $e (@{$events}) {
    next if ref($e) ne 'HASH' || !exists $e->{class};
    $seen{$e->{class}} = 1;
  }

  my $suffix = $ENV{audit_filter_expect_for_suffix} // '';
  for my $need (classes_from_env('audit_filter_expect_required')) {
    die "expected class '$need' for $suffix\n" if !$seen{$need};
  }
  for my $bad (classes_from_env('audit_filter_expect_absent')) {
    die "class '$bad' must not be logged\n" if $seen{$bad};
  }
  my $ok = $ENV{audit_filter_expect_ok};
  die "audit_filter_expect_ok not set\n" if !defined $ok || $ok eq '';
  print "$ok\n";
EOF
