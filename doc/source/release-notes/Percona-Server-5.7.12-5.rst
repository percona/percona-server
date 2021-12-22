.. rn:: 5.7.12-5

===========================
 |Percona Server| 5.7.12-5
===========================

Percona is glad to announce the GA (Generally Available) release of |Percona
Server| 5.7.12-5 on June 6th, 2016 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.12-5/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.7.12
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-12.html>`_, including
all the bug fixes in it, |Percona Server| 5.7.12-5 is the current GA release in
the |Percona Server| 5.7 series. All of Percona's software is open-source and
free, all the details of the release can be found in the `5.7.12-5 milestone at
Launchpad <https://launchpad.net/percona-server/+milestone/5.7.12-5>`_

Bugs Fixed
==========

 ``MEMORY`` storage engine did not support JSON columns. Bug fixed
 :bug:`1536469`.

 When :ref:`tokudb_read_free_replication` was enabled for TokuDB and there
 was no explicit primary key for the replicated TokuDB table there could be
 duplicated records in the table on update operation. The fix disables
 :ref:`tokudb_read_free_replication` for tables without explicit primary key
 and does rows lookup for ``UPDATE`` and ``DELETE`` binary log events and
 issues warning. Bug fixed :bug:`1536663` (:tokubug:`950`).

 Attempting to execute a non-existing prepared statement with
 :ref:`response_time_distribution` plugin enabled could lead to a server crash.
 Bug fixed :bug:`1538019`.

 TokuDB was using using different memory allocators, this was causing
 ``safemalloc`` warnings in debug builds and crashes because memory accounting
 didn't add up. Bug fixed :bug:`1546538` (:tokubug:`962`).

 Adding an index to an InnoDB temporary table while
 :variable:`expand_fast_index_creation` was enabled could lead to server
 assertion. Bug fixed :bug:`1554622`.

 |Percona Server| was missing the :variable:`innodb_numa_interleave` server
 variable. Bug fixed :bug:`1561091` (upstream :mysqlbug:`80288`).

 Running ``SHOW STATUS`` in parallel to online buffer pool resizing could lead
 to server crash. Bug fixed :bug:`1577282`.

 InnoDB crash recovery might fail if :variable:`innodb_flush_method` was set
 to ``ALL_O_DIRECT``. Bug fixed :bug:`1529885`.

 Fixed heap allocator/deallocator mismatch in
 :ref:`scalability_metrics_plugin`. Bug fixed :bug:`1581051`.

 |Percona Server| is now built with system ``zlib`` library instead of the
 older bundled one. Bug fixed :bug:`1108016`.

 ``CMake`` would fail if TokuDB tests passed. Bug fixed :bug:`1521566`.

 Reduced the memory overhead per page in the InnoDB buffer pool. The fix was
 based on Facebook patch
 `#91e979e <https://github.com/facebook/mysql-5.6/commit/91e979e8436b83400e918fa0f251036e50d0cb5f>`_.
 Bug fixed :bug:`1536693` (upstream :mysqlbug:`72466`).

 ``CREATE TABLE ... LIKE ...`` could create a system table with an unsupported
 enforced engine. Bug fixed :bug:`1540338`.

 Change buffer merge could throttle to 5% of I/O capacity on an idle server.
 Bug fixed :bug:`1547525`.

 Parallel doublewrite memory was not freed with
 :variable:`innodb_fast_shutdown` was set to ``2``. Bug fixed :bug:`1578139`.

 Server will now show more descriptive error message when |Percona Server|
 fails with ``errno == 22 "Invalid argument"``, if
 :variable:`innodb_flush_method` was set to ``ALL_O_DIRECT``. Bug fixed
 :bug:`1578604`.

 The error log warning ``Too many connections`` was only printed for connection
 attempts when :variable:`max_connections` + one ``SUPER`` have connected. If
 the extra ``SUPER`` is not connected, the warning was not printed for a
 non-SUPER connection attempt. Bug fixed :bug:`1583553`.

 ``apt-cache show`` command for ``percona-server-client`` was showing
 ``innotop`` included as part of the package. Bug fixed :bug:`1201074`.

 A replication slave would fail to connect to a master running 5.5. Bug fixed
 :bug:`1566642` (upstream :mysqlbug:`80962`).

 Upgrade logic for figuring if TokuDB upgrade can be performed from the
 version on disk to the current version was broken due to regression introduced
 when fixing :ftbug:`684` in |Percona Server| :rn:`5.7.11-4`. Bug fixed
 :ftbug:`717`.

 Fixed ``jemalloc`` version parsing error. Bug fixed :tokubug:`528`.

 If ``ALTER TABLE`` was run while :variable:`tokudb_auto_analyze` variable was
 enabled it would trigger auto-analysis, which could lead to a server crash if
 ``ALTER TABLE DROP KEY`` was used because it would be operating on the old
 table/key meta-data. Bug fixed :tokubug:`945`.

 The :variable:`tokudb_pk_insert_mode` session variable has been deprecated and
 the behavior will be that of the former :variable:`tokudb_pk_insert_mode` set
 to ``1``. The optimization will be used where safe and not used where not
 safe. Bug fixed :tokubug:`952`.

 Bug in TokuDB Index Condition Pushdown was causing ``ORDER BY DESC`` to
 reverse the scan outside of the `WHERE` bounds. This would cause query to hang
 in a ``sending data`` state for several minutes in some environments with
 large amounts of data (3 billion records) if the ``ORDER BY DESC`` statement
 was used. Bugs fixed :tokubug:`988`, :tokubug:`233`, and :tokubug:`534`.

Other bugs fixed: :bug:`1510564` (upstream :mysqlbug:`78981`), :bug:`1533482`
(upstream :mysqlbug:`79999`), :bug:`1553166`, :bug:`1496282` (:tokubug:`964`),
:bug:`1496786` (:tokubug:`956`), :bug:`1566790`, :ftbug:`718`, :tokubug:`914`,
:tokubug:`937`, :tokubug:`954`, :tokubug:`955`, :tokubug:`970`, :tokubug:`971`,
:tokubug:`972`, :tokubug:`976`, :tokubug:`977`, :tokubug:`981`, :tokubug:`982`,
:tokubug:`637`, and :tokubug:`982`.
