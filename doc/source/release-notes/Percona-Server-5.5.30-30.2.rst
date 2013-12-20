.. rn:: 5.5.30-30.2

==============================
 |Percona Server| 5.5.30-30.2 
==============================

Percona is glad to announce the release of |Percona Server| 5.5.30-30.2 on April 10th, 2013 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.30-30.2/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.5.30 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-30.html>`_, including all the bug fixes in it, |Percona Server| 5.5.30-30.2 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.30-30.2 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.30-30.2>`_. 

New Features
============

 |Percona Server| has implemented priority connection scheduling for the :ref:`threadpool`. (*Alexey Kopytov*) 

 |Percona Server| will now be shipped with the ``libjemalloc`` library. Benchmark showing the impact of memory allocators on |MySQL| performance can be found in this `blogpost <http://www.mysqlperformanceblog.com/2012/07/05/impact-of-memory-allocators-on-mysql-performance/>`_. (*Ignacio Nin*)

 This release of |Percona Server| has fixed a number of performance bugs. (*Alexey Kopytov*)

 :ref:`innodb_lazy_drop_table_page` has been removed and its controlling variable :variable:`innodb_lazy_drop_table` has been deprecated. Feature has been removed because the upstream ``DROP TABLE`` implementation has been improved. (*Laurynas Biveinis*)

Known Issues
============

 This release contains a regression introduced by the fix for bug :bug:`1131187`. The workaround is to disable the query cache. Regression is reported as bug :bug:`1170103`.

Bug Fixes
=========

 Due to parse error in the :file:`percona-server.spec` |Percona Server| rpm packages couldn't be built on *RHEL* 5 and *CentOS* 5. Bug fixed :bug:`1144777` (*Ignacio Nin*).

 When **mysqldump** was used with :option:`--innodb-optimize-keys` option it produced invalid SQL for cases when there was an explicitly named foreign key constraint which implied an implicit secondary index with the same name. Fixed by detecting such cases and omitting the corresponding secondary keys from deferred key creation optimization. Bug fixed :bug:`1081016` (*Alexey Kopytov*).

 |Percona Server| was built with YaSSL which could cause some of the programs that use it to crash. Fixed by building packages with OpenSSL support rather than the bundled YaSSL library. Bug fixed :bug:`1104977` (*Ignacio Nin*).

 Running a DDL statement while :variable:`innodb_lazy_drop_table` was enabled could cause assertion failure. Bugs fixed :bug:`1086227` and :bug:`1128848` (*Laurynas Biveinis*).

 Fixed yum dependencies that were causing conflicts in ``CentOS`` 6.3 during installation. Bugs fixed :bug:`1031427` and  :bug:`1051874` (*Ignacio Nin*).

 The log tracker thread was unaware of the situation when the oldest untracked log records are overwritten by the new log data. In some corner cases this could lead to assertion errors in the log parser or bad changed page data. Bug fixed :bug:`1108613` (*Laurynas Biveinis*).

 Ported a fix from *MariaDB* for the upstream bug :mysqlbug:`63144`. ``CREATE TABLE``  or ``CREATE TABLE IF NOT EXISTS`` statements on an existing table could wait on a metadata lock instead of failing or returning immediately if there is a transaction that executed a query which opened that table. Bug fixed :bug:`1127008` (*Sergei Glushchenko*).

 Fix for bug :bug:`1070856` introduced a regression in |Percona Server| :rn:`5.5.28-29.3` which could cause a server to hang when binary log is enabled. Bug fixed :bug:`1162085` (*Alexey Kopytov*).

 Fixed upstream bug :mysqlbug:`49169` by avoiding the ``malloc`` call in the ``read_view_create_low()`` in most cases. This significantly improves |InnoDB| scalability on read-only workloads, especially when the default glibc memory allocator is used. Bug fixed :bug:`1131187` (*Alexey Kopytov*).

 Removed ``trx_list`` scan in ``read_view_open_now()`` which is another problem originally reported as upstream bug :mysqlbug:`49169`. This also provides much better scalability in |InnoDB| high-concurrent workloads. Bugs fixed :bug:`1131189` (*Alexey Kopytov*).

 In the event that a slave was disconnected from the master, under certain conditions, upon reconnect, it would report that it received a packet larger than the :variable:`slave_max_allowed_packet` variable. Bug fixed :bug:`1135097` (*George Ormond Lorch III*).

 Fixed the upstream bug :mysqlbug:`62578` which caused |MySQL| client to abort the connections on terminal resize. Bug fixed :bug:`925343` (*Sergei Glushchenko*).

 |Percona Server| would re-create the test database when using ``rpm`` on server upgrade, even if the database was previously removed. Bug fixed :bug:`710799` (*Alexey Bychko*).

 Debian packages included the old version of **innotop**. Fixed by removing **innotop** and its ``InnoDBParser`` Perl package from source and Debian installation. Bug fixed :bug:`1032139` (*Alexey Bychko*).

 UDF/configure.ac was incompatible with ``automake`` 1.12. Bug fixed :bug:`1099387` (*Alexey Bychko*).

 Reduced the overhead from `innodb_pass_corrupt_table`` value checks by optimizing them for better CPU branch prediction. Bug fixed :bug:`1125248` (*Alexey Kopytov*).
 
 ``dialog.so`` used by the :ref:`pam_plugin` couldn't be loaded with Perl and Python clients when :variable:`plugin-dir` option was set in the ``[client]`` section of the :file:`my.cnf`. Bug fixed :bug:`1155859` (*Sergei Glushchenko*).

 Fixed the upstream bug :mysqlbug:`68845` which could unnecessarily increase contention on ``log_sys->mutex`` in write-intensive workloads. Bug fixed :bug:`1163439` (*Alexey Kopytov*).

 Ported back from the upstream |MySQL| 5.6 the fix for unnecessary ``log_flush_order_mutex`` acquisition. Bug fixed :bug:`1163262` (*Alexey Kopytov*).

 When **mysqldump** was used with :option:`--innodb-optimize-keys` and :option:`--no-data` options, all secondary key definitions would be lost. Bug fixed :bug:`989253` (*Alexey Kopytov*).

 Warning about the *Percona Toolkit* UDFs was omitted when installing from Percona's *Debian* repositories. Bug fixed :bug:`1015506` (*Alexey Bychko*).

 |Percona Server| was missing help texts in the |MySQL| client because the help tables were missing. Bug fixed :bug:`1041981` (*Alexey Bychko*).

 Fixed the upstream bug :mysqlbug:`68197` that caused |InnoDB| to misclassify internal read operations as synchronous when they were actually asynchronous when :ref:`threadpool` feature was used. Bug fixed :bug:`1107539` (*Sergei Glushchenko*).

 Suboptimal code for :ref:`user_stats` feature has been optimized to make sure no additional work is done when :variable:`userstat` is disabled. Bug fixed :bug:`1128066` (*Alexey Kopytov*).

Other bug fixes: bug fixed :bug:`1103850` (*Laurynas Biveinis*), bug fixed :bug:`1146621` (*Laurynas Biveinis*), bug fixed :bug:`1050536` (*Alexey Bychko*), bug fixed :bug:`1144059` (*Roel Van de Paar*), bug fixed :bug:`1154962` (*Hrvoje Matijakovic*), bug fixed :bug:`1154959` (*Hrvoje Matijakovic*), bug fixed :bug:`1154957` (*Hrvoje Matijakovic*), bug fixed :bug:`1154954` (*Hrvoje Matijakovic*).
