.. _upstream_bug_fixes:

=============================================================
 List of upstream |MySQL| bugs fixed in |Percona Server| 5.6
=============================================================

+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`41975` - Support for SSL options not included in mysqlbinlog                       |
|:Launchpad bug: :bug:`1258154`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.15-63.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`71092` - InnoDB FTS introduced new mutex sync level in 5.6.15, broke UNIV_SYNC ... |
|:Launchpad bug: :bug:`1258154`                                                                               |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.15-63.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70417` - rw_lock_x_lock_func_nowait() calls os_thread_get_curr_id() mostly ...     |
|:Launchpad bug: :bug:`1230220`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: 5.6.16                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70490` - Suppression is too strict on some systems                                 |
|:Launchpad bug: :bug:`1205196`                                                                               |
|:Upstream state: No Feedback (checked on 2013-12-19)                                                         |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64556` - Interrupting a query inside InnoDB causes an unrelated warning to be ...  |
|:Launchpad bug: :bug:`1115158`                                                                               |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70500` - Page cleaner should perform LRU flushing regardless of server activity    |
|:Launchpad bug: :bug:`1234562`                                                                               |
|:Upstream state: Open (checked on 2013-12-19)                                                                |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`60682` - deadlock from thd_security_context                                        |
|:Launchpad bug: :bug:`1115048`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70489` - Crash when using AES_ENCRYPT on empty string                              |
|:Launchpad bug: :bug:`1201033`                                                                               |
|:Upstream state: Unsupported                                                                                 |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68481` - InnoDB LRU flushing for MySQL 5.6 needs work                              |
|:Launchpad bug: :bug:`1232406`                                                                               |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70453` - Add hard timeouts to page cleaner flushes                                 |
|:Launchpad bug: :bug:`1232101`                                                                               |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69170` - buf_flush_LRU is lazy                                                     |
|:Launchpad bug: :bug:`1231918`                                                                               |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68555` - thread convoys from log_checkpoint_margin with innodb_buffer_pool_inst... |
|:Launchpad bug: :bug:`1236884`                                                                               |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.13-61.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70228` - Is buf_LRU_free_page() really supposed to make non-zip block sticky at ...|
|:Launchpad bug: :bug:`1220544`                                                                               |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.13-60.6`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69617` - 5.6.12 removed UNIV_SYNC_DEBUG from UNIV_DEBUG                            |
|:Launchpad bug: :bug:`1216815`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-60.6`                                                                             |
|:Upstream fix: 5.6.16                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69258` - does buf_LRU_buf_pool_running_out need to lock buffer pool mutexes        |
|:Launchpad bug: :bug:`1219842`                                                                               |
|:Upstream state: Not a Bug                                                                                   |
|:Fix Released: :rn:`5.6.13-60.6`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70216` - Unnecessary overhead from persistent adaptive hash index latches          |
|:Launchpad bug: :bug:`1218347`                                                                               |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.13-60.6`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62018` - innodb adaptive hash index mutex contention                               |
|:Launchpad bug: :bug:`1216804`                                                                               |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.13-60.6`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69179` - accessing information_schema.partitions causes plans to change            |
|:Launchpad bug: :bug:`1192354`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.13-60.5`                                                                             |
|:Upstream fix: 5.6.14                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`42415` - UPDATE/DELETE with LIMIT clause unsafe for SBL even with ORDER BY PK ...  |
|:Launchpad bug: :bug:`1132194`                                                                               |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.13-60.5`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69639` - mysql failed to build with dtrace Sun D 1.11                              |
|:Launchpad bug: :bug:`1196460`                                                                               |
|:Upstream state: Open (checked on 2013-12-19)                                                                |
|:Fix Released: :rn:`5.6.13-60.5`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69524` - Some tests for table cache variables fail if open files limit is too low  |
|:Launchpad bug: :bug:`1182572`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.12-60.4`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68354` - Server crashes on update/join FEDERATED + local table when only 1 local...|
|:Launchpad bug: :bug:`1182572`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.6.12-60.4`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69856` - mysql_install_db does not function properly in 5.6 for debug builds       |
|:Launchpad bug: :bug:`1179359`                                                                               |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.12-60.4`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`63451` - atomic/x86-gcc.h:make_atomic_cas_body64 potential miscompilation bug      |
|:Launchpad bug: :bug:`878022`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.16                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`70277` - last argument of LOAD DATA ... SET ... statement repeated twice in binlog |
|:Launchpad bug: :bug:`1223196`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.15                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69252` - All the parts.partition_max* tests are broken with MTR --parallel         |
|:Launchpad bug: :bug:`1180481`                                                                               |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69265` - -DBUILD_CONFIG=mysql_release -DWITH_DEBUG=ON fails 4 and skips 27 MTR ... |
|:Launchpad bug: :bug:`1163135`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68714` - Remove literal statement digest values from perfschema tests              |
|:Launchpad bug: :bug:`1157078`                                                                               |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68635` - Doc: Multiple issues with performance_schema_max_statement_classes        |
|:Launchpad bug: :bug:`1157075`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68800` - client doesn't read plugin-dir from my.cnf set by MYSQL_READ_DEFAULT_FILE |
|:Launchpad bug: :bug:`1155859`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69124` - Incorrect truncation of long SET expression in LOAD DATA can cause SQL ...|
|:Launchpad bug: :bug:`1175519`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68970` - fsp_reserve_free_extents switches from small to big tblspace handling ... |
|:Launchpad bug: :bug:`1169494`                                                                               |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68713` - create_duplicate_weedout_tmp_table() leaves key_part_flag uninitialized   |
|:Launchpad bug: :bug:`1157037`                                                                               |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68490` - slave_max_allowed_packet Not Honored on Slave IO Connect                  |
|:Launchpad bug: :bug:`1135097`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68999` - SSL_OP_NO_COMPRESSION not defined                                         |
|:Launchpad bug: :bug:`1183610`                                                                               |
|:Upstream state: No Feedback (checked on 2013-12-19)                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68845` - Unnecessary log_sys->mutex reacquisition in mtr_log_reserve_and_write()   |
|:Launchpad bug: :bug:`1163439`                                                                               |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62578` - mysql client aborts connection on terminal resize                         |
|:Launchpad bug: :bug:`925343`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`49169` - read_view_open_now is inefficient with many concurrent sessions           |
|:Launchpad bug: :bug:`1131187` and :bug:`1131189`                                                            |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`63144` - CREATE TABLE IF NOT EXISTS metadata lock is too restrictive               |
|:Launchpad bug: :bug:`1127008`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.13                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68477` - Suboptimal code in skip_trailing_space()                                  |
|:Launchpad bug: :bug:`1132351`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68476` - Suboptimal code in my_strnxfrm_simple()                                   |
|:Launchpad bug: :bug:`1132350`                                                                               |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67504` - Duplicate error in replication with slave triggers and auto increment     |
|:Launchpad bug: :bug:`1068210`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67974` - Server crashes in add_identifier on concurrent ALTER TABLE and SHOW ENGINE|
|:Launchpad bug: :bug:`1017192`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68045` - security vulnerability CVE-2012-4414                                      |
|:Launchpad bug: :bug:`1049871`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66550` - security vulnerability CVE-2012-4414                                      |
|:Launchpad bug: :bug:`1049871`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67685` - security vulnerability CVE-2012-5611                                      |
|:Launchpad bug: :bug:`1083377`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66237` - Temporary files created by binary log cache are not purged after transa...|
|:Launchpad bug: :bug:`1070856`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54430` - innodb should retry partial reads/writes where errno was 0                |
|:Launchpad bug: :bug:`1079596`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62856` - Check for "stack overrun" doesn't work with gcc-4.6, server crashes       |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61180` - korr/store macros in my_global.h assume the argument to be a char pointer |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61178` - Incorrect implementation of intersect(ulonglong) in non-optimized Bitmap..|
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54127` - mysqld segfaults when built using --with-max-indexes=128                  |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64800` - mysqldump with --include-master-host-port putting quotes around port no.  | 
|:Launchpad bug: :bug:`1013432`                                                                               |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66301` - INSERT ... ON DUPLICATE KEY UPDATE + innodb_autoinc_lock_mode=1 is broken |
|:Launchpad bug: :bug:`1035225`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.12                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`60743` - typo in cmake/dtrace.cmake                                                |
|:Launchpad bug: :bug:`1013455`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: 5.6.13                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64663` - Segfault when adding indexes to InnoDB temporary tables                   |
|:Launchpad bug: :bug:`999147`                                                                                |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64432` - Bug :mysqlbug:`54330` (Broken fast index creation) was never fixed in 5.5 |
|:Launchpad bug: :bug:`939485`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`25007` - memory tables with dynamic rows format                                    |
|:Launchpad bug: N/A                                                                                          |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61595` - mysql-test/include/wait_for_slave_param.inc timeout logic is incorrect    |
|:Launchpad bug: :bug:`800035`                                                                                |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54160` - InnoDB should retry on failed read or write, not immediately panic        |
|:Launchpad bug: :bug:`764395`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68116` - InnoDB monitor may hit an assertion error in buf_page_get_gen in debug ...|
|:Launchpad bug: :bug:`1100178`                                                                               |
|:Upstream state: Analyzing (checked on 2013-12-19)                                                           |
|:Fix Released: :rn:`5.6.10-60.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`65946` - Sid_map::Sid_map calls DBUG which may have unitialized THR_KEY_mysys and..|
|:Launchpad bug: :bug:`1050758`                                                                               |
|:Upstream state: Duplicate/Closed                                                                            |
|:Fix Released: :rn:`5.6.5-60.0`                                                                              |
|:Upstream fix: 5.6.15                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`20001` - Support for temp-tables in INFORMATION_SCHEMA                             |
|:Launchpad bug: N/A                                                                                          |
|:Upstream state: Verified (checked on 2013-12-19)                                                            |
|:Fix Released: :rn:`5.6.5-60.0`                                                                              |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`69146` - Optimization in buf_pool_get_oldest_modification if srv_buf_pool_instances|
|:Launchpad bug: :bug:`1176496`                                                                               |
|:Upstream state: Open (checked on 2013-12-19)                                                                |
|:Fix Released: :rn:`5.6.5-60.0`                                                                              |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
