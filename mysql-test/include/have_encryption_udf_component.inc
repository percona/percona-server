--disable_query_log

#
# Check if the variable ENCRYPTION_UDF_COMPONENT is set
#
if (!$ENCRYPTION_UDF_COMPONENT) {
   --skip component requires the environment variable \$ENCRYPTION_UDF_COMPONENT to be set (normally done by mtr), see the file plugin.defs
}

#
## Check if --plugin-dir was setup for component_encryption_udf
#
if (`SELECT CONCAT('--plugin-dir=', REPLACE(@@plugin_dir, '\\\\', '/')) != '$ENCRYPTION_UDF_COMPONENT_OPT/'`) {
    --skip component_encryption_udf requires that --plugin-dir is set to the component_encryption_udf dir (the .opt file does not contain \$ENCRYPTION_UDF_COMPONENT_OPT)
}

--enable_query_log
