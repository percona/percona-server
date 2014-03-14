disable_query_log;
#
# Check if server has support for loading plugins
#
if (`SELECT @@have_dynamic_loading != 'YES'`) {
  --skip Socket auth plugin requires dynamic loading
}

#
# Check if the variable SOCKET_AUTH is set
#
if (!$SOCKET_AUTH) {
  --skip SOCKET_AUTH plugin requires the environment variable \$SOCKET_AUTH to be set (normally done by mtr)
}

#
# Check if --plugin-dir was setup for null_audit db
#
if (`SELECT CONCAT('--plugin-dir=', REPLACE(@@plugin_dir, '\\\\', '/')) != '$SOCKET_AUTH_OPT/'`) {
  --skip socket_auth plugin requires that --plugin-dir is set to the socket_auth plugin dir (either the .opt file does not contain \$SOCKET_AUTH_OPT or another plugin is in use)
}
enable_query_log;
