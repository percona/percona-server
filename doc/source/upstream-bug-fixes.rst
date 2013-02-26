.. _upstream_bug_fixes:

=============================================================
 List of upstream |MySQL| bugs fixed in |Percona Server| 5.5
=============================================================

+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68116` - InnoDB monitor may hit an assertion error in buf_page_get_gen in debug ...|
|:Launchpad bug: :bug:`1100178`                                                                               |
|:Upstream state: Analyzing (checked on 2013-02-21)                                                           |
|:Fix Released: :rn:`5.5.29-30.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67504` - Duplicate error in replication with slave triggers and auto increment     |
|:Launchpad bug: :bug:`1068210`                                                                               |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.5.29-30.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67983` - Memory leak on filtered slave                                             |
|:Launchpad bug: :bug:`1042946`                                                                               |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.29-30.0`                                                                             |
|:Upstream fix: 5.5.31                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67974` - Server crashes in add_identifier on concurrent ALTER TABLE and SHOW ENGINE|
|:Launchpad bug: :bug:`1017192`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.29-30.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68045` - security vulnerability CVE-2012-4414                                      |
|:Launchpad bug: :bug:`1049871`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.29-29.4`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66550` - security vulnerability CVE-2012-4414                                      |
|:Launchpad bug: :bug:`1049871`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.28-29.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67685` - security vulnerability CVE-2012-5611                                      |
|:Launchpad bug: :bug:`1083377`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.5.28-29.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66237` - Temporary files created by binary log cache are not purged after transa...|
|:Launchpad bug: :bug:`1070856`                                                                               |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.5.28-29.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67606` - MySQL crashes with segmentation fault when disk quota is reached          |
|:Launchpad bug: :bug:`1079596`                                                                               |
|:Upstream state: Duplicate                                                                                   |
|:Fix Released: :rn:`5.5.28-29.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67737` - mysqldump test sometimes fails due to mixing stdout and stderr            |
|:Launchpad bug: :bug:`959198`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.28-29.2`                                                                             |
|:Upstream fix: 5.5.29                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62856` - Check for "stack overrun" doesn't work with gcc-4.6, server crashes       |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.5.28-29.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61180` - korr/store macros in my_global.h assume the argument to be a char pointer |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.5.27-29.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61179` - Unoptimized versions of korr/store macros in my_global.h are used on ...  |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.5.27-29.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61178` - Incorrect implementation of intersect(ulonglong) in non-optimized Bitmap..|
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.5.27-29.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54127` - mysqld segfaults when built using --with-max-indexes=128                  |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.5.27-29.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64800` - mysqldump with --include-master-host-port putting quotes around port no.  | 
|:Launchpad bug: :bug:`1013432`                                                                               |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.5.27-28.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66301` - INSERT ... ON DUPLICATE KEY UPDATE + innodb_autoinc_lock_mode=1 is broken |
|:Launchpad bug: :bug:`1035225`                                                                               |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.5.27-28.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`60743` - typo in cmake/dtrace.cmake                                                |
|:Launchpad bug: :bug:`1013455`                                                                               |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.5.25a-27.1`                                                                            |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64663` - Segfault when adding indexes to InnoDB temporary tables                   |
|:Launchpad bug: :bug:`999147`                                                                                |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.5.24-26.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64624` - Mysql is crashing during replication                                      |
|:Launchpad bug: :bug:`915814`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.24-26.0`                                                                             |
|:Upstream fix: 5.5.26                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64160` - page size 1024 but the only supported page size in this release is=16384  |
|:Launchpad bug: :bug:`966844`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.21-25.1`                                                                             |
|:Upstream fix: 5.5.22                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64432` - Bug :mysqlbug:`54330` (Broken fast index creation) was never fixed in 5.5 |
|:Launchpad bug: :bug:`939485`                                                                                |
|:Upstream state: Documenting (checked on 2013-02-21)                                                         |
|:Fix Released: :rn:`5.5.21-25.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`49336` - mysqlbinlog does not accept input from stdin when stdin is a pipe         |
|:Launchpad bug: :bug:`933969`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.21-25.0`                                                                             |
|:Upstream fix: 5.5.28                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62557` - SHOW SLAVE STATUS gives wrong output with master-master and using SET...  |
|:Launchpad bug: :bug:`860910`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.17-22.1`                                                                             |
|:Upstream fix: 5.5.28                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`45702` - Impossible to specify myisam_sort_buffer > 4GB on 64 bit machines         |
|:Launchpad bug: :bug:`878404`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.17-22.1`                                                                             |
|:Upstream fix: 5.5.22                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62516` - Fast index creation does not update index statistics                      |
|:Launchpad bug: :bug:`857590`                                                                                |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.5.16-22.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`51196` - Slave SQL: Got an error writing communication packets, Error_code: 1160   |
|:Launchpad bug: :bug:`813587`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.14-20.5`                                                                             |
|:Upstream fix: 5.5.21                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61595` - mysql-test/include/wait_for_slave_param.inc timeout logic is incorrect    |
|:Launchpad bug: :bug:`800035`                                                                                |
|:Upstream state: Verified (checked on 2013-02-21)                                                            |
|:Fix Released: :rn:`5.5.13-20.4`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`60788` - InnoDB crashes with an assertion failure when receiving a signal on pwrite|
|:Launchpad bug: :bug:`764395`                                                                                |
|:Upstream state: Duplicate                                                                                   |
|:Fix Released: :rn:`5.5.11-20.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`51325` - Dropping an empty innodb table takes a long time with large buffer pool   |
|:Launchpad bug: none                                                                                         |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.10-20.1`                                                                             |
|:Upstream fix: 5.5.20                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`56433` - Auto-extension of InnoDB files                                            |
|:Launchpad bug: none                                                                                         |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.5.10-20.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
