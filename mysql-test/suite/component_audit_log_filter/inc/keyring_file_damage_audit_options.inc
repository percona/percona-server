# ==== Purpose ====
#
# Damage the log encryption options stored in the keyring file of
# component_keyring_file and restart the server with
# audit_log_filter.encryption=NONE, the log cannot be encrypted with such
# options.
#
# ==== Usage ====
#
# --let AUDIT_OPTIONS_DAMAGE = iterations | salt | json
# --source ../inc/keyring_file_damage_audit_options.inc
#
# ==== Parameters ====
#
# $AUDIT_OPTIONS_DAMAGE
#    iterations: the number of iterations of the options is zero, such
#                options are read but they are not valid
#    salt: the salt of the options is not a hexadecimal string
#    json: the options are not a JSON document
#
# $KEYRING_FILE_PATH, $PLUGIN_DIR_OPT
#    Set by suite/component_keyring_file/inc/setup_component.inc
#

--source include/shutdown_mysqld.inc

--perl
  use strict;
  use JSON::PP;
  my $path = $ENV{'KEYRING_FILE_PATH'};
  my $damage = $ENV{'AUDIT_OPTIONS_DAMAGE'};
  open(my $fh, '<', $path) or die "open $path: $!";
  my $content = do { local $/; <$fh> };
  close($fh);
  my $keyring = JSON::PP->new->decode($content);
  my $damaged = 0;
  foreach my $element (@{$keyring->{'elements'}}) {
    next unless $element->{'data_id'} =~ /^audit_log-/;
    my $options = pack('H*', $element->{'data'});
    if ($damage eq 'iterations') {
      $options =~ s/"iterations":\d+/"iterations":0/
          or die "no iterations in the options";
    } elsif ($damage eq 'salt') {
      $options =~ s/"salt":"[^"]*"/"salt":"zz"/ or die "no salt in the options";
    } else {
      $options = 'not-json';
    }
    $element->{'data'} = uc(unpack('H*', $options));
    $damaged++;
  }
  die "no log encryption options in $path" unless $damaged;
  open($fh, '>', $path) or die "open $path: $!";
  print $fh JSON::PP->new->encode($keyring);
  close($fh);
EOF

--echo # Re-starting mysql server with damaged log encryption options ($AUDIT_OPTIONS_DAMAGE)
--let $restart_parameters = restart: $PLUGIN_DIR_OPT --audit_log_filter.encryption=NONE
--source include/start_mysqld_no_echo.inc
