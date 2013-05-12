.. _upstream_bug_fixes:

=============================================================
 List of upstream |MySQL| bugs fixed in |Percona Server| 5.6
=============================================================

+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68477` - Suboptimal code in skip_trailing_space()                                  |
|:Launchpad bug: :bug:`1132351`                                                                               |
|:Upstream state: Verified (checked on 2013-03-05)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68476` - Suboptimal code in my_strnxfrm_simple()                                   |
|:Launchpad bug: :bug:`1132350`                                                                               |
|:Upstream state: Verified (checked on 2013-03-05)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67504` - Duplicate error in replication with slave triggers and auto increment     |
|:Launchpad bug: :bug:`1068210`                                                                               |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67974` - Server crashes in add_identifier on concurrent ALTER TABLE and SHOW ENGINE|
|:Launchpad bug: :bug:`1017192`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
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
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67606` - MySQL crashes with segmentation fault when disk quota is reached          |
|:Launchpad bug: :bug:`1079596`                                                                               |
|:Upstream state: Duplicate                                                                                   |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62856` - Check for "stack overrun" doesn't work with gcc-4.6, server crashes       |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61180` - korr/store macros in my_global.h assume the argument to be a char pointer |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61179` - Unoptimized versions of korr/store macros in my_global.h are used on ...  |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61178` - Incorrect implementation of intersect(ulonglong) in non-optimized Bitmap..|
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54127` - mysqld segfaults when built using --with-max-indexes=128                  |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64800` - mysqldump with --include-master-host-port putting quotes around port no.  | 
|:Launchpad bug: :bug:`1013432`                                                                               |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66301` - INSERT ... ON DUPLICATE KEY UPDATE + innodb_autoinc_lock_mode=1 is broken |
|:Launchpad bug: :bug:`1035225`                                                                               |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`60743` - typo in cmake/dtrace.cmake                                                |
|:Launchpad bug: :bug:`1013455`                                                                               |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64663` - Segfault when adding indexes to InnoDB temporary tables                   |
|:Launchpad bug: :bug:`999147`                                                                                |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64432` - Bug :mysqlbug:`54330` (Broken fast index creation) was never fixed in 5.5 |
|:Launchpad bug: :bug:`939485`                                                                                |
|:Upstream state: Documenting (checked on 2013-02-21)                                                         |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61595` - mysql-test/include/wait_for_slave_param.inc timeout logic is incorrect    |
|:Launchpad bug: :bug:`800035`                                                                                |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`60788` - InnoDB crashes with an assertion failure when receiving a signal on pwrite|
|:Launchpad bug: :bug:`764395`                                                                                |
|:Upstream state: Duplicate                                                                                   |
|:Fix Released: :rn:`5.6.11-60.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68116` - InnoDB monitor may hit an assertion error in buf_page_get_gen in debug ...|
|:Launchpad bug: :bug:`1100178`                                                                               |
|:Upstream state: Analyzing (checked on 2013-02-21)                                                           |
|:Fix Released: :rn:`5.6.10-60.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`65946` - Sid_map::Sid_map calls DBUG which may have unitialized THR_KEY_mysys and..|
|:Launchpad bug: :bug:`1050758`                                                                               |
|:Upstream state: Duplicate                                                                                   |
|:Fix Released: :rn:`5.6.5-60.0`                                                                              |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
