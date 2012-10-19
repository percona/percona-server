.. rn:: 5.5.27-29.0

===============================
 |Percona Server| 5.5.27-29.0
===============================

Percona is glad to announce the release of |Percona Server| 5.5.27-29.0 on October 11th, 2012 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.27-29.0/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.5.27 <http://dev.mysql.com/doc/refman/5.5/en/news-5-5-27.html>`_, including all the bug fixes in it, |Percona Server| 5.5.27-29.0 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.27-29.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.27-29.0>`_. 

New Features
============

  * |Percona Server| now supports :ref:`changed_page_tracking`. This feature will be used for implementing faster incremental backups that use this information to avoid full data scans.

  * Number of binlog files can be restricted when using |Percona Server| with the new :variable:`max_binlog_files` option.

Bug Fixes
=========

  * Fixed server assertion error related to buffer pool, only visible in debug builds. Bug fixed :bug:`905334` (*Stewart Smith*).

  * Fix for bug :bug:`978036` introduced the :variable:`innodb_sys_stats_root_page` debugging option (only present in debug builds), rendering the previously-existing innodb_sys_stats option its prefix. As such, it became unsettable from command line. Fixed by renaming :variable:`innodb_sys_stats_root_page` to :variable:`innodb_persistent_stats_root_page`. Bug fixed :bug:`1013644` (*Laurynas Biveinis*).

  * Multiple adaptive hash index partitions would cause overly large hash index. Fixed by changing the way partition sizes are calculated initially. Bug fixed :bug:`1018264` (*George Ormond Lorch III*).

  * Postfix would crash on CentOS/RHEL 6.x when using shared dependency (``libmysqlclient.so``). Fixed by building packages with OpenSSL support rather than the bundled YaSSL library. Bug fixed :bug:`1028240` (*Ignacio Nin*).

  * Fixed the issue where LRU dump would hold LRU_list_mutex during the entire dump process. Now the mutex is periodically released in order not to block server while the dump is in progress. Bug fixed :bug:`686534` (*George Ormond Lorch III*).

  * Option :variable:`expire_logs_days` was broken by group_commit patch introduced in |Percona Server| :rn:`5.5.18-23.0`. Bug fixed :bug:`1006214` (*Stewart Smith*).

  * Fixed issue where :variable:`innodb_blocking_lru_restore` did not take an optional bool argument similar to other bool options. Bug fixed :bug:`881001` (*George Ormond Lorch III*).

  * The binlog shouldn't be rotated while it contains XA transactions in the PREPARED state. Bug fixed :bug:`1036040` (*Stewart Smith*).

  * Flashcache support resulted in confusing messages in the error log on |Percona Server| startup even when flashcache was not used. This was fixed by adding new boolean option :variable:`flashcache`. When set to 0 (default), flashcache checks are disabled and when set to 1 checks are enabled. Error message has been made more verbose including error number and system error message as well. Bug fixed :bug:`747032` (*Sergei Glushchenko*).

  * Custom server builds would crash when compiled with a non-default maximum number of indexes per table. Upstream MySQL bugs: `#54127 <http://bugs.mysql.com/bug.php?id=54127>`_, `#61178 <http://bugs.mysql.com/bug.php?id=61178>`_, `#61179 <http://bugs.mysql.com/bug.php?id=61179>`_ and `#61180 <http://bugs.mysql.com/bug.php?id=61180>`_. Bug fixed :bug:`1042517` (*Sergei Glushchenko*).

