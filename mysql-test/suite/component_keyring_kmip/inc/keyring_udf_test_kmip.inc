--echo # ----------------------------------------------------------------------
--echo # Setting up Keyring UDFs

# Install Keyring UDF plugin
--replace_regex /\.dll/.so/
--eval INSTALL PLUGIN keyring_udf SONAME '$KEYRING_UDF'

--replace_regex /\.dll/.so/
--eval CREATE FUNCTION keyring_key_store RETURNS INTEGER SONAME '$KEYRING_UDF'
--replace_regex /\.dll/.so/
--eval CREATE FUNCTION keyring_key_generate RETURNS INTEGER SONAME '$KEYRING_UDF'
--replace_regex /\.dll/.so/
--eval CREATE FUNCTION keyring_key_fetch RETURNS sTRING SONAME '$KEYRING_UDF'
--replace_regex /\.dll/.so/
--eval CREATE FUNCTION keyring_key_type_fetch RETURNS STRING SONAME '$KEYRING_UDF'
--replace_regex /\.dll/.so/
--eval CREATE FUNCTION keyring_key_length_fetch RETURNS INTEGER SONAME '$KEYRING_UDF'
--replace_regex /\.dll/.so/
--eval CREATE FUNCTION keyring_key_remove RETURNS INTEGER SONAME '$KEYRING_UDF'

--echo # ----------------------------------------------------------------------

--disable_result_log
--disable_query_log
--disable_abort_on_error
# Best-effort cleanup for strict KMIP servers that reject duplicate key names.
SELECT keyring_key_remove('AES_g1');
SELECT keyring_key_remove('AES_g2');
SELECT keyring_key_remove('AES_g3');
SELECT keyring_key_remove('AES_s1');
SELECT keyring_key_remove('AES_s2');
SELECT keyring_key_remove('AES_s3');
--enable_abort_on_error
--enable_query_log
--enable_result_log

--echo # Tests for AES key type
--echo # keyring_key_generate tests
SELECT keyring_key_generate('AES_g1', 'AES', 16);
SELECT keyring_key_generate('AES_g2', 'AES', 32);
--error ER_KEYRING_UDF_KEYRING_SERVICE_ERROR
SELECT keyring_key_generate('AES_g3', 'AES', 64) INTO @x;
SELECT @x;

--echo # keyring_key_store tests
--error ER_KEYRING_UDF_KEYRING_SERVICE_ERROR
SELECT keyring_key_store('AES_s1', 'AES', 'Quick brown fox jumped over the lazy dog') INTO @x;
SELECT @x;
SELECT keyring_key_store('AES_s2', 'AES', 'Old MacDonald had a farm');
--error ER_KEYRING_UDF_KEYRING_SERVICE_ERROR
SELECT keyring_key_store('AES_s3', 'AES', 'Quis custodiet ipsos custodes') INTO @x;
SELECT @x;

--echo # keyring_key_fetch tests
SELECT keyring_key_fetch('AES_g1') INTO @aes_g1_fetched;
SELECT keyring_key_fetch('AES_g2') INTO @aes_g2_fetched;
SELECT keyring_key_fetch('AES_g3') INTO @aes_g3_fetched;
SELECT LENGTH(@aes_g1_fetched);
SELECT LENGTH(@aes_g2_fetched);
SELECT LENGTH(@aes_g3_fetched);

SELECT keyring_key_fetch('AES_s1');
SELECT keyring_key_fetch('AES_s2');
SELECT keyring_key_fetch('AES_s3');

--echo # keyring key_type_fetch tests
SELECT keyring_key_type_fetch('AES_g1');
SELECT keyring_key_type_fetch('AES_g2');
SELECT keyring_key_type_fetch('AES_g3');
SELECT keyring_key_type_fetch('AES_s1');
SELECT keyring_key_type_fetch('AES_s2');
SELECT keyring_key_type_fetch('AES_s3');

--echo # keyring key_length_fetch tests
SELECT keyring_key_length_fetch('AES_g1');
SELECT keyring_key_length_fetch('AES_g2');
SELECT keyring_key_length_fetch('AES_g3');
SELECT keyring_key_length_fetch('AES_s1');
SELECT keyring_key_length_fetch('AES_s2');
SELECT keyring_key_length_fetch('AES_s3');

--echo # Restarting the server
--source include/force_restart.inc

--echo # keyring_key_fetch tests after restart
SELECT keyring_key_fetch('AES_g1') INTO @aes_g1_fetched_after_restart;
SELECT keyring_key_fetch('AES_g2') INTO @aes_g2_fetched_after_restart;
SELECT keyring_key_fetch('AES_g3') INTO @aes_g3_fetched_after_restart;
SELECT LENGTH(@aes_g1_fetched_after_restart);
SELECT LENGTH(@aes_g2_fetched_after_restart);
SELECT LENGTH(@aes_g3_fetched_after_restart);

SELECT keyring_key_fetch('AES_s1');
SELECT keyring_key_fetch('AES_s2');
SELECT keyring_key_fetch('AES_s3');

--echo # keyring key_type_fetch tests after restart
SELECT keyring_key_type_fetch('AES_g1');
SELECT keyring_key_type_fetch('AES_g2');
SELECT keyring_key_type_fetch('AES_g3');

--echo # keyring key_length_fetch tests after restart
SELECT keyring_key_length_fetch('AES_g1');
SELECT keyring_key_length_fetch('AES_g2');
SELECT keyring_key_length_fetch('AES_g3');

--echo # keyring_key_remove tests
SELECT keyring_key_remove('AES_g1');
SELECT keyring_key_remove('AES_g2');
--error ER_KEYRING_UDF_KEYRING_SERVICE_ERROR
SELECT keyring_key_remove('AES_g3') INTO @x;
SELECT @x;
--error ER_KEYRING_UDF_KEYRING_SERVICE_ERROR
SELECT keyring_key_remove('AES_s1') INTO @x;
SELECT @x;
SELECT keyring_key_remove('AES_s2');
--error ER_KEYRING_UDF_KEYRING_SERVICE_ERROR
SELECT keyring_key_remove('AES_s3') INTO @x;
SELECT @x;

--echo # ----------------------------------------------------------------------

--echo # Clean-up
DROP FUNCTION keyring_key_store;
DROP FUNCTION keyring_key_fetch;
DROP FUNCTION keyring_key_remove;
DROP FUNCTION keyring_key_generate;
DROP FUNCTION keyring_key_type_fetch;
DROP FUNCTION keyring_key_length_fetch;
UNINSTALL PLUGIN keyring_udf;
--echo # ----------------------------------------------------------------------
