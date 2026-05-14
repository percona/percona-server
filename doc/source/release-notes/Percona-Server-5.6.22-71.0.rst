.. rn:: 5.6.22-71.0

==============================
 |Percona Server| 5.6.22-71.0 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.22-71.0 on January 12th, 2015 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.22-71.0/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.22 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-22.html>`_, including all the bug fixes in it, |Percona Server| 5.6.22-71.0 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.22-71.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.22-71.0>`_. 

New Features
============

 |Percona Server| has implemented improved slow log reporting for queries in :ref:`stored procedures <improved_sp_reporting>`.

 TokuDB storage engine package has been updated to version `7.5.4 <http://docs.tokutek.com/tokudb/tokudb-release-notes.html#tokudb-version-7-x>`_. |Percona Server| with an older version of TokuDB could hit an early scaling limit when the binary log was enabled. TokuDB 7.5.4 fixes this problem by using the hints supplied by the binary log group commit algorithm to avoid fsync'ing its recovery log during the commit phase of the 2 phase commit algorithm that *MySQL* uses for transactions when the binary log is enabled.

Bugs Fixed
==========

 *Debian* and *Ubuntu* init scripts no longer have a hardcoded server startup timeout. This has been done to accommodate situations where server startup takes a very long time, for example, due to a crash recovery or buffer pool dump restore. Bugs fixed :bug:`1072538` and :bug:`1328262`.

 A read-write workload on compressed *InnoDB* tables might have caused an assertion error. Bug fixed :bug:`1268656`.

 Selecting from :table:`GLOBAL_TEMPORARY_TABLES` table while running an online ``ALTER TABLE`` in parallel could lead to server crash. Bug fixed :bug:`1294190`.

 A wrong stack size calculation could lead to a server crash when Performance Schema tables were storing big amount of data or in case of server being under highly concurrent load. Bug fixed :bug:`1351148` (upstream :mysqlbug:`73979`).

 A query on an empty table with a ``BLOB`` column may crash the server. Bug fixed :bug:`1384568` (upstream :mysqlbug:`74644`).
 
 A read-write workload on compressed *InnoDB* tables might have caused an assertion error. Bug fixed :bug:`1395543`.

 If :ref:`handlersocket_page` was enabled, the server would hang during shutdown. Bug fixed :bug:`1397859`.

 The default *MySQL* configuration file, :file:`my.cnf`, was not installed during a new installation on *CentOS*. Bug fixed :bug:`1405667`.

 The query optimizer did not pick a covering index for some ``ORDER BY`` queries. Bug fixed :bug:`1394967` (upstream :mysqlbug:`57430`).

 ``SHOW ENGINE INNODB STATUS`` was displaying two identical ``TRANSACTIONS`` sections. Bug fixed :bug:`1404565`. 

 A race condition in :ref:`multiple_user_level_locks` implementation could cause a deadlock. Bug fixed :bug:`1405076`.

Other bugs fixed: :bug:`1394357`, :bug:`1337251`, :bug:`1399174`, :bug:`1396330` (upstream :mysqlbug:`74987`), :bug:`1401776` (upstream :mysqlbug:`75189`).
