--disable_query_log
--disable_result_log

CREATE TEMPORARY TABLE digest_type(
  id INT UNSIGNED NOT NULL AUTO_INCREMENT,
  digest_name VARCHAR(64) NOT NULL,
  builtin_template VARCHAR(64),
  PRIMARY KEY(id),
  UNIQUE(digest_name)
);

--enable_query_log
--enable_result_log

--let $openssl_binary_version_do_not_skip = 1

--let $openssl_binary_version = 1\\.0\\.2
--source include/have_openssl_binary_version.inc
if ($openssl_binary_version_match)
{
  --disable_query_log
  --disable_result_log

  INSERT INTO digest_type(digest_name, builtin_template) VALUES
    ('md5'      , 'MD5(@message)'),
    ('sha1'     , 'SHA(@message)'),

    ('sha224'   , 'SHA2(@message, 224)'),
    ('sha256'   , 'SHA2(@message, 256)'),
    ('sha384'   , 'SHA2(@message, 384)'),
    ('sha512'   , 'SHA2(@message, 512)'),

    ('md4'      , NULL),
    ('sha'      , NULL),
    ('ripemd160', NULL),
    ('whirlpool', NULL)
  ;

  --enable_query_log
  --enable_result_log
}

--let $openssl_binary_version = 1\\.1\\.0
--source include/have_openssl_binary_version.inc
if ($openssl_binary_version_match)
{
  --disable_query_log
  --disable_result_log

  INSERT INTO digest_type(digest_name, builtin_template) VALUES
    ('md5'       , 'MD5(@message)'),
    ('sha1'      , 'SHA(@message)'),

    ('sha224'    , 'SHA2(@message, 224)'),
    ('sha256'    , 'SHA2(@message, 256)'),
    ('sha384'    , 'SHA2(@message, 384)'),
    ('sha512'    , 'SHA2(@message, 512)'),

    ('md4'       , NULL),
    ('md5-sha1'  , NULL),
    ('ripemd160' , NULL),
    ('whirlpool' , NULL),
    ('blake2b512', NULL),
    ('blake2s256', NULL)
  ;

  --enable_query_log
  --enable_result_log
}

--let $openssl_binary_version = 1\\.1\\.1
--source include/have_openssl_binary_version.inc
if ($openssl_binary_version_match)
{
  --disable_query_log
  --disable_result_log

  INSERT INTO digest_type(digest_name, builtin_template) VALUES
    ('md5'       , 'MD5(@message)'),
    ('sha1'      , 'SHA(@message)'),

    ('sha224'    , 'SHA2(@message, 224)'),
    ('sha256'    , 'SHA2(@message, 256)'),
    ('sha384'    , 'SHA2(@message, 384)'),
    ('sha512'    , 'SHA2(@message, 512)'),

    ('md4'       , NULL),
    ('md5-sha1'  , NULL),
    ('ripemd160' , NULL),
    ('sha512-224', NULL),
    ('sha512-256', NULL),
    ('whirlpool' , NULL),
    ('sm3'       , NULL),
    ('blake2b512', NULL),
    ('blake2s256', NULL),
    ('sha3-224'  , NULL),
    ('sha3-256'  , NULL),
    ('sha3-384'  , NULL),
    ('sha3-512'  , NULL),
    ('shake128'  , NULL),
    ('shake256'  , NULL)
  ;

  --enable_query_log
  --enable_result_log
}

--let $openssl_binary_version = 3\\.0\\.\\d+
--source include/have_openssl_binary_version.inc
if ($openssl_binary_version_match)
{
  --disable_query_log
  --disable_result_log

  # skipping MD4 and RMD 160 as they are from the deprecated engine

  INSERT INTO digest_type(digest_name, builtin_template) VALUES
    ('md5'       , 'MD5(@message)'),
    ('sha1'      , 'SHA(@message)'),

    ('sha224'    , 'SHA2(@message, 224)'),
    ('sha256'    , 'SHA2(@message, 256)'),
    ('sha384'    , 'SHA2(@message, 384)'),
    ('sha512'    , 'SHA2(@message, 512)'),

    ('md5-sha1'  , NULL),
    ('sha512-224', NULL),
    ('sha512-256', NULL),
    ('sha3-224'  , NULL),
    ('sha3-256'  , NULL),
    ('sha3-384'  , NULL),
    ('sha3-512'  , NULL)
  ;

  # BLAKE2B512, BLAKE2S256, SHAKE128, SHAKE256 and SM3 can only be used in
  # create_digest() and not in asymmetric_sign() as these digests do not have
  # corresponding asn1 object identifiers
  if ($include_digests_with_no_ans1_ids)
  {
    INSERT INTO digest_type(digest_name, builtin_template) VALUES
      ('sm3'       , NULL),
      ('blake2b512', NULL),
      ('blake2s256', NULL),
      ('shake128'  , NULL),
      ('shake256'  , NULL)
    ;
  }

  --enable_query_log
  --enable_result_log
}

--let $number_of_digest_types = `SELECT COUNT(*) FROM digest_type`

if (!$number_of_digest_types)
{
  --die cannot extract available digests from the openssl utility
}
