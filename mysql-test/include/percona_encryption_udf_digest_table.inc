CREATE TEMPORARY TABLE digest_type(
  id INT UNSIGNED NOT NULL AUTO_INCREMENT,
  digest_name VARCHAR(64) NOT NULL,
  builtin_template VARCHAR(64),
  PRIMARY KEY(id)
);

INSERT INTO digest_type(digest_name, builtin_template) VALUES
  ('MD5' , 'MD5(@message)'),
  ('SHA1', 'SHA(@message)'),

  ('SHA224', 'SHA2(@message, 224)'),
  ('SHA256', 'SHA2(@message, 256)'),
  ('SHA384', 'SHA2(@message, 384)'),
  ('SHA512', 'SHA2(@message, 512)'),

  ('BLAKE2B512', NULL),
  ('BLAKE2S256', NULL),
  ('MD4'       , NULL),
  ('RIPEMD'    , NULL),
  ('RIPEMD160' , NULL),
  ('RMD160'    , NULL),
  ('SHAKE128'  , NULL),
  ('SHAKE256'  , NULL),
  ('SM3'       , NULL),
  ('WHIRLPOOL' , NULL)
;

--let $number_of_digest_types = `SELECT COUNT(*) FROM digest_type`
