#
# Script tests if TokuDB _should_ be present.
# Sets $HAS_TOKUDB to either 0 or 1
#
--let $HAS_TOKUDB=0
#
# Check if server has support for loading plugins
#
if (`SELECT @@have_dynamic_loading = 'YES'`) {
  if ($TOKUDB) {
    #
    # Check if --plugin-dir was setup for TokuDB
    #
    if (`SELECT CONCAT('--plugin-dir=', REPLACE(@@plugin_dir, '\\\\', '/')) = '$TOKUDB_OPT/'`) {
      --let $HAS_TOKUDB=1
    }
  }
}
