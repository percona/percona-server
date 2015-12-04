.. rn:: 5.6.15-63.0

==============================
 |Percona Server| 5.6.15-63.0
==============================

Percona is glad to announce the release of |Percona Server| 5.6.15-63.0 on December 19th, 2013 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.15-63.0/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.6.15 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-15.html>`_, including all the bug fixes in it, |Percona Server| 5.6.15-63.0 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.15-63.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.15-63.0>`_.

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

 :variable:`innodb_log_checksum_algorithm` did not have any effect when set from :file:`my.cnf` or mysqld command line, it would have effect only if set dynamically. Bug fixed :bug:`1248505`. 

 Server would crash on shutdown if :ref:`atomic_fio` feature is enabled. Bug fixed :bug:`1255628` (*Jan Lindström*).

 |Percona Server| would crash when data was select from :table:`XTRADB_RSEG` table when |InnoDB| system table space was initialized with lower then default number of rollback segments. Bug fixed :bug:`1260259`.

 Certain types of workloads (large result sets, blobs, slow clients) can have longer waits on network I/O (socket reads and writes). Whenever server waits, this should be communicated to the :ref:`threadpool`, so it can start new query by either waking a waiting thread or sometimes creating a new one. Ported |MariaDB| patch `MDEV-156 <https://mariadb.atlassian.net/browse/MDEV-156>`_, bug fixed :bug:`1159743`.

 ``mysqldump --innodb-optimize-keys`` was generating incorrect ``CREATE TABLE`` statements for partitioned tables. Bug fixed :bug:`1233841`.

 Fixed errors when server was compiled with ``-DWITH_LIBWRAP=ON`` option. Bug fixed :bug:`1240442`.

 If :variable:`innobase_atomic_writes` was used on separate undo files that do not exists would lead to operating system error. Bug fixed :bug:`1255638` (*Jan Lindström*).

 Default value for :variable:`thread_pool_max_threads` has been changed from ``500`` to ``100 000`` (the maximum supported number of connections), because limiting the total number of threads in the :ref:`threadpool` can result in deadlocks and uneven distribution of worker threads between thread groups in case of stalled connections. Bug fixed :bug:`1258097`.

 ``PURGE CHANGED_PAGE_BITMAPS BEFORE`` statement would delete the changed page data after the specified LSN and up to the start of the next bitmap file. If this data were to be used for fast incremental backups, its absence would cause |Percona XtraBackup| to fall back to the full-scan incremental backup mode. Bug fixed :bug:`1260035` (*Andrew Gaul*).

 Server performance could degrade under heavy load or it could deadlock on shutdown while performing purge. Bug fixed :bug:`1236696`.

 Server could crash under heavy load if |InnoDB| compression was used. Bug fixed :bug:`1240371`.

 Redo log checksum mismatches would be diagnosed using the data page checksum algorithm setting instead of redo log checksum algorithm one. Bug fixed :bug:`1250148`.

Other bugs fixed: bug :bug:`1082333`, bug :bug:`1260945`, bug :bug:`1248046`, bug :bug:`1243067`, bug :bug:`1238563`, bug :bug:`1258154` (upstream bug :mysqlbug:`71092`), bug :bug:`1242748`, bug :bug:`1239062`, bug :bug:`1200788`, bug :bug:`1193319`, bug :bug:`1240044`.
