.. rn:: 5.6.19-67.0

==============================
 |Percona Server| 5.6.19-67.0 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.19-67.0 on June 30th, 2014 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.19-67.0/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.19 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-19.html>`_, including all the bug fixes in it, |Percona Server| 5.6.19-67.0 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.19-67.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.19-67.0>`_. 

**NOTE**: There was no |Percona Server| 5.6.18 release because there was no |MySQL| Community Server 5.6.18 release. That version number was used for |MySQL| Enterprise Edition release to address the OpenSSL "Heartbleed" issue. 

New Features
============

 Percona has merged a contributed patch by Kostja Osipov implementing the :ref:`multiple_user_level_locks` feature. This feature fixes the upstream bugs: :mysqlbug:`1118` and :mysqlbug:`67806`.

 TokuDB Storage engine, which is available as a separate package that can be :ref:`installed <tokudb_installation>` along with the |Percona Server| 5.6.19-67.0, is now considered GA quality.

 |Percona Server| now supports the MTR :option:`--valgrind` option for a server that is either statically or dynamically linked with ``jemalloc``.

Bugs Fixed
==========

 The ``libperconaserverclient18.1`` package was missing the library files. Bug fixed :bug:`1329911`.

 |Percona Server| introduced a regression in :rn:`5.6.17-66.0` when support for TokuDB storage engine was implemented. This regression caused spurious "wrong table structure" errors for ``PERFORMANCE_SCHEMA`` tables. Bug fixed :bug:`1329772`.

 Race condition in group commit code could lead to a race condition in PFS instrumentation code resulting in a server crash. Bug fixed :bug:`1309026` (upstream :mysqlbug:`72681`).

Other bugs fixed: :bug:`1326348` and :bug:`1167486`. 


