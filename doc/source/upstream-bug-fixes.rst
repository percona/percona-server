.. _upstream_bug_fixes:

=============================================================
 List of upstream |MySQL| bugs fixed in |Percona Server| 5.1
=============================================================

+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`68045` - security vulnerability CVE-2012-5611                                      |
|:Launchpad bug: :bug:`1083377`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.1.67-14.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67685` - security vulnerability CVE-2012-5611                                      |
|:Launchpad bug: :bug:`1083377`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.1.66-14.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66550` - security vulnerability CVE-2012-4414                                      |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: N/A                                                                                         |
|:Fix Released: :rn:`5.1.66-14.2`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61180` - korr/store macros in my_global.h assume the argument to be a char ...     |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Verified (checked on 2013-01-18)                                                            |
|:Fix Released: :rn:`5.1.66-14.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61179` - Unoptimized versions of korr/store macros in my_global.h are used on...   |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Verified (checked on 2013-01-18)                                                            |
|:Fix Released: :rn:`5.1.66-14.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61178` - Incorrect implementation of intersect(ulonglong) in non-optimized Bitmap..|
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Verified (checked on 2013-01-18)                                                            |
|:Fix Released: :rn:`5.1.66-14.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`54127` - mysqld segfaults when built using --with-max-indexes=128                  |
|:Launchpad bug: :bug:`1042517`                                                                               |
|:Upstream state: Verified (checked on 2013-01-18)                                                            |
|:Fix Released: :rn:`5.1.66-14.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`67177` - MySQL 5.1 is incompatibile with automake 1.12                             |
|:Launchpad bug: :bug:`1064953`                                                                               |
|:Upstream state: Verified (checked on 2013-01-18)                                                            |
|:Fix Released: :rn:`5.1.66-14.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62856` - Check for "stack overrun" doesn't work with gcc-4.6, server crashes       |
|:Launchpad bug: :bug:`902472`                                                                                |
|:Upstream state: Verified (checked on 2013-01-18)                                                            |
|:Fix Released: :rn:`5.1.66-14.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`61509` - mysqld (5.1.57) segfaults with gcc 4.6                                    |
|:Launchpad bug: :bug:`902471`                                                                                |
|:Upstream state: Verified (checked on 2013-01-18)                                                            |
|:Fix Released: :rn:`5.1.66-14.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`66301` - INSERT ... ON DUPLICATE KEY UPDATE + innodb_autoinc_lock_mode=1 is broken |
|:Launchpad bug: :bug:`1035225`                                                                               |
|:Upstream state: Verified (checked on 2013-01-18)                                                            |
|:Fix Released: :rn:`5.1.65-14.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64469` - Deadlock or crash on concurrent TRUNCATE TABLE and SELECT * FROM I_S      |
|:Launchpad bug: :bug:`903617`                                                                                |
|:Upstream state: Analyzing (checked on 2013-01-18)                                                           |
|:Fix Released: :rn:`5.1.62-13.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64128` - InnoDB error in server log of innodb_bug34300                             |
|:Launchpad bug: :bug:`937859`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.1.62-13.3`                                                                             |
|:Upstream fix: 5.1.63                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`49336` - mysqlbinlog does not accept input from stdin when stdin is a pipe         |
|:Launchpad bug: :bug:`933969`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.1.62-13.3`                                                                             |
|:Upstream fix: 5.1.66                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`64127` - MTR --warnings option misses some of InnoDB errors and warnings           |
|:Launchpad bug: :bug:`937859`                                                                                |
|:Upstream state: Verified (checked on 2013-01-18)                                                            |
|:Fix Released: :rn:`5.1.62-13.3`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62557` - SHOW SLAVE STATUS gives wrong output with master-master and using SET...  |
|:Launchpad bug: :bug:`860910`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.1.60-13.1`                                                                             |
|:Upstream fix: 5.1.66                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`45702` - Impossible to specify myisam_sort_buffer > 4GB on 64 bit machines         |
|:Launchpad bug: :bug:`878404`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.1.60-13.1`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`53761` - RANGE estimation for matched rows may be 200 times different              |
|:Launchpad bug: :bug:`832528`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.1.59-13.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`62516` - Fast index creation does not update index statistics                      |
|:Launchpad bug: :bug:`857590`                                                                                |
|:Upstream state: Verified (checked on 2013-01-18)                                                            |
|:Fix Released: :rn:`5.1.59-13.0`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`51196` - Slave SQL: Got an error writing communication packets, Error_code: 1160   |
|:Launchpad bug: :bug:`813587`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.1.58-12.9`                                                                             |
|:Upstream fix: 5.1.62                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`56433` - Auto-extension of InnoDB files                                            |
|:Launchpad bug: none                                                                                         |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.1.56-12.7`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`51325` - Dropping an empty innodb table takes a long time with large buffer pool   |
|:Launchpad bug: none                                                                                         |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.1.56-12.7`                                                                             |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`47337` - innochecksum not built for --with-plugin-innodb_plugin --without-plugin...|
|:Launchpad bug: :bug:`671764`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.1.53-12.4`                                                                             | 
|:Upstream fix: 5.1.60                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`48883` - Test "innodb_information_schema" takes fewer locks than expected          |
|:Launchpad bug: :bug:`677407`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.1.53-11.7`                                                                             |
|:Upstream fix: 5.1.52sp1                                                                                     |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`38551` - RBR/MBR + Query Cache + "invalidating query cache entries (table)"        |
|:Launchpad bug: :bug:`609027`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.1.49-rel12.0`                                                                          |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`55032` - Query cache sometime insert queries to cache, but doesn't find ...        |
|:Launchpad bug: none                                                                                         |
|:Upstream state: Verified (checked on 2013-01-18)                                                            |
|:Fix Released: :rn:`5.1.47-rel11.2`                                                                          |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`53371` - Parent directory entry ("..") can be abused to bypass table level grants. |
|:Launchpad bug: none                                                                                         |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`1.0.6-rel10.2`                                                                           |
|:Upstream fix: 5.1.51                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`53237` - mysql_list_fields/COM_FIELD_LIST stack smashing - remote execution...     |
|:Launchpad bug: none                                                                                         |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`1.0.6-rel10.2`                                                                           |
|:Upstream fix: 5.1.51                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`50974` - Server keeps receiving big (> max_allowed_packet) packets indefinitely    |
|:Launchpad bug: none                                                                                         |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`1.0.6-rel10.2`                                                                           |
|:Upstream fix: 5.1.51                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`53237` - mysql_list_fields/COM_FIELD_LIST stack smashing - remote execution ...    |
|:Launchpad bug: :bug:`580324`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`5.1.47-rel11.0`                                                                          |
|:Upstream fix: 5.1.49                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`47621` - MySQL and InnoDB data dictionaries will become out of sync when renaming..|
|:Launchpad bug: :bug:`488315`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`1.0.6-9`                                                                                 |
|:Upstream fix: 5.1.47                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`47622` - the new index is added before the existing ones in MySQL, but after one...|
|:Launchpad bug: :bug:`488315`                                                                                |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`1.0.6-9`                                                                                 |
|:Upstream fix: 5.1.51                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`39793` - Foreign keys not constructed when column has a '#' in a comment or ...    |
|:Launchpad bug: none                                                                                         |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`1.0.3-7`                                                                                 |
|:Upstream fix: 5.1.47                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`44140` - Insert buffer operation may destroy the page during its recovery process  |
|:Launchpad bug: none                                                                                         |
|:Upstream state: Open                                                                                        |
|:Fix Released: :rn:`1.0.3-7`                                                                                 |
|:Upstream fix: N/A                                                                                           |
+-------------------------------------------------------------------------------------------------------------+
|:Upstream bug: :mysqlbug:`42101` - Race condition in innodb_commit_concurrency                               |
|:Launchpad bug: none                                                                                         |
|:Upstream state: Closed                                                                                      |
|:Fix Released: :rn:`1.0.3-7`                                                                                 |
|:Upstream fix: 5.1.47                                                                                        |
+-------------------------------------------------------------------------------------------------------------+
