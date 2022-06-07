# suite/rocksdb/include/i_s_basic.inc
#
# SUMMARY
#
#    Basic MyRocks information_schema test that covers issues and sequences that
#    are common to all of the MyRocks information_schema plugins.
#
# USAGE
#
#    let $i_s_plugin=<plugin_table_name>

if (!$i_s_plugin)
{
  --die i_s_plugin not specified to suite/rocksdb/include/i_s_basic.inc
}

--eval show create table information_schema.$i_s_plugin

#
# Test that MYR-221 is addressed and that proper error is returned instead of
# a crash when the SE is unloaded but the I_S plugin is still present
#
uninstall plugin rocksdb;

--error ER_PLUGIN_IS_NOT_LOADED
--eval select * from information_schema.$i_s_plugin

# Restart the server to get the SE plugin loaded again, currently MyRocks does
# not do well with unload/load sequences.
--source include/restart_mysqld.inc
