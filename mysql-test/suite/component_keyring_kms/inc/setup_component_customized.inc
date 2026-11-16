# Setup test to use keyring component

--echo # ----------------------------------------------------------------------
--echo # Setup

--let PLUGIN_DIR_OPT = $KEYRING_KMS_COMPONENT_OPT

--let COMPONENT_LIBRARY = `SELECT SUBSTRING_INDEX('$KEYRING_KMS_COMPONENT_LOAD', '=', -1)`
--let COMPONENT_DIR = $KEYRING_KMS_COMPONENT_DIR
--let COMPONENT_NAME = `SELECT SUBSTRING_INDEX('$COMPONENT_LIBRARY', '.', 1)`
--let GLOBAL_MANIFEST_CONTENT = `SELECT CONCAT('{ \"components\": \"file://', '$COMPONENT_NAME', '\" }')`
# Create manifest file for mysqld binary
--source include/keyring_tests/helper/binary_create_customized_manifest.inc

# Create global keyring config
--let KEYRING_KMS_PATH = `SELECT CONCAT( '$MYSQLTEST_VARDIR', '/keyring_kms')`
--let KEYRING_CONFIG_CONTENT = `SELECT CONCAT('{ \"path\": \"', '$KEYRING_KMS_PATH', '\", \"region\": \"', '$KMS_REGION', '\", \"kms_key\": \"', '$KMS_KEY', '\", \"auth_key\": \"', '$KMS_AUTH_KEY', '\", \"secret_access_key\": \"', '$KMS_SECRET_ACCESS_KEY', '\", \"read_only\": false', IF('$KMS_ENDPOINT' = '', '', CONCAT(', \"endpoint\": \"', '$KMS_ENDPOINT', '\"')), IF('$KMS_SESSION_TOKEN' = '', '', CONCAT(', \"session_token\": \"', '$KMS_SESSION_TOKEN', '\"')), ' }')`
--source include/keyring_tests/helper/global_keyring_create_customized_config.inc

# A plain HTTP KMS_ENDPOINT (an emulator) makes the component warn
if (`SELECT '$KMS_ENDPOINT' LIKE 'http:%'`)
{
  --disable_query_log
  CALL mtr.add_suppression("uses plain HTTP: keys are sent to it unencrypted");
  --enable_query_log
}

# Restart server with manifest file 
--source include/keyring_tests/helper/start_server_with_manifest.inc
--echo # ----------------------------------------------------------------------

