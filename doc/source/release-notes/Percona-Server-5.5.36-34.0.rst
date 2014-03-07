.. rn:: 5.5.36-34.0

==============================
 |Percona Server| 5.5.36-34.0 
==============================

Percona is glad to announce the release of |Percona Server| 5.5.36-34.0 on March 10th, 2014. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.36-34.0/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.36 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-36.html>`_, including all the bug fixes in it, |Percona Server| 5.5.36-34.0 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.36-34.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.36-34.0>`_. 

New Features
============
 
 *Debian* and *Ubuntu* packaging has been reworked to meet the packaging standards.

 :ref:`mysqlbinlog_change_db` has been ported from |Percona Server| 5.1.

 |Percona Server| has implemented :ref:`slowlog_rotation` feature to provide users with better control of slow query log disk space usage.

 In order to comply with Linux distribution packaging standards |Percona|'s version of ``libmysqlclient`` has been renamed to ``libperconaserver``. The old name was conflicting with the upstream ``libmysqlclient``. Except for packaging, ``libmysqlclient`` and ``libperconaserverclient`` of the same version do not have any differences. Users wishing to continue using ``libmysqlclient`` will have to install the corresponding package of their distribution, such as ``mysql-lib`` for *CentOS* and ``libmysqlclient18`` for *Ubuntu*/*Debian*. Users wishing to build software against ``libperconaserverclient`` should install ``libperconaserverclient-dev`` package. An old version of  Percona-built ``libmysqlclient`` will be available for `download <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.35-rel33.0/deb/>`_.

Bugs Fixed
==========

 The |XtraDB| version number in ``univ.i`` was incorrect. Bug fixed :bug:`1277383`.

 :ref:`udf_percona_toolkit` were only shipped with RPM packages. Bug fixed :bug:`1159625`.

 Server could crash if it was signaled with ``SIGHUP`` early in the server startup. Bug fixed :bug:`1249193` (upstream :mysqlbug:`62311`).

 Server could crash if |XtraDB| :variable:`innodb_dict_size` option was set due to incorrect attempts to remove indexes in use from the dictionary cache. Bugs fixed :bug:`1250018` and :bug:`758788`.

 Fix for bug :bug:`1227581`, a buffer pool mutex split regression, was not complete, thus a combination of write workload and tablespace drop could crash the server if |InnoDB| compression was used. Bug fixed :bug:`1269352`.

 Binary RPM packages couldn't be built from source tarballs on *Fedora* 19. Bug fixed :bug:`1229598`.

 |Percona Server| that was compiled from source package had different server version string from that of binary packages. Bug fixed :bug:`1244178`.

 |InnoDB| did not handle the cases of asynchronous and synchronous I/O requests completing partially or being interrupted. Bugs fixed :bug:`1262500` (upstream :mysqlbug:`54430`), and :bug:`1263087` (*Andrew Gaul*).

 Fixed the ``CMake`` warnings that were happening when ``Makefile`` was generated. Bugs fixed :bug:`1274827` (upstream :mysqlbug:`71089`).

 |Percona Server| source tree has been reorganized to match the |MySQL| source tree layout closer. Bug fixed :bug:`1014477`.

 On *Ubuntu* Precise multiple architecture versions of ``libmysqlclient18`` couldn't be installed side by side. Bug fixed :bug:`1052636`.


Other bugs fixed: :bug:`1005787`.
