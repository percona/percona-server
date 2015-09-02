.. rn:: 5.5.45-37.4

==============================
 |Percona Server| 5.5.45-37.4
==============================

Percona is glad to announce the release of |Percona Server| 5.5.45-37.4 on September 2nd, 2015. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.45-37.4/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.45 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-45.html>`_, including all the bug fixes in it, |Percona Server| 5.5.45-37.4 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.45-37.4 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.45-37.4>`_. 

Bugs Fixed
==========

 Querying ``INFORMATION_SCHEMA`` :table:`GLOBAL_TEMPORARY_TABLES` table would crash threads working with internal temporary tables used by ``ALTER TABLE``. Bug fixed :bug:`1113388`.

 ``FLUSH INDEX_STATISTICS`` / ``FLUSH CHANGED_PAGE_BITMAPS`` and ``FLUSH USER_STATISTICS`` / ``RESET CHANGE_PAGE_BITMAPS`` pairs of commands were inadvertently joined, i.e. issuing either command had the effect of both. The first pair, besides flushing both index statistics and changed page bitmaps, had the effect of ``FLUSH INDEX_STATISTICS`` requiring ``SUPER`` instead of ``RELOAD`` privilege. The second pair resulted in ``FLUSH USER_STATISTICS`` destroying changed page bitmaps. Bug fixed :bug:`1472251`.
 
 If a new connection thread was created while a ``SHOW PROCESSLIST`` command or a :table:`INFORMATION_SCHEMA.PROCESSLIST` query was in progress, it could have a negative ``TIME_MS`` value returned in the ``PROCESSLIST`` output. Bug fixed :bug:`1379582`. 

Other bugs fixed: :bug:`768038` and :bug:`1472256`. 
