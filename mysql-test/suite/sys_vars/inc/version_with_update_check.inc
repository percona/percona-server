# Check that @@version_with_update without its fourth (update) component
# is equal to @@version, so it has the same base version and exactly one
# copy of @@version_suffix.
#
# Usage:
#   --source suite/sys_vars/inc/version_with_update_check.inc

# Done in SQL, because mysqltest would expand $1 in --let.
--disable_query_log
SET @version_with_update_no_update = REGEXP_REPLACE(@@global.version_with_update, '^([0-9]+[.][0-9]+[.][0-9]+)[.][0-9]+', '$1');
--enable_query_log

--let $assert_text= @@version_with_update without the update component is equal to @@version
--let $assert_cond= STRCMP(@version_with_update_no_update, @@global.version) = 0
--source include/assert.inc
