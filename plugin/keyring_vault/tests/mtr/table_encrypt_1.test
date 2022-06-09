--source include/have_keyring_vault_plugin.inc

--source generate_default_mount_conf_files.inc
--source is_vault_server_up.inc
--source include/not_repeat.inc

# Create mount points
--let MOUNT_POINT_SERVICE_OP=CREATE
--let $KEYRING_CONF_FILE=$KEYRING_CONF_FILE_1
--source mount_point_service.inc

--let $keyring_restart_param=restart:--early-plugin-load="keyring_vault=$KEYRING_VAULT_PLUGIN" --loose-keyring_vault_config=$KEYRING_CONF_FILE_1 $KEYRING_VAULT_PLUGIN_OPT
--source include/table_encrypt_1.inc

# Delete mount points
--let MOUNT_POINT_SERVICE_OP=DELETE
--let $KEYRING_CONF_FILE=$KEYRING_CONF_FILE_1
--source mount_point_service.inc
