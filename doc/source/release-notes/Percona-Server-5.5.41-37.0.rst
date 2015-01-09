.. rn:: 5.5.41-37.0

==============================
 |Percona Server| 5.5.41-37.0
==============================

Percona is glad to announce the release of |Percona Server| 5.5.41-37.0 on January 9th, 2015. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.41-37.0/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.41 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-41.html>`_, including all the bug fixes in it, |Percona Server| 5.5.41-37.0 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.41-37.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.41-37.0>`_. 

New Features
============

 |Percona Server| has implemented :ref:`csv_engine_mode`. This feature also fixes the bug :bug:`1316042` (upstream :mysqlbug:`71091`).

 |Percona Server| has implemented improved slow log reporting for queries in :ref:`stored procedures <improved_sp_reporting>`.

Bugs Fixed
==========

 *Debian* and *Ubuntu* init scripts no longer have a hardcoded server startup timeout. This has been done to accommodate situations where server startup takes a very long time, for example, due to a crash recovery or buffer pool dump restore. Bugs fixed :bug:`1072538` and :bug:`1328262`.

 If HandlerSocket was enabled, the server would hang during shutdown. Bug fixed :bug:`1319904`.
 
 Wrong stack calculation could lead to a server crash when Performance Schema tables were storing big amount of data or in case of server being under highly concurrent load. Bug fixed :bug:`1351148` (upstream :mysqlbug:`73979`).
 
 Values of ``IP`` and ``DB`` fields in the :ref:`audit_log_plugin` were incorrect. Bug fixed :bug:`1379023`.

 *Percona Server* 5.5 would fail to build with GCC 4.9.1 (such as bundled with *Ubuntu Utopic*) in debug configuration. Bug fixed :bug:`1396358` (upstream :mysqlbug:`75000`). 

 Default *MySQL* configuration file, :file:`my.cnf`, was not installed during the new installation on *CentOS*. Bug fixed :bug:`1405667`.

 A session on a server in mixed mode binlogging would switch to row-based binlogging whenever a temporary table was created and then queried. This switch would last until the session end or until all temporary tables in the session were dropped. This was unnecessarily restrictive and has been fixed so that only the statements involving temporary tables were logged in the row-based format whereas the rest of the statements would continue to use the statement-based logging. Bug fixed :bug:`1313901` (upstream :mysqlbug:`72475`).
 
 Purging bitmaps exactly up to the last tracked LSN would abort :ref:`changed_page_tracking`. Bug fixed :bug:`1382336`.

 ``mysql_install_db`` script would silently ignore any mysqld startup failures. Bug fixed :bug:`1382782` (upstream :mysqlbug:`74440`).

Other bugs fixed: :bug:`1067103`, :bug:`1394357`, :bug:`1282599`, :bug:`1335590`, :bug:`1335590`, :bug:`1401791` (upstream :mysqlbug:`73281`), and :bug:`1396330` (upstream :mysqlbug:`74987`).
