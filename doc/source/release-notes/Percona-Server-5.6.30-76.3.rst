.. rn:: 5.6.30-76.3

==============================
 |Percona Server| 5.6.30-76.3 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.30-76.3 on May 25th, 2016 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.30-76.3/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.30 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-30.html>`_, including all the bug fixes in it, |Percona Server| 5.6.30-76.3 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.30-76.3 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.30-76.3>`_.

Bugs Fixed
==========
 
 When :ref:`tokudb_read_free_replication` was enabled for |TokuDB| and there was no explicit primary key for the replicated |TokuDB| table there could be duplicated records in the table on update operation. The fix disables :ref:`tokudb_read_free_replication` for tables without explicit primary key and does rows lookup for ``UPDATE`` and ``DELETE`` binary log events and issues warning. Bug fixed :bug:`1536663` (:tokubug:`950`).

 Attempting to execute a non-existing prepared statement with :ref:`response_time_distribution` plugin enabled could lead to a server crash. Bug fixed :bug:`1538019`.

 |TokuDB| was using using different memory allocators, this was causing ``safemalloc`` warnings in debug builds and crashes because memory accounting didn't add up. Bug fixed :bug:`1546538` (:tokubug:`962`).

 Fixed heap allocator/deallocator mismatch in :ref:`scalability_metrics_plugin`. Bug fixed :bug:`1581051`.

 |Percona Server| is now built with system ``zlib`` library instead of the older bundled one. Bug fixed :bug:`1108016`.

 Reduced the memory overhead per page in the |InnoDB| buffer pool. The fix was based on Facebook patch `#91e979e <https://github.com/facebook/mysql-5.6/commit/91e979e8436b83400e918fa0f251036e50d0cb5f>`_. Bug fixed :bug:`1536693` (upstream :mysqlbug:`72466`).

 ``CREATE TABLE ... LIKE ...`` could create a system table with an unsupported enforced engine. Bug fixed :bug:`1540338`.

 Change buffer merge could throttle to 5% of I/O capacity on an idle server. Bug fixed :bug:`1547525`.

 ``Slave_open_temp_tables`` would fail to decrement on the slave with disabled binary log if master was killed. Bug fixed :bug:`1567361`.

 Server will now show more descriptive error message when |Percona Server| fails with ``errno == 22 "Invalid argument"``, if :variable:`innodb_flush_method` was set to ``ALL_O_DIRECT``. Bug fixed :bug:`1578604`.

 Killed connection threads could get their sockets closed twice on shutdown. Bug fixed :bug:`1580227`.

 ``AddressSanitizer`` build with ``LeakSanitizer`` enabled was failing at ``gen_lex_hash`` invocation. Bug fixed :bug:`1580993` (upstream :mysqlbug:`80014`).

 ``apt-cache show`` command for ``percona-server-client`` was showing ``innotop`` included as part of the package. Bug fixed :bug:`1201074`.

 ``mysql-systemd`` would fail with PAM authentication and proxies due to regression introduced when fixing :bug:`1534825` in |Percona Server| :rn:`5.6.29-76.2`. Bug fixed :bug:`1558312`.

 Upgrade logic for figuring if |TokuDB| upgrade can be performed from the version on disk to the current version was broken due to regression introduced when fixing :ftbug:`684` in |Percona Server| :rn:`5.6.27-75.0`. Bug fixed :ftbug:`717`.

 If ``ALTER TABLE`` was run while :variable:`tokudb_auto_analyze` variable was enabled it would trigger auto-analysis, which could lead to a server crash if ``ALTER TABLE DROP KEY`` was used because it would be operating on the old table/key meta-data. Bug fixed :tokubug:`945`.

 The |TokuDB| storage engine with :variable:`tokudb_pk_insert_mode` set to ``1`` is safe to use in all conditions. On ``INSERT IGNORE`` or ``REPLACE INTO``, it tests to see if triggers exist on the table, or replication is active with ``!BINLOG_FORMAT_STMT`` before it allows the optimization. If either of these conditions are met, then it falls back to the "safe" operation of looking up the target row first. Bug fixed :tokubug:`952`.

 Bug in |TokuDB| Index Condition Pushdown was causing ``ORDER BY DESC`` to reverse the scan outside of the `WHERE` bounds. This would cause query to hang in a ``sending data`` state for several minutes in some environments with large amounts of data (3 billion records) if the ``ORDER BY DESC`` statement was used. Bugs fixed :tokubug:`988`, :tokubug:`233`, and :tokubug:`534`.

Other bugs fixed: :bug:`1399562` (upstream :mysqlbug:`75112`), :bug:`1510564` (upstream :mysqlbug:`78981`), :bug:`1496282` (:tokubug:`964`), :bug:`1496786` (:tokubug:`956`), :bug:`1566790`, :bug:`1552673`, :bug:`1567247`, :bug:`1567869`, :ftbug:`718`, :tokubug:`914`, :tokubug:`970`, :tokubug:`971`, :tokubug:`972`, :tokubug:`976`, :tokubug:`977`, :tokubug:`981`, :tokubug:`637`, and :tokubug:`982`.  
