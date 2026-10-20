# Writes the configuration file for the keyring_kmip component.
#
# Expects these variables, set with --let so that they are also exported:
#   KMIP_COMPONENT_CONFIG_FILE  path of the configuration file to write
#   KMIP_COMPONENT_NAME_PART    middle part of the keyring path and object
#                               group name, e.g. "customized"; may be empty
#   KMIP_COMPONENT_ID_LENGTH    number of random hex digits that make the
#                               keyring path and object group unique
#
# Connection settings come from the KMIP_ADDR, KMIP_PORT, KMIP_CLIENT_CA,
# KMIP_CLIENT_KEY and KMIP_SERVER_CA environment variables, falling back to
# the defaults described in the suite README.

--perl
  use strict;

  sub json_string {
    my ($value) = @_;
    $value =~ s/(["\\])/\\$1/g;
    $value =~ s/([\x00-\x1f])/sprintf('\\u%04x', ord($1))/ge;
    return '"' . $value . '"';
  }

  open my $uuid_fh, '<', '/proc/sys/kernel/random/uuid' or die "Cannot open /proc/sys/kernel/random/uuid: $!";
  my $uuid = <$uuid_fh>;
  close $uuid_fh or die "Cannot close /proc/sys/kernel/random/uuid: $!";
  chomp $uuid;
  $uuid =~ s/-//g;
  $uuid = substr($uuid, 0, $ENV{'KMIP_COMPONENT_ID_LENGTH'});

  my $name_part = $ENV{'KMIP_COMPONENT_NAME_PART'};
  my $path_part = $name_part eq '' ? '' : $name_part . '_';
  my $group_part = $name_part eq '' ? '' : $name_part . '-';

  my @fields = (
    [ 'path',         $ENV{'MYSQLTEST_VARDIR'} . '/keyring_kmip_' . $path_part . $uuid ],
    [ 'server_addr',  $ENV{'KMIP_ADDR'} || '127.0.0.1' ],
    [ 'server_port',  $ENV{'KMIP_PORT'} || '5696' ],
    [ 'client_ca',    $ENV{'KMIP_CLIENT_CA'} || '/tmp/certs/mysql-client-cert.pem' ],
    [ 'client_key',   $ENV{'KMIP_CLIENT_KEY'} || '/tmp/certs/mysql-client-key.pem' ],
    [ 'server_ca',    $ENV{'KMIP_SERVER_CA'} || '/tmp/certs/vault-kmip-ca.pem' ],
    [ 'object_group', 'test-object-group-' . $group_part . $uuid ],
  );
  my $config_content =
    '{ ' . join(', ', map { json_string($_->[0]) . ': ' . json_string($_->[1]) } @fields) . ' }';

  my $config_file = $ENV{'KMIP_COMPONENT_CONFIG_FILE'};
  open my $cfh, '>', $config_file or die "Cannot write $config_file: $!";
  print $cfh $config_content or die "Cannot write $config_file: $!";
  close $cfh or die "Cannot close $config_file: $!";
EOF
