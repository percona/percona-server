.. rn:: 5.6.16-64.0

=================================================
 |Percona Server| 5.6.16-64.0 [Not Released Yet]
=================================================

Percona is glad to announce the release of |Percona Server| 5.6.16-64.0 on March 6th, 2014 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.16-64.0/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.16 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-16.html>`_, including all the bug fixes in it, |Percona Server| 5.6.16-64.0 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.16-64.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.16-64.0>`_.

New Features
============

 |Percona Server| now supports :ref:`tokudb_compression` algorithms.

 In order to comply with Linux distribution packaging standards |Percona|'s version of ``libmysqlclient`` has been renamed to ``libperconaserver``. The reason this was done was to avoid name conflicts with packages upstream name and library conflicts, otherwise, ``libmysqlclient`` and ``libperconaserverclient`` of same version have no differences. Users will have to use ``libmysqlclient`` of respective distributions, for *CentOS* it is ``mysql-libs`` and for *Ubuntu*/*Debian* it is ``libmysqlclient18`` package. 
 
 :ref:`mysqlbinlog_change_db` has been ported from |Percona Server| 5.1.

Bugs Fixed
==========

 |Percona Server| 5.6 installation on *Debian* would fail due to default config reference to ``/etc/mysql/conf.d`` which didn't exist. Bug fixed :bug:`1206648`.

 Due to a regression in the buffer pool mutex split, a combination of |InnoDB| compression, write workload, and multiple active purge threads could lead to a server crash. Bug fixed :bug:`1247021`.

 Several fixes for multiple XA engine support in the server have been ported from |MariaDB|. Bugs fixed :bug:`1255551` (upstream :mysqlbug:`70854`) and :bug:`1255549` (upstream :mysqlbug:`47134`).
 
 ``auth_pam.so`` shared library needed for :ref:`pam_plugin` was missing from ``RPM`` packages. Bug fixed :bug:`1268246`.

 Fix for bug :bug:`1227581`, a buffer pool mutex split regression, was not complete, thus a combination of write workload to |InnoDB| compressed table and a tablespace drop could crash the server. Bug fixed :bug:`1269352`.

 |Percona Server| compilation with Performance Schema turned off would fail due to regression introduced by the 5.6 priority mutex framework. Bug fixed :bug:`1272747`.

 The |InnoDB| page cleaner thread could have incorrectly decided whether the server is busy or idle on some of its iterations and consequently issue a too big flush list flushing request on a busy server, causing performance instabilities. Bug fixed :bug:`1238039`.

 |Percona Server| had different server version value when installing from Source and from Binary/RPM. Bug fixed :bug:`1244178`.
 
 |InnoDB| did not handle the cases of asynchronous and synchronous I/O requests completing partially or being interrupted. Bugs fixed :bug:`1262500` (upstream :mysqlbug:`54430`), and :bug:`1263087` (*Andrew Gaul*).

 Fixed the upstream bug :mysqlbug:`70768`: the fix for upstream bug :mysqlbug:`70768` may cause a high rate of RW lock creations and desctructions, resulting in a performance regression on some workloads. Bug fixed :bug:`1279671`.

 *Debian* and *Ubuntu* packaging has been reworked to meet the packaging standards. Bug fixed :bug:`1281261`.

 Fixed the ``CMake`` warnings that were happening when ``Makefile`` was generated. Bugs fixed :bug:`1274827`, upstream bug fixed :mysqlbug:`71089` and :bug:`1274411` (upstream :mysqlbug:`71094`).

Other bugs fixed: :bug:`1052636`, :bug:`1014477`, :bug:`1276445`, :bug:`1274827` (upstream :mysqlbug:`71089`), :bug:`1264952`, :bug:`1005787`, :bug:`1285064`, :bug:`1229598`, :bug:`1277505` (upstream :mysqlbug:`71624`), and :bug:`1204871` (upstream :mysqlbug:`71270`).
