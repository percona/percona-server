.. rn:: 5.6.16-64.0

==============================
 |Percona Server| 5.6.16-64.0 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.16-64.0 on March 10th, 2014 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.16-64.0/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.16 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-16.html>`_, including all the bug fixes in it, |Percona Server| 5.6.16-64.0 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.16-64.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.16-64.0>`_.

New Features
============

 |Percona Server| has implemented :ref:`backup_locks` that can be used as a lightweight alternative to ``FLUSH TABLES WITH READ LOCK`` for taking physical and logical backups.

 |Percona Server| has split a new :ref:`lru_manager_thread` out of the InnoDB cleaner thread, that performs ``LRU`` flushes and evictions to refill the free lists.

 |Percona Server| now has server-side support for :ref:`tokudb_compression` option syntax.

 |Percona Server| has implemented :ref:`slowlog_rotation` feature to provide users with better control of slow query log disk space usage.

 :ref:`mysqlbinlog_change_db` has been ported from |Percona Server| 5.1.

 In order to comply with Linux distribution packaging standards |Percona|'s version of ``libmysqlclient`` has been renamed to ``libperconaserver``. The old name was conflicting with the upstream ``libmysqlclient``. Except for packaging, ``libmysqlclient`` and ``libperconaserverclient`` of the same version do not have any differences. Users wishing to continue using ``libmysqlclient`` will have to install the corresponding package of their distribution, such as ``mysql-lib`` for *CentOS* and ``libmysqlclient18`` for *Ubuntu*/*Debian*. Users wishing to build software against ``libperconaserverclient`` should install ``libperconaserverclient-dev`` package. An old version of  Percona-built ``libmysqlclient`` will be available for `download <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.15-rel63.0/deb/>`_.
 
Bugs Fixed
==========

 The |XtraDB| version number in ``univ.i`` was incorrect. Bug fixed :bug:`1277383`.
 
 :ref:`udf_percona_toolkit` were only shipped with RPM packages. Bug fixed :bug:`1159625`.

 A debug server build will crash if, while performing a bulk insert to a partitioned table, one of the partitions will return a failure for ``end_bulk_insert`` handler call. Bug fixed :bug:`1204871` (upstream :mysqlbug:`71270`).

 |Percona Server| 5.6 installation on *Debian* would fail due to default config reference to ``/etc/mysql/conf.d`` which didn't exist. Bug fixed :bug:`1206648`.

 Due to a regression in the buffer pool mutex split, a combination of |InnoDB| compression, write workload, and multiple active purge threads could lead to a server crash. Bug fixed :bug:`1247021`.

 Server would crash on startup when XA support functions were activated by a second storage engine. Fix for this bug was ported from |MariaDB|. Bug fixed :bug:`1255549` (upstream :mysqlbug:`47134`). 

 ``FLUSH STATUS`` could cause a server crash on the next transaction commit if two XA-supporting storage engines are in use. Fix for this bug was ported from |MariaDB|. Bugs fixed :bug:`1255551` (upstream :mysqlbug:`70854`).
 
 ``auth_pam.so`` shared library needed for :ref:`pam_plugin` was missing from ``RPM`` packages. Bug fixed :bug:`1268246`.

 Fix for bug :bug:`1227581`, a buffer pool mutex split regression, was not complete, thus a combination of write workload to |InnoDB| compressed table and a tablespace drop could crash the server. Bug fixed :bug:`1269352`.

 Binary RPM packages couldn't be built from source tarballs on *Fedora* 19. Bug fixed :bug:`1229598`.

 |Percona Server| compilation with Performance Schema turned off would fail due to regression introduced by the 5.6 priority mutex framework. Bug fixed :bug:`1272747`.

 The |InnoDB| page cleaner thread could have incorrectly decided whether the server is busy or idle on some of its iterations and consequently issue a too big flush list flushing request on a busy server, causing performance instabilities. Bug fixed :bug:`1238039` (upstream :mysqlbug:`71988`).

 |Percona Server| had different server version value when installing from Source and from Binary/RPM. Bug fixed :bug:`1244178`.
 
 |InnoDB| did not handle the cases of asynchronous and synchronous I/O requests completing partially or being interrupted. Bugs fixed :bug:`1262500` (upstream :mysqlbug:`54430`), and :bug:`1263087` (*Andrew Gaul*).

 The fix for upstream bug :mysqlbug:`70768` may cause a high rate of RW lock creations and destructions, resulting in a performance regression on some workloads. Bug fixed :bug:`1279671` (upstream :mysqlbug:`71708`).

 *Debian* and *Ubuntu* packaging has been reworked to meet the packaging standards. Bug fixed :bug:`1281261`.

 Fixed the ``CMake`` warnings that were happening when ``Makefile`` was generated. Bugs fixed :bug:`1274827`, upstream bug fixed :mysqlbug:`71089` and :bug:`1274411` (upstream :mysqlbug:`71094`).

 On *Ubuntu* Precise multiple architecture versions of ``libmysqlclient18`` couldn't be installed side by side. Bug fixed :bug:`1052636`.

 |Percona Server| source tree has been reorganized to match the |MySQL| source tree layout closer. Bug fixed :bug:`1014477`.

 Performance schema autosizing heuristics have been updated to account for Percona Server-specific ``wait/synch/mutex/sql/THD::LOCK_temporary_tables`` mutex. Bug fixed :bug:`1264952`.

 Database administrator password could be seen in plain text if when ``debconf-get-selections`` was executed. Bug fixed :bug:`1018291`.

Other bugs fixed: :bug:`1276445`, :bug:`1005787`, :bug:`1285064`, :bug:`1229598`, and :bug:`1277505` (upstream :mysqlbug:`71624`).
