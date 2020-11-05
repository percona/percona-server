# Creates or deletes secret mount point
# Mount point is :
#  - created when $mount_point_service_op is set to CREATE
#  - deleted when $mount_point_service_op is set to DELETE

--let EXPORTED_VAULT_CONF_MOUNT_POINT_UUID = $vault_conf_mount_point_uuid
--let EXPORTED_VAULT_CONF_MOUNT_POINT_SUFFIX = $vault_conf_mount_point_suffix
--let EXPORTED_VAULT_CONF_MOUNT_POINT_DIRECTORY = $vault_conf_mount_point_directory
--let EXPORTED_VAULT_CONF_ADDRESS =
if ($vault_conf_address)
{
  --let EXPORTED_VAULT_CONF_ADDRESS = $vault_conf_address
}
--let EXPORTED_VAULT_CONF_TOKEN =
if ($vault_conf_token)
{
  --let EXPORTED_VAULT_CONF_TOKEN = $vault_conf_token
}
--let EXPORTED_VAULT_SERVER_MOUNT_POINT_VERSION = $vault_server_mount_point_version

--let EXPORTED_MOUNT_POINT_SERVICE_OP = $mount_point_service_op

--perl
  use strict;
  # Vaults' credentials from ENV variables
  my $imported_vault_conf_address = $ENV{'EXPORTED_VAULT_CONF_ADDRESS'};
  if (!$imported_vault_conf_address)
  {
    $imported_vault_conf_address = $ENV{'MTR_VAULT_ADDRESS'} or die "Please export MTR_VAULT_ADDRESS variable";
  }

  my $imported_vault_conf_token = $ENV{'EXPORTED_VAULT_CONF_TOKEN'};
  if (!$imported_vault_conf_token)
  {
    $imported_vault_conf_token = $ENV{'MTR_VAULT_TOKEN'} or die("Please export MTR_VAULT_TOKEN variable");
  }
  my $imported_vault_conf_mount_point_uuid = $ENV{'EXPORTED_VAULT_CONF_MOUNT_POINT_UUID'} or die("EXPORTED_VAULT_CONF_MOUNT_POINT_UUID not set\n");
  my $imported_vault_conf_mount_point_suffix = $ENV{'EXPORTED_VAULT_CONF_MOUNT_POINT_SUFFIX'};
  my $imported_vault_conf_mount_point_directory = $ENV{EXPORTED_VAULT_CONF_MOUNT_POINT_DIRECTORY};
  my $imported_vault_server_mount_point_version = $ENV{EXPORTED_VAULT_SERVER_MOUNT_POINT_VERSION};
  my $imported_mount_point_service_op = $ENV{'EXPORTED_MOUNT_POINT_SERVICE_OP'};

  my $vault_mount = $imported_vault_conf_mount_point_uuid;
  if (!$imported_vault_conf_mount_point_directory)
  {
    if ($imported_vault_conf_mount_point_suffix)
    {
      $vault_mount .= $imported_vault_conf_mount_point_suffix;
    }
  }

  my $vault_ca;
  my $vault_ca_cert_opt= "";
  if ($vault_ca)
  {
    $vault_ca_cert_opt= "--cacert $vault_ca";
  }

  my $postdata= "";
  if ($imported_vault_server_mount_point_version eq "2")
  {
    $postdata= qq#{"type":"kv","options":{"version":"2"}}#;
  } else {
    $postdata= qq#{"type":"kv"}#;
  }

  if ($imported_mount_point_service_op eq 'CREATE')
  {
    system(qq#curl --http1.1 -L -H "X-Vault-Token: $imported_vault_conf_token" $vault_ca_cert_opt --data '$postdata' --request POST $imported_vault_conf_address/v1/sys/mounts/$vault_mount#);
  }
  elsif ($imported_mount_point_service_op eq 'DELETE')
  {
    system(qq#curl --http1.1 -L -H "X-Vault-Token: $imported_vault_conf_token" $vault_ca_cert_opt -X DELETE $imported_vault_conf_address/v1/sys/mounts/$vault_mount#);
  }
  else
  {
    die("Mount point should be either created or deleted. The resulting operation is no-op");
  }
EOF
