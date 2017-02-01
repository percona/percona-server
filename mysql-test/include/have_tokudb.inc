#
# Check if server has support for loading plugins
#
if (`SELECT @@have_dynamic_loading != 'YES'`) {
  --skip TokuDB requires dynamic loading
}

if (!$TOKUDB) {
  --skip TokuDB requires the environment variable \$TOKUDB to be set (normally done by mtr)
}
