.. rn:: 5.6.26-74.0

==============================
 |Percona Server| 5.6.26-74.0 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.26-74.0 on September 15th, 2015 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.26-74.0/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.26 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-26.html>`_, including all the bug fixes in it, |Percona Server| 5.6.26-74.0 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.26-74.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.26-74.0>`_.

New Features
============

 |TokuDB| storage engine `source <https://github.com/percona/tokudb-engine.git>`_ has been merged into the |Percona Server| code. |TokuDB| storage engine :variable:`tokudb_version` variable now has the same value as the |Percona Server| version variable :variable:`version`. 

 |TokuDB| Hot Backup has been renamed to |Percona TokuBackup| and it is now `open source <https://github.com/percona/Percona-TokuBackup.git>`_. Source `code <https://github.com/percona/tokudb-backup-plugin>`_ has been integrated into |Percona Server| code as a git submodule. |TokuDB| Hot Backup `plugin source <https://github.com/percona/tokudb-backup-plugin.git>`_ has been merged into |Percona Server| code.

 *Tokutek Fractal Tree* has been renamed to |Percona FT| and its `source code <https://github.com/percona/PerconaFT.git>`_ has been integrated into |Percona Server| code as a git submodule. 

 |TokuDB| `tests <https://github.com/percona/tokudb-percona-server-5.6>`_ for |Percona Server| 5.6 have been merged into |Percona Server| 5.6 code.

 Google `SNAPPY <http://google.github.io/snappy/>`_ compression/decompression algorithm is now available as :ref:`TokuDB Compression <tokudb_compression>` table format.

 |Percona Server| now supports changing the :variable:`server_id` variable :ref:`per session <per_session_server-id>`, by implementing the new :variable:`pseudo_server_id` variable. This feature is also fixing upstream bug :mysqlbug:`35125`.

 |Percona Server| has temporarily disabled savepoints in triggers and stored functions. The reason is that even having fixed bug :bug:`1438990` and bug :bug:`1464468` we have found more cases where savepoints in triggers break binary logging and replication, resulting in server crashes and broken slaves. This feature will be disabled until the above issues are properly resolved.

 ``LOCK TABLES FOR BACKUP`` now :ref:`flushes the current binary log coordinates <backup-safe_binlog_information>` to |InnoDB|. Thus, under active ``LOCK TABLES FOR BACKUP``, the binary log coordinates in |InnoDB| are consistent with its redo log and any non-transactional updates (as the latter are blocked by ``LOCK BINLOG FOR BACKUP``). It is planned that this change will enable |Percona XtraBackup| to avoid issuing the more invasive ``LOCK BINLOG FOR BACKUP`` command under some circumstances. 

 ``innodb_stress`` has been added to the list of default MTR suites. For most supported systems satisfying the newly added dependencies is straightforward, but on *CentOS* 5, the default Python is too old. Thus ``python26`` and ``python26-mysqldb`` packages should be installed there and ``python26`` should be made the default python for the testsuite environment.

 Three new |TokuDB| variables, :variable:`tokudb_client_pool_threads`, :variable:`tokudb_cachetable_pool_threads`, and :variable:`tokudb_checkpoint_pool_threads`, have been implemented to improve the controlling of thread pool size.

 |Percona Server| has implemented new :variable:`tokudb_enable_partial_eviction` option in |TokuDB| to allow disabling of partial eviction of nodes.

 |Percona Server| has implemented new :variable:`tokudb_compress_buffers_before_eviction` option in |TokuDB| which allows the evictor to compress unused internal node partitions in order to reduce memory requirements as a first step of partial eviction before fully evicting the partition and eventually the entire node. 

Bugs Fixed
==========

 Querying :table:`GLOBAL_TEMPORARY_TABLES` table would crash threads working with internal temporary tables used by ``ALTER TABLE``. Bug fixed :bug:`1113388`.

 Selecting from :table:`GLOBAL_TEMPORARY_TABLES` table while running an online ``ALTER TABLE`` on a partitioned table in parallel could lead to a server crash. Bug fixed :bug:`1193264`.

 :ref:`innodb_kill_idle_trx` feature could cause an assertion on a debug build due to a race condition. Bug fixed :bug:`1206008`.

 ``libmylsqclient_16`` symbols were missing in |Percona Server| shared library package on *RHEL*/*CentOS* 7. Bug fixed :bug:`1420691`.

 Prepared statements in stored procedures could crash :ref:`response_time_distribution` plugin. Bug fixed :bug:`1426345`.

 When variable :variable:`innodb_corrupt_table_action` is set to ``Warn/Salvage`` then server could crash on updating table statistics during query execution on affected tables. Bug fixed :bug:`1426610`.

 A sequence of failing ``TRUNCATE TABLE``, then insert to that table, and ``CHECK TABLE`` would crash the server. Bug fixed :bug:`1433197`.

 When |InnoDB| change buffering was enabled and used, executing a ``FLUSH TABLE ... FOR EXPORT`` would cause a server hang and ``SHOW PROCESSLIST`` would show that table in a ``System Lock`` state. Bug fixed :bug:`1454441` (upstream :mysqlbug:`77011`).

 ``FLUSH INDEX_STATISTICS`` / ``FLUSH CHANGED_PAGE_BITMAPS`` and ``FLUSH USER_STATISTICS`` / ``RESET CHANGE_PAGE_BITMAPS`` pairs of commands were inadvertently joined, i.e. issuing either command had the effect of both. The first pair, besides flushing both index statistics and changed page bitmaps, had the effect of ``FLUSH INDEX_STATISTICS`` requiring ``SUPER`` instead of ``RELOAD`` privilege. The second pair resulted in ``FLUSH USER_STATISTICS`` destroying changed page bitmaps. Bug fixed :bug:`1472251`.

 Enabling :variable:`super_read_only` together with :variable:`read_only` in :file:`my.cnf` would result in server crashing on startup. The workaround is to enable :variable:`super_read_only` dynamically on a running server.Bug fixed :bug:`1389935` ( the fix was ported from Facebook patch `#14d5d9 <https://github.com/percona/percona-server/commit/14d5d9a94f2cac1ae67afd8a806a0ed581530f7e>`_).

 Enabling :variable:`super_read_only` as a command line option would not enable :variable:`read_only`. Bug fixed :bug:`1389935` ( the fix was ported from Facebook patch `#14d5d9 <https://github.com/percona/percona-server/commit/14d5d9a94f2cac1ae67afd8a806a0ed581530f7e>`_).

 If a new connection thread was created while a ``SHOW PROCESSLIST`` command or a :table:`INFORMATION_SCHEMA.PROCESSLIST` query was in progress, it could have a negative TIME_MS value returned in the PROCESSLIST output. Bug fixed :bug:`1379582`.

 With :ref:`innodb_fake_changes_page` enabled, a write to an |InnoDB| table that would cause B-tree reorganization could lead to server assertion with ``unknown error code 1000``. Bug fixed :bug:`1410410`.

 Running ``ALTER TABLE ... DISCARD TABLESPACE`` with :ref:`innodb_fake_changes_page` enabled would lead to a server assertion. Bug fixed :bug:`1372219`.

 ``ALTER TABLE`` did not allow to change a column to ``NOT NULL`` if the column was referenced in a foreign key. Bug fixed :bug:`1470677` (upstream :mysqlbug:`77591`).

 ``DROP TABLE IF EXISTS`` which fails due to a foreign key presence could break replication if slave had replication filters. Bug fixed :bug:`1475107` (upstream :mysqlbug:`77684`).

 Enabling :ref:`log_archiving` when :option:`--innodb-read-only` option was enabled would cause server to crash. Bug fixed :bug:`1484432`.

 LRU manager thread flushing was being accounted to ``buffer_flush_background`` *InnoDB*  metrics which was wrong and redundant. Bug fixed :bug:`1491420`.

 Fixed a typo in the cleaner thread loop where ``n_flushed`` is added to, instead of reset, by the idle server flushing. This may cause a cleaner thread sleep skip on a non-idle server. Bug fixed :bug:`1491435`.

 Running |TokuDB| for a long time with lots of file open and close operations could lead to a server crash due to server incorrectly setting a reserved value. Bug fixed :ftbug:`690`. 

 Fixed |TokuDB| memory leak due to data race in context status initialization. Bug fixed :ftbug:`697`. 

 Removed unnecessary calls to ``malloc_usable_size()`` function in PerconaFT library to improve the performance. Bug fixed :ftbug:`682`. 

Other bugs fixed: :bug:`1370002`, :bug:`1464468`, :bug:`1287299`, :bug:`1472256`, :tokubug:`867`, :tokubug:`870`, :tokubug:`878`, :ftbug:`658`, :ftbug:`661`, :ftbug:`663`, :ftbug:`665`, :ftbug:`687`, :ftbug:`696`, :ftbug:`698`, :ftbug:`870`, :ftbug:`685`, and :ftbug:`878`.
