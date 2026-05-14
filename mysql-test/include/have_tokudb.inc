#
# Check if server has support for loading plugins
#
if (`SELECT @@have_dynamic_loading != 'YES'`) {
  --skip TokuDB requires dynamic loading
}

if (!$TOKUDB) {
  --skip TokuDB requires the environment variable \$TOKUDB to be set (normally done by mtr)
}

#
# Check if --plugin-dir was setup for TokuDB
#
if (`SELECT CONCAT('--plugin-dir=', REPLACE(@@plugin_dir, '\\\\', '/')) != '$TOKUDB_OPT/'`) {
  --skip TokuDB requires that --plugin-dir is set to the TokuDB plugin dir (either the .opt file does not contain \$TOKUDB_OPT or another plugin is in use)
}
