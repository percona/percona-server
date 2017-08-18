# Deletes from Vault all keys created by a server sourcing this file.
# The sourcing test needs to set $KEYRING_CONF_FILE variable to
# the location of keyring_vault configuration file.
let KEYRING_CONF_FILE=$KEYRING_CONF_FILE;
let CURL_OUTPUT_FILE=$MYSQL_TMP_DIR/curl_output.txt;
let SERVER_UUID= query_get_value(SELECT @@SERVER_UUID, @@SERVER_UUID, 1);

--perl
  use strict;
  use MIME::Base64 qw( decode_base64 );
  my $keyring_conf_file= $ENV{'KEYRING_CONF_FILE'} or die("KEYRING_CONF_FILE not set\n");
  my $curl_output_file= $ENV{'CURL_OUTPUT_FILE'} or die("CURL_OUTPUT_FILE not set\n");
  my $server_uuid= $ENV{'SERVER_UUID'} or die("SERVER_UUID not set\n");
  my $token;
  my $vault_url;
  my $secret_mount_point;
  my $vault_ca;
  my $CONF_FILE;
  open(CONF_FILE, "$keyring_conf_file") or die("Could not open configuration file.\n");
  while (my $row = <CONF_FILE>)
  {
    if ($row =~ m/token[ ]*=[ ]*(.*)/)
    {
      $token=$1;
    }
    elsif ($row =~ m/vault_url[ ]*=[ ]*(.*)/)
    {
      $vault_url=$1;
    }
    elsif ($row =~ m/secret_mount_point[ ]*= [ ]*(.*)/)
    {
      $secret_mount_point=$1;
    }
    elsif ($row =~ m/vault_ca[ ]*= [ ]*(.*)/)
    {
      $vault_ca=$1;
    }
  }
  close(CONF_FILE);
  if ($token eq "" || $vault_url eq "" || $secret_mount_point eq "")
  {
    die("Could not read vault credentials from configuration file.\n");
  }

  system(qq#curl -H "X-Vault-Token: $token" --cacert $vault_ca -X GET $vault_url/v1/$secret_mount_point/?list=true > $curl_output_file#);

  my $CURL_OUTPUT_FILE;
  open(CURL_OUTPUT_FILE, "$curl_output_file") or die("Could not open file with output from curl.\n");
  my $curl_output = <CURL_OUTPUT_FILE>;
  close(CURL_OUTPUT_FILE);
  my $keys_list_tag = "keys\":[";
  my $keys_pos = index($curl_output, $keys_list_tag);
  if ($keys_pos != -1)
  {
    $keys_pos += length($keys_list_tag);
    my $keys_end_pos = index($curl_output, "]", $keys_pos);
    if ($keys_pos == -1)
    {
      die("Unable to parse keys list retrieved from Vault.\n");
    }
    my $keys_tags = substr($curl_output, $keys_pos, $keys_end_pos - $keys_pos);
    $keys_tags =~ tr/\"//d;
    my @keys = split(',', $keys_tags);
    foreach(@keys)
    {
      my $decoded_key_sign = decode_base64($_);
      if ($decoded_key_sign =~ m/.*$server_uuid.*/)
      {
        system(qq#curl -H "X-Vault-Token: $token" --cacert $vault_ca -X DELETE $vault_url/v1/$secret_mount_point/$_ &#);
      }
    }
  }
EOF
