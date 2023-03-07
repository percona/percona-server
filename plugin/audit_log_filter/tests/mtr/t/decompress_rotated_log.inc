# ==== Purpose ====
#
# Finds rotated GZIP compressed (with '.gz' suffiks in its name) log if any
# and decompress it.
#
# ==== Usage ====
#
# --let $audit_filter_log_path = audit logs path
# --source decompress_rotated_log.inc
#
# Parameters:
#
# $audit_filter_log_path
#   The audit logs path.

--let audit_filter_log_path = $audit_filter_log_path

perl;
  my $dir_name = $ENV{'audit_filter_log_path'} or die "Empty audit_filter_log_path";
  opendir(my $dh, $dir_name) or die "Can't opendir $dir_name: $!";

  while (readdir $dh) {
    if ($_ =~ /audit_filter\.\d{8}T\d{6}\..*log\.gz/) {
      my $decompressed_name = $_ =~ s/\.gz//r;
      system("gunzip -c $dir_name$_ > $dir_name$decompressed_name") == 0 or die "system failed: $?";
      unlink("$dir_name$_") or die "Can't delete $_: $!";
    }
  }

  closedir $dh;
EOF
