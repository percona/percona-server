# ==== Purpose ====
#
# Leave only the log encryption options with the latest ID in the keyring
# file of component_keyring_file, store them under another ID and restart
# the server.
#
# ==== Usage ====
#
# --let AUDIT_OPTIONS_ID = audit_log-<YYYYMMDDTHHMMSS>-<sequence number>
# --source ../inc/keyring_file_set_audit_options_id.inc
#
# ==== Parameters ====
#
# $AUDIT_OPTIONS_ID
#    The new ID of the options
#
# $KEYRING_FILE_PATH, $PLUGIN_DIR_OPT
#    Set by suite/component_keyring_file/inc/setup_component.inc
#

--source include/shutdown_mysqld.inc

--perl
  use strict;
  use JSON::PP;
  my $path = $ENV{'KEYRING_FILE_PATH'};
  my $new_id = $ENV{'AUDIT_OPTIONS_ID'};
  open(my $fh, '<', $path) or die "open $path: $!";
  my $content = do { local $/; <$fh> };
  close($fh);
  my $keyring = JSON::PP->new->decode($content);
  my @elements;
  my $latest;
  my @latest_key;
  foreach my $element (@{$keyring->{'elements'}}) {
    if ($element->{'data_id'} !~ /^audit_log-(\d{8}T\d{6})-(\d+)$/) {
      push @elements, $element;
      next;
    }
    # The sequence numbers are compared as numbers of any length
    my @key = ($1, length($2), $2);
    if (!defined($latest) ||
        ($key[0] cmp $latest_key[0] || $key[1] <=> $latest_key[1] ||
         $key[2] cmp $latest_key[2]) > 0) {
      $latest = $element;
      @latest_key = @key;
    }
  }
  die "no log encryption options in $path" unless defined($latest);
  $latest->{'data_id'} = $new_id;
  push @elements, $latest;
  $keyring->{'elements'} = \@elements;
  open($fh, '>', $path) or die "open $path: $!";
  print $fh JSON::PP->new->encode($keyring);
  close($fh);
EOF

--echo # Re-starting mysql server with the log encryption options stored as $AUDIT_OPTIONS_ID only
--let $restart_parameters = restart: $PLUGIN_DIR_OPT
--source include/start_mysqld_no_echo.inc
