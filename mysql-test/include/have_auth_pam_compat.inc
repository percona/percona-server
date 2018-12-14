#
# Check if server has support for loading plugins
#
if (`SELECT @@have_dynamic_loading != 'YES'`) {
  --skip auth_pam_compat plugin requires dynamic loading
}

#
# Check if the variable AUTH_PAM_COMPAT is set
#
if (!$AUTH_PAM_COMPAT) {
  --skip Could not find library for auth_pam_compat plugin, see the file plugin.defs
}

#
# Check if --plugin-dir was setup for auth_pam_compat
#
if (`SELECT CONCAT('--plugin-dir=', REPLACE(@@plugin_dir, '\\\\', '/')) != '$AUTH_PAM_COMPAT_OPT/'`) {
  --skip auth_pam_compat plugin requires that --plugin-dir is set to the auth_pam_compat plugin dir
}
enable_query_log;
