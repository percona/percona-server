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
--let KMIP_COMPONENT_CONFIG_FILE = $COMPONENT_DIR/$COMPONENT_NAME.cnf
--let KMIP_COMPONENT_NAME_PART = customized
--let KMIP_COMPONENT_ID_LENGTH = 16
--source create_component_config.inc

# Restart server with manifest file 
--source include/keyring_tests/helper/start_server_with_manifest.inc
--echo # ----------------------------------------------------------------------
