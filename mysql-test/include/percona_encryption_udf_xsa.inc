#
# Creating functions from encryption_udf
#

--replace_result $ENCRYPTION_UDF_LIB ENCRYPTION_UDF_LIB
eval CREATE FUNCTION create_digest RETURNS STRING SONAME "$ENCRYPTION_UDF_LIB";

--replace_result $ENCRYPTION_UDF_LIB ENCRYPTION_UDF_LIB
eval CREATE FUNCTION create_asymmetric_priv_key RETURNS STRING SONAME "$ENCRYPTION_UDF_LIB";

--replace_result $ENCRYPTION_UDF_LIB ENCRYPTION_UDF_LIB
eval CREATE FUNCTION create_asymmetric_pub_key RETURNS STRING SONAME "$ENCRYPTION_UDF_LIB";

if ($algorithm == 'RSA')
{
  --replace_result $ENCRYPTION_UDF_LIB ENCRYPTION_UDF_LIB
  eval CREATE FUNCTION asymmetric_encrypt RETURNS STRING SONAME "$ENCRYPTION_UDF_LIB";

  --replace_result $ENCRYPTION_UDF_LIB ENCRYPTION_UDF_LIB
  eval CREATE FUNCTION asymmetric_decrypt RETURNS STRING SONAME "$ENCRYPTION_UDF_LIB";
}

--replace_result $ENCRYPTION_UDF_LIB ENCRYPTION_UDF_LIB
eval CREATE FUNCTION asymmetric_sign RETURNS STRING SONAME "$ENCRYPTION_UDF_LIB";

--replace_result $ENCRYPTION_UDF_LIB ENCRYPTION_UDF_LIB
eval CREATE FUNCTION asymmetric_verify RETURNS INTEGER SONAME "$ENCRYPTION_UDF_LIB";

--source include/percona_encryption_udf_digest_table.inc

--echo
--echo ** checking private key generation, public key derivation, encryption/decryption and sign/verify functionality
--disable_query_log

--echo
--echo ** [$algorithm] checking asymmertic algorithm functions
eval SET @algorithm = "$algorithm";

--echo ** [$algorithm][$key_length] checking asymmetric algorithm functions
--echo **** [$algorithm][$key_length] generating private key
eval SET @priv = create_asymmetric_priv_key(@algorithm, $key_length);
--echo **** [$algorithm][$key_length] deriving public key
SET @pub = create_asymmetric_pub_key(@algorithm, @priv);

# for RSA <max message size> = <key length in bytes> - <pkcs1 padding>
--let $max_message_size = `SELECT $key_length DIV 8`
if ($algorithm == 'RSA')
{
  --let $max_message_size = `SELECT $max_message_size - 11`
}

# size of MD5 hash in bytes is 32
eval SET @random_pattern = REPEAT(MD5(42), $max_message_size DIV 32 + 1);

if ($algorithm == 'RSA')
{
  --echo **** [$algorithm][$key_length] checking operations on NULL message
  SET @message = NULL;
  --echo ****** [$algorithm][$key_length] checking encryption with public key of NULL message
  --error ER_UDF_ERROR
  SELECT asymmetric_encrypt(@algorithm, @message, @pub);
  --echo ****** [$algorithm][$key_length] checking encryption with privcate key of NULL message
  --error ER_UDF_ERROR
  SELECT asymmetric_encrypt(@algorithm, @message, @priv);
}

--let $message_index = 1
while ($message_index <= 4)
{
  --let $message_length = `SELECT ELT($message_index, 0, 1, $max_message_size DIV 2, $max_message_size)`
  eval SET @message = LEFT(@random_pattern, $message_length);
  --echo **** [$algorithm][$key_length][message_len=$message_length] checking operations

  if ($algorithm == 'RSA')
  {
    --echo ****** [$algorithm][$key_length][message_len=$message_length] checking encryption with public key
    SET @message_enc_with_pub = asymmetric_encrypt(@algorithm, @message, @pub);
    --echo ****** [$algorithm][$key_length][message_len=$message_length] checking decryption with private key
    SET @message_dec_with_priv = asymmetric_decrypt(@algorithm, @message_enc_with_pub, @priv);
    --let $assert_text = [$algorithm][$key_length][message_len=$message_length] message decrypted with private key must match the original one
    --let $assert_cond = @message_dec_with_priv = @message
    --let $include_silent = 1
    --source include/assert.inc
    --let $include_silent =

    --echo ****** [$algorithm][$key_length][message_len=$message_length] checking encryption with private key
    SET @message_enc_with_priv = asymmetric_encrypt(@algorithm, @message, @priv);
    --echo ****** [$algorithm][$key_length][message_len=$message_length] checking decryption with public key
    SET @message_dec_with_pub = asymmetric_decrypt(@algorithm, @message_enc_with_priv, @pub);
    --let $assert_text = [$algorithm][$key_length][message_len=$message_length] message decrypted with public key must match the original one
    --let $assert_cond = @message_dec_with_pub = @message
    --let $include_silent = 1
    --source include/assert.inc
    --let $include_silent =
  }

  --echo ****** [$algorithm][$key_length][message_len=$message_length] checking sign/verify functionality
  --let $digest_type_idx = 1
  while($digest_type_idx <= $number_of_digest_types)
  {
    --let $digest_name = `SELECT digest_name FROM digest_type WHERE id = $digest_type_idx`
    --echo ******** [$algorithm][$key_length][message_len=$message_length][$digest_name] calculating digest
    eval SET @digest_type = '$digest_name';
    SET @message_digest = create_digest(@digest_type, @message);
    --echo ******** [$algorithm][$key_length][message_len=$message_length][$digest_name] calculating signature
    SET @message_signature = asymmetric_sign(@algorithm, @message_digest, @priv, @digest_type);
    --echo ******** [$algorithm][$key_length][message_len=$message_length][$digest_name] verifying signature
    SET @verification_result = asymmetric_verify(@algorithm, @message_digest, @message_signature, @pub, @digest_type);
    --let $assert_text = [$algorithm][$key_length][message_len=$message_length][$digest_name] signature must pass verification
    --let $assert_cond = @verification_result = 1
    --let $include_silent = 1
    --source include/assert.inc
    --let $include_silent =

    --inc $digest_type_idx
  }

  --inc $message_index
}

if ($algorithm == 'RSA')
{
  --let $message_length = $max_message_size + 1
  --echo **** [$algorithm][$key_length] checking operations on oversize message
  eval SET @message = LEFT(@random_pattern, $message_length);
  --echo ****** [$algorithm][$key_length] checking encryption of oversize message with public key
  --error ER_UDF_ERROR
  SELECT asymmetric_encrypt(@algorithm, @message, @pub);
  --echo ****** [$algorithm][$key_length] checking encryption of oversize message with private key
  --error ER_UDF_ERROR
  SELECT asymmetric_encrypt(@algorithm, @message, @priv);
}

--enable_query_log


#
# Dropping functions from encryption_udf
#
DROP FUNCTION asymmetric_verify;
DROP FUNCTION asymmetric_sign;
if ($algorithm == 'RSA')
{
  DROP FUNCTION asymmetric_decrypt;
  DROP FUNCTION asymmetric_encrypt;
}
DROP FUNCTION create_asymmetric_pub_key;
DROP FUNCTION create_asymmetric_priv_key;
DROP FUNCTION create_digest;
