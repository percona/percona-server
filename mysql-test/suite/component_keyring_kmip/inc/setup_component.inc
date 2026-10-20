# Setup test to use keyring component

# Used environment variables:
# KMIP_PORT
# KMIP_ADDR
# KMIP_CLIENT_CA
# KMIP_CLIENT_KEY
# KMIP_SERVER_CA

--echo # ----------------------------------------------------------------------
--echo # Setup

# Ensure the standard binary manifest (read_local_manifest: true) is in place.
# A previous customized-setup test may have replaced or removed it; restoring
# it here guarantees that the instance manifest is always read at server start.
--perl
  use strict;
  use File::Basename;
  my $content = '{ "read_local_manifest": true }';
  my $manifest_file_ext = ".my";
  my ($exename, $path, $suffix) = fileparse($ENV{'MYSQLD'}, qr/\.[^.]*/);
  my $manifest_file_path = $path.$exename.$manifest_file_ext;
  open(my $mh, "> $manifest_file_path") or die "Cannot write $manifest_file_path: $!";
  print $mh $content or die;
  close($mh) or die;
EOF

--let PLUGIN_DIR_OPT = $KEYRING_KMIP_COMPONENT_OPT

# Data directory location
--let CURRENT_DATADIR = `SELECT @@datadir`

--let COMPONENT_LIBRARY = `SELECT SUBSTRING_INDEX('$KEYRING_KMIP_COMPONENT_LOAD', '=', -1)`
--let COMPONENT_DIR = $KEYRING_KMIP_COMPONENT_DIR
--let COMPONENT_NAME = `SELECT SUBSTRING_INDEX('$COMPONENT_LIBRARY', '.', 1)`
--source include/keyring_tests/helper/global_keyring_create_config.inc

# Create local keyring config
--let KEYRING_KMIP_PATH = `SELECT CONCAT( '$MYSQLTEST_VARDIR', '/keyring_kmip')`
--echo # Creating local configuration file for keyring component: $COMPONENT_NAME
--let KMIP_COMPONENT_CONFIG_FILE = $CURRENT_DATADIR/$COMPONENT_NAME.cnf
--let KMIP_COMPONENT_NAME_PART =
--let KMIP_COMPONENT_ID_LENGTH = 24
--source create_component_config.inc

# Create local manifest file for current server instance
--let LOCAL_MANIFEST_CONTENT = `SELECT CONCAT('{ \"components\": \"file://', '$COMPONENT_NAME', '\" }')`
--source include/keyring_tests/helper/instance_create_manifest.inc

# Restart server with manifest file 
--source include/keyring_tests/helper/start_server_with_manifest.inc
--echo # ----------------------------------------------------------------------
