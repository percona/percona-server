# ==== Purpose ====
#
# Finds rotated encrypted (with '%Y%m%dT%H%M%-1.enc' like suffiks in its name)
# log if any and decrypts it.
#
# ==== Usage ====
#
# --let $audit_filter_log_path = audit logs path
# --let $encryption_password = password used to encrypt log file
# --source decrypt_rotated_log.inc
#
# Parameters:
#
# $audit_filter_log_path
#   The audit logs path.
# $encryption_password
#   Password used to encrypt log file.

--let audit_filter_log_path = $audit_filter_log_path
--let encryption_password = $encryption_password

perl;
  my $dir_name = $ENV{'audit_filter_log_path'} or die "Empty audit_filter_log_path";
  my $password = $ENV{'encryption_password'} or die "Empty encryption_password";

  opendir(my $dh, $dir_name) or die "Can't opendir $dir_name: $!";

  while (readdir $dh) {
    if ($_ =~ /audit_filter\.\d{8}T\d{6}\..*log.*\.\d{8}T\d{6}\-\d\.enc/) {
      my $decrypted_name = $_ =~ s/\.\d{8}T\d{6}\-\d\.enc//r;
      system("openssl enc -d -aes-256-cbc -pass pass:$password -md sha256 -nosalt -iter 1 -in $dir_name$_ -out $dir_name$decrypted_name") == 0 or die "system failed: $?";
      unlink("$dir_name$_") or die "Can't delete $_: $!";
    }
  }

  closedir $dh;
EOF
