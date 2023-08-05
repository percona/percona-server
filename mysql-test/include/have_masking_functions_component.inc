--disable_query_log

#
# Check if the variable MASKING_FUNCTIONS_COMPONENT is set
#
if (!$MASKING_FUNCTIONS_COMPONENT) {
   --skip component requires the environment variable \$MASKING_FUNCTIONS_COMPONENT to be set (normally done by mtr), see the file plugin.defs
}

#
## Check if --plugin-dir was setup for component_encryption_udf
#
if (`SELECT CONCAT('--plugin-dir=', REPLACE(@@plugin_dir, '\\\\', '/')) != '$MASKING_FUNCTIONS_COMPONENT_OPT/'`) {
    --skip component_masking_functions requires that --plugin-dir is set to the component_masking_functions dir (the .opt file does not contain \$MASKING_FUNCTIONS_COMPONENT_OPT)
}

--enable_query_log
