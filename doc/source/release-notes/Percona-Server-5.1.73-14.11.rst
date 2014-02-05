.. rn:: 5.1.73-14.11

===============================
 |Percona Server| 5.1.73-14.11 
===============================

Percona is glad to announce the release of |Percona Server| 5.1.73-14.11 on December 20th, 2013 (Downloads are available from `Percona Server 5.1.73-14.11 downloads <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.73-14.11>`_ and from the `Percona Software Repositories <http://www.percona.com/doc/percona-server/5.1/installation.html>`_).

Based on `MySQL 5.1.73 <http://dev.mysql.com/doc/relnotes/mysql/5.1/en/news-5-1-73.html>`_, this release will include all the bug fixes in it. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.1.73-14.11 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.1.73-14.11>`_.

Bugs Fixed
==========

 ``INSTALL PLUGIN`` statement would crash server if :ref:`user_stats` were enabled. Bug fixed :bug:`1011047`.

 Running top-level make would fail while running ``autoreconf`` in UDF subdirectory if the *Automake* version was 1.13. Bug fixed :bug:`1244110`.

 Server would fail to build with *GCC* 4.8 in build configurations that have the |MySQL| maintainer mode enabled. Bugs fixed :bug:`1244154` (upstream bug :mysqlbug:`68909`) and :bug:`1186190` (upstream bug :mysqlbug:`69407`).

 ``PURGE CHANGED_PAGE_BITMAPS BEFORE`` statement would delete the changed page data after the specified LSN and up to the start of the next bitmap file. If this data were to be used for fast incremental backups, its absence would cause |Percona XtraBackup| to fall back to the full-scan incremental backup mode. Bug fixed :bug:`1260035` (*Andrew Gaul*).

Other bug fixes: :bug:`1082333`.
