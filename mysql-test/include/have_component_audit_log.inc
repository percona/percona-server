disable_query_log;

#
# Check if the variable AUDIT_LOG_COMPONENT is set
#
if (!$AUDIT_LOG_COMPONENT) {
   --skip test requires the environment variable \$AUDIT_LOG_COMPONENT to be set (normally done by mtr), see the file plugin.defs
}

enable_query_log;
