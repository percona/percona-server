.. rn:: 5.5.35-33.0

==============================
 |Percona Server| 5.5.35-33.0 
==============================

Percona is glad to announce the release of |Percona Server| 5.5.35-33.0 on December 20th, 2013. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.35-33.0/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.35 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-35.html>`_, including all the bug fixes in it, |Percona Server| 5.5.35-33.0 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.35-33.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.35-33.0>`_. 

New Features
============

 :ref:`threadpool` high priority scheduling is now enabled by default by changing the default :variable:`thread_pool_high_prio_tickets` value from ``0`` to ``4294967295``.

 |Percona Server| now supports :ref:`low_priority_queue_throttling`. This feature should improve :ref:`threadpool` performance under high concurrency in a situation when thread groups are oversubscribed. 

 Introduced new :variable:`thread_pool_high_prio_mode` to provide more fine-grained control over high priority scheduling either globally or per connection in :ref:`threadpool`.

 |Percona Server| has :ref:`extended <extended_mysqlbinlog>` :command:`mysqlbinlog` to provide ``SSL`` and compression support.

 |Percona Server| has reduced the performance overhead of the :ref:`user_stats` feature.

Bugs Fixed
==========

 ``INSTALL PLUGIN`` statement would crash server if :ref:`user_stats` were enabled. Bug fixed :bug:`1011047`.

 Fixed the assertion error caused by a race condition between one thread performing a tablespace delete and another doing a compressed page flush list relocation. Bug fixed :bug:`1227581`.

 Server would crash on shutdown if :ref:`atomic_fio` feature is enabled. Bug fixed :bug:`1255628` (*Jan Lindström*).

 Fixed the compiler errors, caused by merge regression in |Percona Server| :rn:`5.5.33-31.1`. Bug fixed :bug:`1218417`.

 ``mysqldump --innodb-optimize-keys`` was generating incorrect ``CREATE TABLE`` statements for partitioned tables. Bug fixed :bug:`1233841`.

 Default value for :variable:`thread_pool_max_threads` has been changed from ``500`` to ``100 000`` (the maximum supported number of connections), because limiting the total number of threads in the threadpool can result in deadlocks and uneven distribution of worker threads between thread groups in case of stalled connections. Bug fixed :bug:`1258097`.

 ``PURGE CHANGED_PAGE_BITMAPS BEFORE`` statement would delete the changed page data after the specified LSN and up to the start of the next bitmap file. If this data were to be used for fast incremental backups, its absence would cause |Percona XtraBackup| to fall back to the full-scan incremental backup mode. Bug fixed :bug:`1260035` (*Andrew Gaul*).

 Debug server build would crash during |InnoDB| crash recovery if the crash recovery had found transactions that needed cleaning up. Bug fixed :bug:`1247305`.

 Variable :variable:`thread_pool_high_prio_tickets` is now a session variable. Bug fixed :bug:`1166271`.

Other bugs fixed: bug :bug:`1082333`.
