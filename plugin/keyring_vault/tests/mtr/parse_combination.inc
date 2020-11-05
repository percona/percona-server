--disable_query_log
SET @jsonified_version_suffix = CONCAT('["', REPLACE(@@version_suffix, '-', '","'), '"]');
SET @extraction_path = '$[0]';
--let $extracted_mode_component = `SELECT JSON_UNQUOTE(JSON_EXTRACT(@jsonified_version_suffix, @extraction_path))`
SET @extraction_path = '$[1]';
--let $extracted_version_component = `SELECT JSON_UNQUOTE(JSON_EXTRACT(@jsonified_version_suffix, @extraction_path))`
--enable_query_log

--let $mode_component_valid = 0
if ($extracted_mode_component == "mount")
{
  --let $vault_conf_mount_point_directory =
  --let $mode_component_valid = 1
}
if ($extracted_mode_component == "directory")
{
  --let $vault_conf_mount_point_directory = server
  --let $mode_component_valid = 1
}
if (!$mode_component_valid)
{
  -- die invalid extracted mode component
}

--let $version_component_valid = 0
if ($extracted_version_component == "v1")
{
  --let $vault_conf_mount_point_version = 1
  --let $vault_server_mount_point_version = 1
  --let $version_component_valid = 1
}
if ($extracted_version_component == "v2")
{
  --let $vault_conf_mount_point_version = 2
  --let $vault_server_mount_point_version = 2
  --let $version_component_valid = 1
}
if ($extracted_version_component == "auto_v1")
{
  --let $vault_conf_mount_point_version = AUTO
  --let $vault_server_mount_point_version = 1
  --let $version_component_valid = 1
}
if ($extracted_version_component == "auto_v2")
{
  --let $vault_conf_mount_point_version = AUTO
  --let $vault_server_mount_point_version = 2
  --let $version_component_valid = 1
}
if ($extracted_version_component == "empty_v1")
{
  --let $vault_conf_mount_point_version =
  --let $vault_server_mount_point_version = 1
  --let $version_component_valid = 1
}
if ($extracted_version_component == "empty_v2")
{
  --let $vault_conf_mount_point_version =
  --let $vault_server_mount_point_version = 2
  --let $version_component_valid = 1
}
if (!$version_component_valid)
{
  -- die invalid extracted version component
}

if (!$MTR_VAULT_VERSION)
{
  --die MTR_VAULT_VERSION is not defined
}

--let $vault_version_valid = 0
if ($MTR_VAULT_VERSION == 1)
{
  --let $vault_version_valid = 1
}
if ($MTR_VAULT_VERSION == 2)
{
  --let $vault_version_valid = 1
}
if (!$vault_version_valid)
{
  -- die invalid MTR_VAULT_VERSION value
}

if ($MTR_VAULT_VERSION == 1)
{
  if ($vault_server_mount_point_version == 2)
  {
    --skip this combination cannot be run on V1 Vault server
  }
}
