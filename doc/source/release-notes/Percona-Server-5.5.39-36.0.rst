.. rn:: 5.5.39-36.0

==============================
 |Percona Server| 5.5.39-36.0
==============================

Percona is glad to announce the release of |Percona Server| 5.5.39-36.0 on August 29th, 2014. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.39-36.0/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.39 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-39.html>`_, including all the bug fixes in it, |Percona Server| 5.5.39-36.0 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.39-36.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.39-36.0>`_. 

New Features
============

 |Percona Server| :ref:`audit_log_plugin` now supports ``JSON`` and ``CSV`` formats. The format choice is controlled by :variable:`audit_log_format` variable.

 |Percona Server| :ref:`audit_log_plugin` now supports :ref:`streaming the audit log to syslog <streaming_to_syslog>`.

Bugs Fixed
==========

 Querying :table:`INNODB_CHANGED_PAGES` with a range condition ``START_LSN > x AND END_LSN < y`` would lead to a server crash if the range was empty with x greater than y. Bug fixed :bug:`1202252` (*Jan Lindström* and *Sergei Petrunia*).

 With :ref:`changed_page_tracking` feature enabled, queries from the :table:`INNODB_CHANGED_PAGES` could read the bitmap data whose write was in still progress. This would cause the query to fail with an ``ER_CANT_FIND_SYSTEM_REC`` and a warning printed to the server error log. The workaround is to add an appropriate ``END_LSN``-limiting condition to the query. Bug fixed :bug:`1346122`.

 ``mysqld-debug`` was missing from *Debian* packages. This regression was introduced in |Percona Server| :rn:`5.5.36-34.0`. Bug fixed :bug:`1290087`.

 Fixed a memory leak in :ref:`slowlog_rotation`. Bug fixed :bug:`1314138`.

 The audit log plugin would write log with XML syntax errors when ``OLD`` and ``NEW`` formats were used. Bug fixed :bug:`1320879`.

 A server built with system OpenSSL support, such as the distributed Percona Server binaries, had SSL-related memory leaks. Bug fixed :bug:`1334743` (upstream :mysqlbug:`73126`).

 If the bitmap directory has a bitmap file sequence with a start LSN of one file less than a start LSN of the previous file, a debug build would assert when queries were run on :table:`INNODB_CHANGED_PAGES` table. Bug fixed :bug:`1342494`.

 Server would crash on login attempt if ``mysql.user`` table was truncated. Bug fixed :bug:`1322218`.

Other bugs fixed: :bug:`1337324`, :bug:`1151723`, :bug:`1182050`, :bug:`1182072`, :bug:`1280875`, :bug:`1182046`, :bug:`1328482` (upstream :mysqlbug:`73418`), and :bug:`1334317` (upstream :mysqlbug:`73111`).
