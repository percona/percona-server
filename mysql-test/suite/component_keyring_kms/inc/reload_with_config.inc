# ==== Purpose ====
#
# Rewrite the local keyring_kms configuration with fixed dummy credentials
# plus extra options, then reload the keyring.
#
# Only use it while the keyring data file is empty: no key is decrypted on
# reload then, so the dummy credentials and endpoints are never contacted.
#
# ==== Usage ====
#
# --let $kms_extra_config = , \"endpoint\": \"https://kms.example.invalid\"
# --let $kms_reload_must_fail = 1
# --source ../inc/reload_with_config.inc
#
# ==== Parameters ====
#
# $kms_extra_config
#    JSON members appended to the configuration, each preceded by a comma.
#    Empty for none.
#
# $kms_reload_must_fail
#    1 when the reload is expected to fail, empty otherwise.
#    On failure the last configuration error from the error log is shown.

--let KEYRING_CONFIG_CONTENT = `SELECT CONCAT('{ \"path\": \"', '$KEYRING_KMS_PATH', '\", \"region\": \"us-east-1\", \"kms_key\": \"mtr-kms-key\", \"auth_key\": \"mtr-auth-key\", \"secret_access_key\": \"mtr-secret-access-key\", \"read_only\": false', '$kms_extra_config', ' }')`
--source include/keyring_tests/helper/local_keyring_create_config.inc

if ($kms_reload_must_fail)
{
  --error ER_RELOAD_KEYRING_FAILURE
  ALTER INSTANCE RELOAD KEYRING;
  SELECT DATA FROM performance_schema.error_log
    WHERE DATA LIKE '%component_keyring_kms reported: ''Configuration error:%'
    ORDER BY LOGGED DESC LIMIT 1;
}
if (!$kms_reload_must_fail)
{
  ALTER INSTANCE RELOAD KEYRING;
}

SELECT STATUS_KEY, STATUS_VALUE FROM performance_schema.keyring_component_status
  WHERE STATUS_KEY IN ('Component_status', 'Endpoint');
