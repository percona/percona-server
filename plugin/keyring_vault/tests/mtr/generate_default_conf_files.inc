--let $KEYRING_CONF_FILE_1=$MYSQL_TMP_DIR/keyring_vault1.conf
--let $KEYRING_CONF_FILE_2=$MYSQL_TMP_DIR/keyring_vault2.conf
--let $KEYRING_INVALID_TOKEN_CONF_FILE=$MYSQL_TMP_DIR/keyring_vault_invalid_token.conf

--let $KEYRING_CONF_TEMPLATE_FILE=$MYSQL_TEST_DIR/std_data/keyring_vault_confs/keyring_vault_mtr_template1.conf
--let $KEYRING_CONF_FILE_TO_GENERATE=$KEYRING_CONF_FILE_1
--source ../plugin/keyring_vault/tests/mtr/generate_conf_file.inc

--let $KEYRING_CONF_TEMPLATE_FILE=$MYSQL_TEST_DIR/std_data/keyring_vault_confs/keyring_vault_mtr_template2.conf
--let $KEYRING_CONF_FILE_TO_GENERATE=$KEYRING_CONF_FILE_2
--source ../plugin/keyring_vault/tests/mtr/generate_conf_file.inc

--let $KEYRING_CONF_TEMPLATE_FILE=$MYSQL_TEST_DIR/std_data/keyring_vault_confs/keyring_vault_mtr_invalid_token_template.conf
--let $KEYRING_CONF_FILE_TO_GENERATE=$KEYRING_INVALID_TOKEN_CONF_FILE
--source ../plugin/keyring_vault/tests/mtr/generate_conf_file.inc
