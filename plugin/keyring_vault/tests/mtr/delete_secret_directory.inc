# Deletes directory with secrets inside a mount point. It deletes all secrets in
# directory. When all secrets get deleted from a directory - directory itself
# is removed by Vault Server.

# The sourcing test needs to set $KEYRING_CONF_FILE variable to
# the location of keyring_vault configuration file
let KEYRING_CONF_FILE=$KEYRING_CONF_FILE;
let SERVER_UUID= query_get_value(SELECT @@SERVER_UUID, @@SERVER_UUID, 1);

--perl
  use strict;
  use MIME::Base64 qw( decode_base64 );
  my $keyring_conf_file= $ENV{'KEYRING_CONF_FILE'} or die("KEYRING_CONF_FILE not set\n");
  my $server_uuid= $ENV{'SERVER_UUID'} or die("SERVER_UUID not set\n");
  my $vardir= $ENV{MYSQLTEST_VARDIR} or die "Need MYSQLTEST_VARDIR";
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

  my $vault_ca_cert_opt= "";
  if ($vault_ca)
  {
    $vault_ca_cert_opt= "--cacert $vault_ca";
  }

  system(qq#curl --http1.1 -L -H "X-Vault-Token: $token" -X LIST $vault_ca_cert_opt $vault_url/v1/$secret_mount_point > $vardir/tmp/curl_list_result#);

  my $curl_conn_successful = 1;
  my $curl_response = 0;

  if (!-s "$vardir/tmp/curl_list_result")
  {
    # result file is empty, thus connection could not be established
    $curl_conn_successful = 0;
  }
  else
  {
    # Check if vault server has returned errors
    open my $file, '<', "$vardir/tmp/curl_list_result";
    $curl_response = <$file>;
    if (index($curl_response, "\"errors\":[\"") != -1)
    {
      $curl_conn_successful = 0;
    }
    close $file;
  }

  if (!$curl_conn_successful)
  {
    if ($curl_response)
    {
      die("--Cannot delete secret directory due to : $curl_response. Please remove directory $secret_mount_point manualy.");
    }
    else
    {
      die("--Cannot delete secret directory. Please remove directory $secret_mount_point manualy.");
    }
  }

  open(my $help_file, '>', "$vardir/tmp/help_file");

  # First we need to get a list of secrets inside a directory. To do that we need to parse a response and get all secret ids from keys:"[]" identifier.
  # Next we need to split on ',' to get list of secret ids in an array (@secrets) "id1","id2" etc. The last step is to get the key id without
  # double quotes and delete the secret. When all secrets are removed from a directory Vault will remove the directory itself.

  my $CURL_LIST_RESULT;
  open(CURL_LIST_RESULT, "$vardir/tmp/curl_list_result") or die("Could not open file with list of secrets in a mount point. Please remove directory $secret_mount_point manualy.\n");
  while (my $row = <CURL_LIST_RESULT>)
  {
    print $help_file $row;
    if ($row =~ m/keys":\[(.*)\]/)
    {
      my $secrets_string;
      $secrets_string = $1;
      print $help_file $secrets_string;
      my @secrets = split /,/, $secrets_string;
      foreach (@secrets)
      {
        print $help_file $_;
        if ($_ =~ m/"(.*)"/)
        {
          print $help_file $1;
          system(qq#curl --http1.1 -L -H "X-Vault-Token: $token" $vault_ca_cert_opt -X DELETE $vault_url/v1/$secret_mount_point/$1#);
        }
      }
    }
  }
  close(help_file);
  close(CURL_LIST_RESULT);
EOF
