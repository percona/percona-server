.. rn:: 5.5.27-28.0

===============================
 |Percona Server| 5.5.27-28.0
===============================

Percona is glad to announce the release of |Percona Server| 5.5.27-28.0 on August 23rd, 2012 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.27-28.0/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.5.27 <http://dev.mysql.com/doc/refman/5.5/en/news-5-5-27.html>`_, including all the bug fixes in it, |Percona Server| 5.5.27-28.0 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.27-28.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.27-28.0>`_. 

Bug fix for bug :bug:`1007268` has been targeted for the next |Percona Server| release. Workaround for this bug exists and it's setting up the :variable:`innodb_lazy_drop_table` to 1.

New Features
============

  * |Percona Server| supports tunable buffer size for fast index creation in |InnoDB|. This value was calculated based on the merge block size (which was hardcoded to 1 MB) and the minimum index record size. By adding the session variable :variable:`innodb_merge_sort_block_size` block size that is used in the merge sort can now be adjusted for better performance.

  * |Percona Server| has implemented ability to have a MySQL :ref:`psaas_utility_user` who has system access to do administrative tasks but limited access to user schema. This feature is especially useful to those operating MySQL As A Service.

  * New :ref:`expanded_option_modifiers` have been added to allow access control to system variables.

  * New table :table:`INNODB_UNDO_LOGS` has been added to allow access to undo segment information. Each row represents an individual undo segment and contains information about which rollback segment the undo segment is currently owned by, which transaction is currently using the undo segment, and other size and type information for the undo segment. This information is 'live' and calculated for each query of the table.

Bug Fixes
=========

  * Fixed incorrect merge of MySQL bug `#61188 <http://bugs.mysql.com/bug.php?id=61188>`_ fix which caused server to freeze with "has waited at buf0buf.c line 2529 for XXX seconds the semaphore" errors. This regression was introduced in |Percona Server| 5.5.23-25.3. Bug fixed :bug:`1026926` (*Stewart Smith*).

  * Fixed regression introduced in |Percona Server| 5.5.23-25.3 when merging the upstream fix for MySQL bug `#64284 <http://bugs.mysql.com/bug.php?id=64284>`_. Bug fixed :bug:`1015109` (*Stewart Smith*).

  * Fixed the upstream MySQL bug `#66301 <http://bugs.mysql.com/bug.php?id=66301>`_. Concurrent INSERT ... ON DUPLICATE KEY UPDATE statements on a table with an AUTO_INCREMENT column could result in spurious duplicate key errors (and, as a result, lost data due to some rows being updated rather than inserted) with the default value of innodb_autoinc_lock_mode=1. Bug fixed :bug:`1035225` (*Alexey Kopytov*).

  * Removed error log warnings that occured after enabling :variable:`innodb_use_sys_stats_table` and before ANALYZE TABLE is run for each table. Bug fixed :bug:`890623` (*Alexey Kopytov*).

  * Removed the unneeded lrusort.py script. The server now does this sorting automatically and has done for some time. Bug fixed :bug:`882653` (*Stewart Smith*).

  * Fixed the malformed CHANGE MASTER query in the output of ``mysqldump`` with :option:`--include-master-host-port` option. Bug fixed :bug:`1013432` (*Stewart Smith*).

