# Setup test to use keyring component

--echo # ----------------------------------------------------------------------
--echo # Setup

--let PLUGIN_DIR_OPT = $KEYRING_KMIP_COMPONENT_OPT

--let COMPONENT_LIBRARY = `SELECT SUBSTRING_INDEX('$KEYRING_KMIP_COMPONENT_LOAD', '=', -1)`
--let COMPONENT_DIR = $KEYRING_KMIP_COMPONENT_DIR
--let COMPONENT_NAME = `SELECT SUBSTRING_INDEX('$COMPONENT_LIBRARY', '.', 1)`
--let GLOBAL_MANIFEST_CONTENT = `SELECT CONCAT('{ \"components\": \"file://', '$COMPONENT_NAME', '\" }')`
# Create manifest file for mysqld binary
--source include/keyring_tests/helper/binary_create_customized_manifest.inc

# Create global keyring config
--let KEYRING_KMIP_PATH = `SELECT CONCAT( '$MYSQLTEST_VARDIR', '/keyring_kmip_customized')`
--echo # Creating custom global configuration file for keyring component: $COMPONENT_NAME
--perl
  use strict;
  my $vardir = $ENV{'MYSQLTEST_VARDIR'};
  my $config_file = $ENV{'COMPONENT_DIR'} . "/" . $ENV{'COMPONENT_NAME'} . ".cnf";
  open my $uuid_fh, '<', '/proc/sys/kernel/random/uuid' or die "Cannot open /proc/sys/kernel/random/uuid: $!";
  my $uuid = <$uuid_fh>;
  close $uuid_fh or die "Cannot close /proc/sys/kernel/random/uuid: $!";
  chomp $uuid;
  $uuid =~ s/-//g;
  $uuid = substr($uuid, 0, 16);
  my $keyring_path = $vardir . '/keyring_kmip_customized_' . $uuid;
  my $server_addr = $ENV{'KMIP_ADDR'} || '127.0.0.1';
  my $server_port = $ENV{'KMIP_PORT'} || '5696';
  my $client_ca = $ENV{'KMIP_CLIENT_CA'} || '/tmp/certs/mysql-client-cert.pem';
  my $client_key = $ENV{'KMIP_CLIENT_KEY'} || '/tmp/certs/mysql-client-key.pem';
  my $server_ca = $ENV{'KMIP_SERVER_CA'} || '/tmp/certs/vault-kmip-ca.pem';
  my $config_content = '{ "path": "' . $keyring_path . '", "server_addr": "' . $server_addr . '", "server_port": "' . $server_port . '", "client_ca": "' . $client_ca . '", "client_key": "' . $client_key . '", "server_ca": "' . $server_ca . '", "object_group": "test-object-group-customized-' . $uuid . '" }';
  open my $cfh, '>', $config_file or die "Cannot write $config_file: $!";
  print $cfh $config_content or die "Cannot write $config_file: $!";
  close $cfh or die "Cannot close $config_file: $!";
EOF

# Restart server with manifest file 
--source include/keyring_tests/helper/start_server_with_manifest.inc
--echo # ----------------------------------------------------------------------
