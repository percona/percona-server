#
# Creating functions from encryption_udf
#
INSTALL COMPONENT 'file://component_encryption_udf';

--let $include_digests_with_no_ans1_ids = 0
--source digest_table.inc

--echo
--echo ** checking private key generation, public key derivation, encryption/decryption and sign/verify functionality
--disable_query_log

--echo
--echo ** [$algorithm] checking asymmetric algorithm functions
eval SET @algorithm = "$algorithm";

--echo ** [$algorithm][$key_length] checking asymmetric algorithm functions
if (!$use_openssl_binary)
{
  --echo **** [$algorithm][$key_length] generating private key
  eval SET @priv = create_asymmetric_priv_key(@algorithm, $key_length);
}
if ($use_openssl_binary)
{
  --let $private_key_file_path = $MYSQLTEST_VARDIR/tmp/private.pem
  --echo **** [$algorithm][$key_length] generating private key with openssl binary
  if ($algorithm == 'RSA')
  {
    --let $subcommand = genrsa
  }
  if ($algorithm == 'DSA')
  {
    --let $subcommand = dsaparam -genkey -noout
  }
  --exec $OPENSSL_EXECUTABLE $subcommand -out $private_key_file_path $key_length
  --replace_result $private_key_file_path <PRIVATE_KEY_FILE_PATH>
  eval SET @priv = TRIM(TRAILING '\n' FROM LOAD_FILE('$private_key_file_path'));
}

if (!$use_openssl_binary)
{
  --echo **** [$algorithm][$key_length] deriving public key
  SET @pub = create_asymmetric_pub_key(@algorithm, @priv);
}
if ($use_openssl_binary)
{
  --let $public_key_file_path = $MYSQLTEST_VARDIR/tmp/public.pem
  --echo **** [$algorithm][$key_length] deriving public key with openssl binary
  if ($algorithm == 'RSA')
  {
    --let $subcommand = rsa
  }
  if ($algorithm == 'DSA')
  {
    --let $subcommand = dsa
  }
  --exec $OPENSSL_EXECUTABLE $subcommand -in $private_key_file_path -pubout -out $public_key_file_path
  --replace_result $public_key_file_path <PUBLIC_KEY_FILE_PATH>
  eval SET @pub = TRIM(TRAILING '\n' FROM LOAD_FILE('$public_key_file_path'));
  --remove_file $public_key_file_path
  --remove_file $private_key_file_path

  --let $assert_text = [$algorithm][$key_length] public key created with create_asymmetric_pub_key must be identical to the one created with openssl $subcommand
  --let $assert_cond = @pub = create_asymmetric_pub_key(@algorithm, @priv)
  --let $include_silent = 1
  --source include/assert.inc
  --let $include_silent =
}

#  <max message size> = <key length in bytes>
--let $max_message_size = `SELECT $key_length DIV 8`

# size of MD5 hash in bytes is 32
eval SET @random_pattern = REPEAT(MD5(42), $max_message_size DIV 32 + 1);

if ($algorithm == 'RSA')
{
  --let $padding_scheme_index = 1
  while ($padding_scheme_index <= 3)
  {
    --let $padding_scheme = `SELECT ELT($padding_scheme_index, 'no', 'pkcs1', 'oaep')`
    eval SET @padding_scheme = '$padding_scheme';

    --echo **** [$algorithm][$key_length][$padding_scheme] checking encrypt/decrypt operations on NULL message
    SET @message = NULL;
    --echo ****** [$algorithm][$key_length][$padding_scheme] checking encryption with public key of NULL message
    --error ER_UDF_ERROR
    SELECT asymmetric_encrypt(@algorithm, @message, @pub, @padding_scheme);
    --echo ****** [$algorithm][$key_length][$padding_scheme] checking encryption with private key of NULL message
    --error ER_UDF_ERROR
    SELECT asymmetric_encrypt(@algorithm, @message, @priv, @padding_scheme);

    --let $max_unpadded_message_size = `SELECT $max_message_size - ELT($padding_scheme_index, 0, 11, 42)`

    --let $message_length = $max_unpadded_message_size + 1
    --echo **** [$algorithm][$key_length][$padding_scheme] checking encrypt/decrypt operations on oversize message
    eval SET @message = LEFT(@random_pattern, $message_length);
    --echo ****** [$algorithm][$key_length][$padding_scheme] checking encryption of oversize message with public key
    --error ER_UDF_ERROR
    SELECT asymmetric_encrypt(@algorithm, @message, @pub, @padding_scheme);
    --echo ****** [$algorithm][$key_length][$padding_scheme] checking encryption of oversize message with private key
    --error ER_UDF_ERROR
    SELECT asymmetric_encrypt(@algorithm, @message, @priv, @padding_scheme);

    --let $message_index = 1
    if ($padding_scheme == 'no') {
      # for encryption with no padding we can only encrypt messages of size equal to the key size
      --let $message_index = 4
    }
    while ($message_index <= 4)
    {
      --let $message_length = `SELECT ELT($message_index, 0, 1, $max_unpadded_message_size DIV 2, $max_unpadded_message_size)`
      eval SET @message = LEFT(@random_pattern, $message_length);
      --echo **** [$algorithm][$key_length][$padding_scheme][message_len=$message_length] checking encrypt/decrypt operations

      --echo ****** [$algorithm][$key_length][$padding_scheme][message_len=$message_length] checking encryption with public key
      SET @message_enc_with_pub = asymmetric_encrypt(@algorithm, @message, @pub, @padding_scheme);
      if ($padding_scheme != 'no')
      {
        # RSA encryption with public key with  'pkcs1'/ 'oaep' padding schemes should include random bytes in padding and therefore
        # calls to "asymmetric_encrypt('RSA', @message, @pub, @padding_scheme)" with the same arguments are expected to
        # return different results
        --let $assert_text = [$algorithm][$key_length][message_len=$message_length] message encrypted with public key for the second time must differ
        --let $assert_cond = @message_enc_with_pub <> asymmetric_encrypt(@algorithm, @message, @pub, @padding_scheme)
      }
      if ($padding_scheme == 'no')
      {
        # RSA encryption with public key with no padding should not include random bytes and therefore
        # calls to "asymmetric_encrypt('RSA', @message, @pub, @padding_scheme)" with the same arguments are expected to
        # return identical results
        --let $assert_text = [$algorithm][$key_length][message_len=$message_length] message encrypted with public key for the second time must be the same
        --let $assert_cond = @message_enc_with_pub = asymmetric_encrypt(@algorithm, @message, @pub, @padding_scheme)
      }
      --let $include_silent = 1
      --source include/assert.inc
      --let $include_silent =

      --echo ****** [$algorithm][$key_length][$padding_scheme][message_len=$message_length] checking decryption with private key
      SET @message_dec_with_priv = asymmetric_decrypt(@algorithm, @message_enc_with_pub, @priv, @padding_scheme);
      --let $assert_text = [$algorithm][$key_length][message_len=$message_length] message decrypted with private key must match the original one
      --let $assert_cond = @message_dec_with_priv = @message
      --let $include_silent = 1
      --source include/assert.inc
      --let $include_silent =

      if ($padding_scheme != 'oaep')
      {
        # encrypting with RSA private key is not supported for PKCS1 OAEP padding

        --echo ****** [$algorithm][$key_length][$padding_scheme][message_len=$message_length] checking encryption with private key
        SET @message_enc_with_priv = asymmetric_encrypt(@algorithm, @message, @priv, @padding_scheme);
        # RSA encryption with private key should be deterministic and therefore
        # calls to "asymmetric_encrypt('RSA', @message, @pub, @padding_scheme)" with the same arguments are expected to
        # return identical results
        --let $assert_text = [$algorithm][$key_length][message_len=$message_length] message encrypted with private key for the second time must not differ
        --let $assert_cond = @message_enc_with_priv = asymmetric_encrypt(@algorithm, @message, @priv, @padding_scheme)
        --let $include_silent = 1
        --source include/assert.inc
        --let $include_silent =

        --echo ****** [$algorithm][$key_length][$padding_scheme][message_len=$message_length] checking decryption with public key
        SET @message_dec_with_pub = asymmetric_decrypt(@algorithm, @message_enc_with_priv, @pub, @padding_scheme);
        --let $assert_text = [$algorithm][$key_length][message_len=$message_length] message decrypted with public key must match the original one
        --let $assert_cond = @message_dec_with_pub = @message
        --let $include_silent = 1
        --source include/assert.inc
        --let $include_silent =
      }

      --inc $message_index
    }

    --inc $padding_scheme_index
  }
}


--let $message_index = 1
while ($message_index <= 4)
{
  --let $message_length = `SELECT ELT($message_index, 0, 1, $max_message_size DIV 2, $max_message_size)`
  eval SET @message = LEFT(@random_pattern, $message_length);

  --echo **** [$algorithm][$key_length][message_len=$message_length] checking sign/verify operations
  --let $digest_type_idx = 1
  while($digest_type_idx <= $number_of_digest_types)
  {
    --let $digest_name = `SELECT digest_name FROM digest_type WHERE id = $digest_type_idx`
    eval SET @digest_type = '$digest_name';
    SET @message_digest = create_digest(@digest_type, @message);
    if ($algorithm == 'RSA')
    {
      SET @primary_signature_padding = 'pkcs1';
      SET @message_signature = asymmetric_sign(@algorithm, @message_digest, @priv, @digest_type, @primary_signature_padding);
      # RSA signing with private key should be deterministic and therefore
      # calls to "asymmetric_sign('RSA', @message_digest, @priv, @digest_type, @primary_signature_padding)" with the same arguments
      # are expected to return identical results
      --let $assert_text = [$algorithm][$key_length][message_len=$message_length][$digest_name] calculating pkcs1 signature for the second time must not differ
      --let $assert_cond = @message_signature = asymmetric_sign(@algorithm, @message_digest, @priv, @digest_type, @primary_signature_padding)
      --let $include_silent = 1
      --source include/assert.inc
      --let $include_silent =

      SET @extra_signature_padding = 'pkcs1_pss';
      SET @extra_message_signature = asymmetric_sign(@algorithm, @message_digest, @priv, @digest_type, @extra_signature_padding);
      --let $assert_text = [$algorithm][$key_length][message_len=$message_length][$digest_name] calculating pkcs1_pss signature for the second time must differ
      --let $assert_cond = @extra_message_signature <> asymmetric_sign(@algorithm, @message_digest, @priv, @digest_type, @extra_signature_padding)
      --let $include_silent = 1
      --source include/assert.inc
      --let $include_silent =

      SET @verification_result = asymmetric_verify(@algorithm, @message_digest, @message_signature, @pub, @digest_type, @primary_signature_padding);
      --let $assert_text = [$algorithm][$key_length][message_len=$message_length][$digest_name] pkcs1 signature must pass verification
      --let $assert_cond = @verification_result = 1
      --let $include_silent = 1
      --source include/assert.inc
      --let $include_silent =

      SET @extra_verification_result = asymmetric_verify(@algorithm, @message_digest, @message_signature, @pub, @digest_type, @primary_signature_padding);
      --let $assert_text = [$algorithm][$key_length][message_len=$message_length][$digest_name] pkcs1_pss signature must pass verification
      --let $assert_cond = @extra_verification_result = 1
      --let $include_silent = 1
      --source include/assert.inc
      --let $include_silent =

      SET @first_cross_verification_result = asymmetric_verify(@algorithm, @message_digest, @extra_message_signature, @pub, @digest_type, @primary_signature_padding);
      --let $assert_text = [$algorithm][$key_length][message_len=$message_length][$digest_name] cross pkcs1_pss-pkcs1 signature must not pass verification
      --let $assert_cond = @first_cross_verification_result = 0
      --let $include_silent = 1
      --source include/assert.inc
      --let $include_silent =

      SET @second_cross_verification_result = asymmetric_verify(@algorithm, @message_digest, @message_signature, @pub, @digest_type, @extra_signature_padding);
      --let $assert_text = [$algorithm][$key_length][message_len=$message_length][$digest_name] cross pkcs1-pkcs1_pss signature must not pass verification
      --let $assert_cond = @second_cross_verification_result = 0
      --let $include_silent = 1
      --source include/assert.inc
      --let $include_silent =
    }
    if ($algorithm == 'DSA')
    {
      SET @message_signature = asymmetric_sign(@algorithm, @message_digest, @priv, @digest_type);
      # DSA signing with private key should include random bytes in padding and therefore
      # calls to "asymmetric_sign('DSA', @message_digest, @priv, @digest_type)" with the same arguments
      # are expected to return different results
      --let $assert_text = [$algorithm][$key_length][message_len=$message_length][$digest_name] calculating signature for the second time must not differ
      --let $assert_cond = @message_signature <> asymmetric_sign(@algorithm, @message_digest, @priv, @digest_type)
      --let $include_silent = 1
      --source include/assert.inc
      --let $include_silent =

      SET @verification_result = asymmetric_verify(@algorithm, @message_digest, @message_signature, @pub, @digest_type);
      --let $assert_text = [$algorithm][$key_length][message_len=$message_length][$digest_name] signature must pass verification
      --let $assert_cond = @verification_result = 1
      --let $include_silent = 1
      --source include/assert.inc
      --let $include_silent =
    }

    --inc $digest_type_idx
  }

  --inc $message_index
}

--enable_query_log


#
# Dropping functions from encryption_udf
#
UNINSTALL COMPONENT 'file://component_encryption_udf';
