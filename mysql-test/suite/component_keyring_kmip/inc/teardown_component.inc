# Teardown

--echo # ----------------------------------------------------------------------
--echo # Teardown

# Tests that enable innodb_redo_log_encrypt leave the redo log encrypted.
# Restart without encryption first so the subsequent inter-test restart can
# start InnoDB without needing the keyring to recover the redo log.
if ($KEYRING_KMIP_USE_BINARY_MANIFEST)
{
  let $restart_parameters = restart: $PLUGIN_DIR_OPT;
  --source include/restart_mysqld_no_echo.inc
  --let $KEYRING_KMIP_USE_BINARY_MANIFEST=
}

# Remove local manifest file for current server instance
--source include/keyring_tests/helper/instance_remove_manifest.inc

# Remove keyring kmip
--source include/keyring_tests/helper/local_keyring_kmip_remove.inc

# Remove local keyring config
--source include/keyring_tests/helper/local_keyring_remove_config.inc

# Keep shared global keyring config in place to avoid parallel test races.
# Preserve the standard teardown output for existing .result files.
--source include/keyring_tests/helper/global_keyring_remove_config.inc

# Restart server without manifest file
--source include/keyring_tests/helper/cleanup_server_with_manifest.inc
--echo # ----------------------------------------------------------------------
