.. rn:: 5.1.65-14.0

=============================
 |Percona Server| 5.1.65-14.0
=============================

Percona is glad to announce the release of |Percona Server| 5.1.65-14.0 on September 3rd, 2012 (Downloads are available from `Percona Server 5.1.65-14.0 downloads <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.65-14.0/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.1.65 <http://dev.mysql.com/doc/refman/5.1/en/news-5-1-65.html>`_, including all the bug fixes in it, |Percona Server| 5.1.65-14.0 is now the current stable release in the 5.1 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.1.65-14.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.1.65-14.0>`_.

Features
========
  
  |Percona Server| now supports :ref:`changed_page_tracking`. This feature will be used for implementing faster incremental backups that use this information to avoid full data scans.  

  New table :table:`INNODB_CHANGED_PAGES` has been implemented. This table contains a list of modified pages from the modified page bitmap files produced by the log tracking thread.

  HandlerSocket has been upgraded to version 1.1.0.
 
Bug Fixes
=========

 Loading LRU dump was preventing shutdown. Bug fixed :bug:`712055` (*George Ormond Lorch III*).

 The kill idle transactions feature in |XtraDB| (if enabled) could sometimes cause the server to crash. Bug Fixed: :bug:`871722` (*Stewart Smith*)

 Fixed server assertion error related to buffer pool, only visible in debug builds. Bug fixed :bug:`905334` (*Laurynas Biveinis*).
 
 Querying I_S.GLOBAL_TEMPORARY_TABLES or TEMPORARY_TABLES would crash threads working with temporary tables. Bug fixed :bug:`951588` (*Laurynas Biveinis*).

 A crash could leave behind an InnoDB temporary table with temporary indexes resulting in an unbootable server. Bug fixed :bug:`999147` (*Laurynas Biveinis*).

 Fixed the upstream MySQL bug `#66301 <http://bugs.mysql.com/bug.php?id=66301>`_. Concurrent INSERT ... ON DUPLICATE KEY UPDATE statements on a table with an AUTO_INCREMENT column could result in spurious duplicate key errors (and, as a result, lost data due to some rows being updated rather than inserted) with the default value of innodb_autoinc_lock_mode=1. Bug fixed :bug:`1035225` (*Alexey Kopytov*)

 Since the output file is simply overwritten when dumping the LRU list, we could end up with a partially written dump file in case of a crash, or when making a backup copy of it. Safer approach has been implemented. It now dumps to a temporary file first, and then renames it to the actual dump file. Bug fixed :bug:`686392` (*George Ormond Lorch III*).

 Fixed the issue where LRU dump would hold LRU_list_mutex during the entire dump process. Now the mutex is periodically released in order not to block server while the dump is in progress. Bug fixed :bug:`686534` (*George Ormond Lorch III*).

 Removed error log warnings that occured after enabling :variable:`innodb_use_sys_stats_table` and before ANALYZE TABLE is run for each table. Bug fixed :bug:`890623` (*Alexey Kopytov*).

 A Server acting as a replication slave with the query cache enabled could crash with glibc detected memory corruption. Bug fixed :bug:`915814` (*George Ormond Lorch III*). 

 If the tablespace has been created with MySQL 5.0 or older, importing that table could crash |Percona Server| in some cases. Bug fixed :bug:`1000221` (*Alexey Kopytov*).

 Error log messages are now more verbose for LRU dump. Bug fixed :bug:`713481` (*George Ormond Lorch III*). 

 Fixed issue where :variable:`innodb_blocking_lru_restore` did not take an optional bool argument similar to other bool options. Bug fixed :bug:`881001` (*George Ormond Lorch III*).

 Removed the unneeded lrusort.py script. The server now does this sorting automatically and has done for some time. Bug fixed :bug:`882653` (*Stewart Smith*).

 Server started with :option:`skip-innodb` crashes on `SELECT * FROM INNODB_TABLE_STATS` or `INNODB_INDEX_STATS`. Bug fixed :bug:`896439` (*Stewart Smith*).

 Removed the INFORMATION_SCHEMA table INNODB_PATCHES (actually XTRADB_ENHANCEMENTS) as it was out of date and isn't in 5.5 or later either. Bug fixed :bug:`1009997` (*Stewart Smith*).

