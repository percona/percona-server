# PS-12000: detect a Debug build by the existence of the debug system
# variable, as mysql-test-run.pl does. VERSION() is not used, because
# --version-suffix replaces the "-debug" build suffix.
let $have_debug = `SELECT COUNT(*) FROM performance_schema.global_variables WHERE VARIABLE_NAME = 'debug'`;
if ($have_debug)
{
  --skip Test requires binaries to be compiled in release mode
}
