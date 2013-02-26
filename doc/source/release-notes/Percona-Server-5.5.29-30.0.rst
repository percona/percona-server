.. rn:: 5.5.29-30.0

==============================
 |Percona Server| 5.5.29-30.0 
==============================

Percona is glad to announce the release of |Percona Server| 5.5.29-29.4 on February 26th, 2012 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.29-29.4/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.5.29 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-29.html>`_, including all the bug fixes in it, |Percona Server| 5.5.29-30.0 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.29-30.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.29-30.0>`_. 

New Features
============

 Ported the :ref:`threadpool` patch from |MariaDB|. This feature enables the server to keep the top performance even with the increased number of client connections.

 New user statements have been introduced for handling the :ref:`changed_page_tracking` log files.

 In addition to the :option:`--debug` build option for :file:`build-binary.sh` script, new :option:`--valgrind` option has been introduced, which will build debug builds with the `Valgrind <http://valgrind.org/>`_ instrumentation enabled.

Bug Fixes
=========

 Ported a fix from `MariaDB <https://mariadb.atlassian.net/browse/MDEV-364>`_ for the upstream bug :mysqlbug:`67974`, which caused server crash on concurrent ``ALTER TABLE`` and ``SHOW ENGINE INNODB STATUS``. Bug fixed :bug:`1017192` (*Sergei Glushchenko*).

 The server could crash when executing an ``INSERT`` statement containing ``BLOB`` values. This regression was introduced in |Percona Server| :rn:`5.5.28-29.2`. Bug fixed :bug:`1100159` (*Laurynas Biveinis*).

 Upstream bug :mysqlbug:`67983` was causing a memory leak on a filtered slave. Bug fixed :bug:`1042946` (*Sergei Glushchenko*).

 |Percona Server| would fail to install on a vanilla *Ubuntu* 12.04 server. Bug fixed :bug:`1103655` (*Ignacio Nin*).

 The master thread was doing dirty buffer pool flush list reads to make its adaptive flushing decisions. Fixed by acquiring the flush list mutex around the list scans. Bug fixed :bug:`1083058` (*Laurynas Biveinis*).

 Upstream changes made to improve |InnoDB| ``DROP TABLE`` performance were not adjusted for |XtraDB|. This could cause server assertion errors. Bugs fixed :bug:`934377`, bug :bug:`1111211`, bug :bug:`1116447` and :bug:`1110102` (*Laurynas Biveinis*).

 The |XtraDB| used to print the open read view list without taking the kernel mutex. Thus any list element might become invalid during its iteration. Fixed by taking the kernel mutex. Bug fixed :bug:`1101030` (*Laurynas Biveinis*).

 When option :variable:`innodb_flush_method=O_DIRECT` was set up, log bitmap files were created and treated as |InnoDB| data files for flushing purposes, which wasn't original intention. Bug fixed :bug:`1105709` (*Laurynas Biveinis*).

 ``INFORMATION_SCHEMA`` plugin name :variable:`innodb_changed_pages` serves also as a command line option, but it is also a prefix of another command line option :variable:`innodb_changed_pages_limit`. |MySQL| option handling would then shadow the former with the latter, resulting in start up errors. Fixed by renaming the :variable:`innodb_changed_pages_limit` option to :variable:`innodb_max_changed_pages`. Bug fixed :bug:`1105726` (*Laurynas Biveinis*).

 Time in slow query log was displayed incorrectly when :variable:`slow_query_log_timestamp_precision` variable was set to microseconds. Bug fixed :bug:`887928` (*Laurynas Biveinis*). 

 Writing bitmap larger than 4GB would cause write to fail. Also a write error for every bitmap page, except the first one, would result in a heap corruption. Bug fixed :bug:`1111226` (*Laurynas Biveinis*).

 Fixed the upstream bug :mysqlbug:`67504` that caused spurious duplicate key errors. Errors would happen if a trigger is fired while a slave was processing replication events for a table that is present only on slave server while there are updates on the replicated table on the master which is used in that trigger. For this to happen master needs to have more than one auto-increment table and the slave needs to have at least one of those tables specified in the :variable:`replicate-ignore-table`. Bug fixed :bug:`1068210` (*George Ormond Lorch III*).

 Fixed failing ``rpm`` builds, that were caused by missing files. Bug fixed :bug:`1099809` (*Alexey Bychko*).

 Fixed the upstream :mysqlbug:`68116` that caused the server crash with assertion error when |InnoDB| monitor with verbose lock info was used under heavy load. This bug is affecting only ``-debug`` builds. Bug fixed :bug:`1100178` (*Laurynas Biveinis*).

 :ref:`changed_page_tracking` wasn't compatible with :option:`innodb_force_recovery=6`. When starting the server log tracking initialization would fail. The server would abort on startup. Bug fixed :bug:`1083596` (*Laurynas Biveinis*).

 Newly created bitmap file would silently overwrite the old one if they had the same file name. Bug fixed :bug:`1111144` (*Laurynas Biveinis*). 

 A server would stop with an assertion error in I/O and AIO routines if large :variable:`innodb_log_block_size` value is used in the combination with changed page tracking. Bug fixed :bug:`1114612` (*Laurynas Biveinis*).

 ``Optimizer_fix`` patch has been removed from |Percona Server|. Bug fixed :bug:`986247` (*Stewart Smith*).

 |InnoDB| monitor was prefetching the data pages for printing lock information even if no lock information was going to be printed. Bug fixed :bug:`1100643` (*Laurynas Biveinis*).

 |InnoDB| and the query plan information were being logged even if they weren't enabled for the slow query log. Bug fixed :bug:`730173` (*Laurynas Biveinis*).

 Fixed the incorrect help text for :variable:`slow_query_log_timestamp_precision`. Bug fixed :bug:`1090965` (*Laurynas Biveinis*).

Other bug fixes: bug fixed :bug:`909376` (*Laurynas Biveinis*), bug fixed :bug:`1082437` (*Laurynas Biveinis*), bug fixed :bug:`1083669` (*Laurynas Biveinis*), bug fixed :bug:`1096904` (*Laurynas Biveinis*), bug fixed :bug:`1091712` (*Laurynas Biveinis*), bug fixed :bug:`1096899` (*Laurynas Biveinis*), bug fixed :bug:`1088954` (*Laurynas Biveinis*), bug fixed :bug:`1096895` (*Laurynas Biveinis*), bug fixed :bug:`1092142` (*Laurynas Biveinis*), bug fixed :bug:`1090874` (*Laurynas Biveinis*), bug fixed :bug:`1089961` (*Laurynas Biveinis*), bug fixed :bug:`1088867` (*Laurynas Biveinis*), bug fixed :bug:`1089031` (*Laurynas Biveinis*), bug fixed :bug:`1108874` (*Laurynas Biveinis*).
