# Setup test to use keyring component

--echo # ----------------------------------------------------------------------
--echo # Setup
--let $rpl_skip_start_slave= 1
--source include/rpl/init_source_replica.inc

--let RPL_PLUGIN_DIR_OPT = $KEYRING_VAULT_COMPONENT_OPT

--let COMPONENT_LIBRARY = `SELECT SUBSTRING_INDEX('$KEYRING_VAULT_COMPONENT_LOAD', '=', -1)`
--let COMPONENT_DIR = $KEYRING_VAULT_COMPONENT_DIR
--let COMPONENT_NAME = `SELECT SUBSTRING_INDEX('$COMPONENT_LIBRARY', '.', 1)`

# Create keyring config for primary server

# Data directory location
--let CURRENT_DATADIR = `SELECT @@datadir`

# Create local keyring config
--source generate_comp_config_content.inc
--let KEYRING_VAULT_PATH = `SELECT CONCAT( '$MYSQLTEST_VARDIR', '/keyring_vault_primary')`
--let KEYRING_CONFIG_CONTENT = `SELECT CONCAT('{ ', '$config_content', ' }')`
--source include/keyring_tests/helper/local_keyring_create_config.inc

# Create local manifest file for current server instance
--let LOCAL_MANIFEST_CONTENT = `SELECT CONCAT('{ \"components\": \"file://', '$COMPONENT_NAME', '\" }')`
--source include/keyring_tests/helper/instance_create_manifest.inc

--let PRIMARY_DATADIR = $CURRENT_DATADIR
--let PRIMARY_KEYRING_VAULT_PATH = $KEYRING_VAULT_PATH

# Create keyring config for replica server

--source include/rpl/connection_replica.inc
--let CURRENT_DATADIR = `SELECT @@datadir`

# Create local keyring config
--let KEYRING_VAULT_PATH = `SELECT CONCAT( '$MYSQLTEST_VARDIR', '/keyring_vault_replica')`
--let KEYRING_CONFIG_CONTENT = `SELECT CONCAT('{ ', '$config_content', ' }')`
--source include/keyring_tests/helper/local_keyring_create_config.inc

# Create local manifest file for current server instance
--let LOCAL_MANIFEST_CONTENT = `SELECT CONCAT('{ \"components\": \"file://', '$COMPONENT_NAME', '\" }')`
--source include/keyring_tests/helper/instance_create_manifest.inc

--let REPLICA_DATADIR = $CURRENT_DATADIR
--let REPLICA_KEYRING_VAULT_PATH = $KEYRING_VAULT_PATH

--source include/rpl/connection_source.inc
--let CURRENT_DATADIR = `SELECT @@datadir`
--echo # ----------------------------------------------------------------------
