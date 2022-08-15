#
# Modern Linux distributions like EL8 and Debian 10 restrict minimum TLS
# protocol version to TLSv1.2, try to detect this and skip TLSv1/TLSv1.1 tests.
#
--perl
  use strict;
  use warnings;
  my $crypto_policy = '/etc/crypto-policies/back-ends/opensslcnf.config';
  my $openssl_conf = '/etc/ssl/openssl.cnf';
  my $policy_file = "";
  if (-r $crypto_policy) {
    $policy_file = $crypto_policy;
  } elsif (-r $openssl_conf) {
    $policy_file = $openssl_conf;
  }
  my $min_protocol_tlsv1 = 0;
  if ($policy_file ne "") {
    open(my $RH, "<", $policy_file) or die "Could not open $policy_file $!";
    while (<$RH>) {
      $_ =~ s/^\s+|\s+$//g;
      if (/^MinProtocol\s=\sTLSv1$/) {
        $min_protocol_tlsv1 = 1;
        last;
      }
    }
    close $RH;
  }
  if (`openssl version -a` =~ /OPENSSL_TLS_SECURITY_LEVEL=2/) {
    $min_protocol_tlsv1 = 0;
  }
  open (my $WH, ">", "$ENV{MYSQL_TMP_DIR}/crypto_policy.inc") or die "Could not open $ENV{MYSQL_TMP_DIR}/crypto_policy.inc: $!";
  print $WH "let \$min_protocol_tlsv1 = $min_protocol_tlsv1;\n";
  close $WH;
EOF
--source $MYSQL_TMP_DIR/crypto_policy.inc
--remove_file $MYSQL_TMP_DIR/crypto_policy.inc

if (!$min_protocol_tlsv1)
{
  --Skip Test requires TLS protocol <= 1.1, while OpenSSL policy sets MinProtocol TLSv1.2
}
