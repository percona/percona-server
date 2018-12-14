#
# Check if server has support for loading plugins
#
if (`SELECT @@have_dynamic_loading != 'YES'`) {
  --skip auth_pam plugin requires dynamic loading
}

#
# Check if the variable AUTH_PAM is set
#
if (!$AUTH_PAM) {
  --skip Could not find library for auth_pam plugin, see the file plugin.defs
}

#
# Check if --plugin-dir was setup for auth_pam
#
if (`SELECT CONCAT('--plugin-dir=', REPLACE(@@plugin_dir, '\\\\', '/')) != '$AUTH_PAM_OPT/'`) {
  --skip auth_pam plugin requires that --plugin-dir is set to the auth_pam plugin dir
}
enable_query_log;
