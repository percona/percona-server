# ==== Purpose ====
#
# Add log encryption options to the keyring file of component_keyring_file.
# The new options are copies of the first log encryption options in the
# file, each with its ID as the password. The server has to be down or it
# has to reload the keyring afterwards.
#
# ==== Usage ====
#
# --let AUDIT_OPTIONS_IDS = <ID> [<ID> ...]
# --source ../inc/keyring_file_add_audit_options.inc
#
# ==== Parameters ====
#
# $AUDIT_OPTIONS_IDS
#    The IDs of the new options, separated by spaces, in the form of
#    audit_log-<YYYYMMDDTHHMMSS>-<sequence number>
#
# $KEYRING_FILE_PATH
#    Set by suite/component_keyring_file/inc/setup_component.inc
#

--perl
  use strict;
  use JSON::PP;
  my $path = $ENV{'KEYRING_FILE_PATH'};
  open(my $fh, '<', $path) or die "open $path: $!";
  my $content = do { local $/; <$fh> };
  close($fh);
  my $keyring = JSON::PP->new->decode($content);
  my ($template) = grep { $_->{'data_id'} =~ /^audit_log-/ }
                        @{$keyring->{'elements'}};
  die "no log encryption options in $path" unless defined($template);
  my $options = JSON::PP->new->decode(pack('H*', $template->{'data'}));
  foreach my $id (split(' ', $ENV{'AUDIT_OPTIONS_IDS'})) {
    my %element = %$template;
    $options->{'password'} = $id;
    $element{'data_id'} = $id;
    $element{'data'} = uc(unpack('H*', JSON::PP->new->encode($options)));
    push @{$keyring->{'elements'}}, \%element;
  }
  open($fh, '>', $path) or die "open $path: $!";
  print $fh JSON::PP->new->encode($keyring);
  close($fh);
EOF
