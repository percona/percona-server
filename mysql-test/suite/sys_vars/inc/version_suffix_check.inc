# Check that @@version, @@version_with_update, VERSION() and
# VERSION_WITH_UPDATE() all end with the current @@version_suffix.
#
# Usage:
#   SET @version_base = <@@version with an empty @@version_suffix>;
#   SET @version_with_update_base =
#     <@@version_with_update with an empty @@version_suffix>;
#   --source suite/sys_vars/inc/version_suffix_check.inc

--let $assert_text= @@version is the base version followed by @@version_suffix
--let $assert_cond= STRCMP(@@global.version, CONCAT(@version_base, IFNULL(@@global.version_suffix, ""))) = 0
--source include/assert.inc

--let $assert_text= @@version_with_update is the base version followed by @@version_suffix
--let $assert_cond= STRCMP(@@global.version_with_update, CONCAT(@version_with_update_base, IFNULL(@@global.version_suffix, ""))) = 0
--source include/assert.inc

--source suite/sys_vars/inc/version_with_update_check.inc

--let $assert_text= VERSION() is equal to @@version
--let $assert_cond= STRCMP(VERSION(), @@global.version) = 0
--source include/assert.inc

--let $assert_text= VERSION_WITH_UPDATE() is equal to @@version_with_update
--let $assert_cond= STRCMP(VERSION_WITH_UPDATE(), @@global.version_with_update) = 0
--source include/assert.inc
