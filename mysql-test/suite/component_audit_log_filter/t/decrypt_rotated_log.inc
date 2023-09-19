# ==== Purpose ====
#
# Finds rotated encrypted (with '%Y%m%dT%H%M%-1.enc' like suffix in its name)
# log if any and decrypts it.
#
# ==== Usage ====
#
# --let $audit_filter_log_path = audit logs path
# --let $encryption_password = password used to encrypt log file
# --let $encryption_iterations = number of iterations used by key derivation function
# --source decrypt_rotated_log.inc
#
# Parameters:
#
# $audit_filter_log_path
#   The audit logs path.
# $encryption_password
#   Password used to encrypt log file.
# $encryption_iterations
#   Number of iterations used by key derivation function.

--let audit_filter_log_path = $audit_filter_log_path
--let encryption_password = $encryption_password
--let encryption_iterations = $encryption_iterations

perl;
  my $help_text = qx(openssl enc -help 2>&1);
  my $openssl_iter_support = 1;

  if ($help_text !~ m/\-iter/) {
    $openssl_iter_support = 0;
  }

  open(OUTPUT, ">$ENV{MYSQLTEST_VARDIR}/tmp/audit_decrypt_openssl_status.inc") or die "Cannot open tmp file";
  print OUTPUT "let \$openssl_iter_support = $openssl_iter_support;\n";
  close(OUTPUT);

  if (!$openssl_iter_support) { exit; }

  my $dir_name = $ENV{'audit_filter_log_path'} or die "Empty audit_filter_log_path";
  my $password = $ENV{'encryption_password'} or die "Empty encryption_password";
  my $iterations = $ENV{'encryption_iterations'} or die "Empty encryption_iterations";

  opendir(my $dh, $dir_name) or die "Can't opendir $dir_name: $!";

  while (readdir $dh) {
    if ($_ =~ /audit_filter\.\d{8}T\d{6}\..*log.*\.\d{8}T\d{6}\-\d\.enc/) {
      my $decrypted_name = $_ =~ s/\.\d{8}T\d{6}\-\d\.enc//r;
      system("openssl enc -d -aes-256-cbc -pass pass:$password -md sha256 -iter $iterations -in $dir_name$_ -out $dir_name$decrypted_name") == 0 or die "system failed: $?";
      unlink("$dir_name$_") or die "Can't delete $_: $!";
    }
  }

  closedir $dh;
EOF

--source  $MYSQLTEST_VARDIR/tmp/audit_decrypt_openssl_status.inc
--remove_file $MYSQLTEST_VARDIR/tmp/audit_decrypt_openssl_status.inc

if (!$openssl_iter_support) {
  --skip 'openssl enc -iter' not supported
}
