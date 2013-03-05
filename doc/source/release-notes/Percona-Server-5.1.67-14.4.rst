.. rn:: 5.1.67-14.4

==============================
 |Percona Server| 5.1.67-14.4 
==============================

Percona is glad to announce the release of |Percona Server| 5.1.67-14.4 on March 8th, 2013 (Downloads are available from `Percona Server 5.1.67-14.4 downloads <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.67-14.4/>`_ and from the `Percona Software Repositories <http://http://www.percona.com/doc/percona-server/5.1/installation.html>`_).

Based on `MySQL 5.1.67 <http://dev.mysql.com/doc/relnotes/mysql/5.1/en/news-5-1-67.html>`_ this release will include all the bug fixes in it. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.1.67-14.4 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.1.67-14.4>`_.

New Features
============

 New user statements have been introduced for handling the :ref:`changed_page_tracking` log files.

Bug Fixes
=========

 Time in slow query log was displayed incorrectly when :variable:`slow_query_log_microseconds_timestamp` variable was set to microseconds. Bug fixed :bug:`887928` (*Laurynas Biveinis*).

 Upstream bug :mysqlbug:`67983` was causing a memory leak on a filtered slave. Bug fixed :bug:`1042946` (*Sergei Glushchenko*).

 The master thread was doing dirty buffer pool flush list reads to make its adaptive flushing decisions. Fixed by acquiring the flush list mutex around the list scans. Bug fixed :bug:`1083058` (*Laurynas Biveinis*).

 The server could crash when executing an ``INSERT``  or ``UPDATE`` statement containing ``BLOB`` values for a compressed table. This regression was introduced in |Percona Server| :rn:`5.1.59-13.0`. Bug fixed :bug:`1100159` (*Laurynas Biveinis*).

 Fixed the upstream :mysqlbug:`68116` that caused the server crash with assertion error when |InnoDB| monitor with verbose lock info was used under heavy load. This bug is affecting only ``-debug`` builds. Bug fixed :bug:`1100178` (*Laurynas Biveinis*).

 When option :variable:`innodb_flush_method=O_DIRECT` was set up, log bitmap files were created and treated as |InnoDB| data files for flushing purposes, which wasn't original intention. Bug fixed :bug:`1105709` (*Laurynas Biveinis*).

 ``INFORMATION_SCHEMA`` plugin name :variable:`innodb_changed_pages` serves also as a command line option, but it is also a prefix of another command line option :variable:`innodb_changed_pages_limit`. |MySQL| option handling would then shadow the former with the latter, resulting in start up errors. Fixed by renaming the :variable:`innodb_changed_pages_limit` option to :variable:`innodb_max_changed_pages`. Bug fixed :bug:`1105726` (*Laurynas Biveinis*).

 Writing bitmap larger than 4GB would cause write to fail. Also a write error for every bitmap page, except the first one, would result in a heap corruption. Bug fixed :bug:`1111226` (*Laurynas Biveinis*).

 :ref:`changed_page_tracking` wasn't compatible with :option:`innodb_force_recovery=6`. When starting the server log tracking initialization would fail. The server would abort on startup. Bug fixed :bug:`1083596` (*Laurynas Biveinis*).

 |InnoDB| monitor was prefetching the data pages for printing lock information even if no lock information was going to be printed. Bug fixed :bug:`1100643` (*Laurynas Biveinis*).

 Newly created bitmap file would silently overwrite the old one if they had the same file name. Bug fixed :bug:`1111144` (*Laurynas Biveinis*).

 A server would stop with an assertion error in I/O and AIO routines if large :variable:`innodb_log_block_size` value is used in the combination with changed page tracking. Bug fixed :bug:`1114612` (*Laurynas Biveinis*). 

 Fixed the regression introduced with the fix for bug :bug:`1083058` which caused unnecessary mutex reacquisitions in adaptive flushing. Bug fixed :bug:`1117067` (*Laurynas Biveinis*).

 |InnoDB| and the query plan information were being logged even if they weren't enabled for the slow query log. Bug fixed :bug:`730173` (*Laurynas Biveinis*).

 Fixed the regular expressions used for filtering the |InnoDB| stats that were causing sporadic extraneous lines in ``mysqldumpslow`` output. Bug fixed :bug:`1097692` (*Laurynas Biveinis*).

Other bug fixes: bug fixed :bug:`1098436` (*Laurynas Biveinis*), bug fixed :bug:`1096904` (*Laurynas Biveinis*), bug fixed :bug:`1096899` (*Laurynas Biveinis*), bug fixed :bug:`1096895` (*Laurynas Biveinis*), bug fixed :bug:`1092142` (*Laurynas Biveinis*), bug fixed :bug:`1091712` (*Laurynas Biveinis*), bug fixed :bug:`1090874` (*Laurynas Biveinis*), bug fixed :bug:`1089961` (*Laurynas Biveinis*), bug fixed :bug:`1088954` (*Laurynas Biveinis*), bug fixed :bug:`1088867` (*Laurynas Biveinis*), bug fixed :bug:`1089265` (*Laurynas Biveinis*), bug fixed :bug:`1089031` (*Laurynas Biveinis*), bug fixed :bug:`1108874` (*Laurynas Biveinis*), bug fixed :bug:`1083669` (*Laurynas Biveinis*), bug fixed :bug:`1082437` (*Laurynas Biveinis*), bug fixed :bug:`909376` (*Laurynas Biveinis*).

