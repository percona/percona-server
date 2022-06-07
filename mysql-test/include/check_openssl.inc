# Only run this test if the server was built with OpenSSL
--let $rsa_var = query_get_value("SHOW STATUS LIKE 'Rsa_public_key'", Variable_name, 1)
if ($rsa_var != 'Rsa_public_key'){
  --skip Need OpenSSL support
}
