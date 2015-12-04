.. rn:: 5.6.20-68.0

==============================
 |Percona Server| 5.6.20-68.0 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.20-68.0 on August 29th, 2014 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.20-68.0/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.20 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-20.html>`_, including all the bug fixes in it, |Percona Server| 5.6.20-68.0 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.20-68.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.20-68.0>`_. 


New Features
============

 |Percona Server| has implemented the |MySQL| 5.7 ``SHOW SLAVE STATUS NONBLOCKING`` syntax for :ref:`show_slave_status_nolock` feature. The existing ``SHOW SLAVE STATUS NOLOCK`` is kept as a deprecated alias and will be removed in |Percona Server| 5.7. There were no functional changes for the feature.

 |Percona Server| :ref:`audit_log_plugin` now supports ``JSON`` and ``CSV`` formats. The format choice is controlled by :variable:`audit_log_format` variable.

 |Percona Server| :ref:`audit_log_plugin` now supports :ref:`streaming the audit log to syslog <streaming_to_syslog>`. 

 TokuDB storage engine package has been updated to version 7.1.8. 

Bugs Fixed
==========

 Querying :table:`INNODB_CHANGED_PAGES` with a range condition ``START_LSN > x AND END_LSN < y`` would lead to a server crash if the range was empty with x greater than y. Bug fixed :bug:`1202252` (*Jan Lindström* and *Sergei Petrunia*).

 SQL statements of other connections were missing in the output of ``SHOW ENGINE INNODB STATUS``, in ``LATEST DETECTED DEADLOCK`` and ``TRANSACTIONS`` sections. This bug was introduced by :ref:`statement_timeout` patch in |Percona Server| :rn:`5.6.13-61.0`. Bug fixed :bug:`1328824`.

 Some of TokuDB distribution files were missing in the TokuDB binary tarball. Bug fixed :bug:`1338945`.

 With :ref:`changed_page_tracking` feature enabled, queries from the :table:`INNODB_CHANGED_PAGES` could read the bitmap data whose write was in still progress. This would cause the query to fail with an ``ER_CANT_FIND_SYSTEM_REC`` and a warning printed to the server error log. The workaround is to add an appropriate ``END_LSN``-limiting condition to the query. Bug fixed :bug:`1193332`.
 
 ``mysqld-debug`` was missing from *Debian* packages. This regression was introduced in |Percona Server| :rn:`5.6.16-64.0`. Bug fixed :bug:`1290087`.

 Fixed a memory leak in :ref:`slowlog_rotation`. Bug fixed :bug:`1314138`.

 The audit log plugin would write log with XML syntax errors when ``OLD`` and ``NEW`` formats were used. Bug fixed :bug:`1320879`.

 Combination of :ref:`log_archiving`, :ref:`changed_page_tracking`, and small |InnoDB| logs could hang the server on the bootstrap shutdown. Bug fixed :bug:`1326379`.

 :option:`--tc-heuristic-recover` option values were broken. Bug fixed :bug:`1334330` (upstream :mysqlbug:`70860`).

 If the bitmap directory has a bitmap file sequence with a start LSN of one file less than a start LSN of the previous file, a debug build would assert when queries were run on :table:`INNODB_CHANGED_PAGES` table. Bug fixed :bug:`1342494`.
 
Other bugs fixed: :bug:`1337247`, :bug:`1350386`, :bug:`1208371`, :bug:`1261341`, :bug:`1151723`, :bug:`1182050`, :bug:`1182068`, :bug:`1182072`, :bug:`1184287`, :bug:`1280875`, :bug:`1338937`, :bug:`1334743`, :bug:`1349394`, :bug:`1182046`, :bug:`1182049`, and :bug:`1328482` (upstream :mysqlbug:`73418`).


