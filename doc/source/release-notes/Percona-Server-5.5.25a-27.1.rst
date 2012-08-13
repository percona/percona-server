.. rn:: 5.5.25a-27.1

===============================
 |Percona Server| 5.5.25a-27.1
===============================

Percona is glad to announce the release of |Percona Server| 5.5.25a-27.1 on July 20th, 2012 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.25a-27.1/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.5.25a <http://dev.mysql.com/doc/refman/5.5/en/news-5-5-25a.html>`_, including all the bug fixes in it, |Percona Server| 5.5.25a-27.1 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.25a-27.1 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.25a-27.1>`_. 

Features
========

  * |Percona Server| has extended standard behavior of variable :variable:`secure-file-priv`. When used with no argument, the `LOAD_FILE()` function will always return NULL. The `LOAD DATA INFILE` and `SELECT INTO OUTFILE` statements will fail with the following error: "The MySQL server is running with the :option:`--secure-file-priv` option so it cannot execute this statement". `LOAD DATA LOCAL INFILE` is not affected by the :option:`--secure-file-priv` option and will still work when it's used without an argument.
 
  * |Percona Server| now uses `thread based profiling <http://www.percona.com/doc/percona-server/5.5/diagnostics/thread_based_profiling.html>`_  by default, instead of process based profiling. This was implemented because with process based profiling, threads on the server, other than the one being profiled, can affect the profiling information.

Bug Fixes
=========

  * |Percona Server| 5.5.24 would crash if userstats were enabled with any replication configured. This was a regression introduced with ssl connections count in statistics tables in Percona Server :rn:`5.5.24-26.0`. Bug fixed :bug:`1008278` (*Vladislav Lesin*).

  * PAM authentication plugin was in different directories in 32bit and 64bit binary tarballs. Bug fixed :bug:`1007271` (*Ignacio Nin*).

  * Querying I_S.GLOBAL_TEMPORARY_TABLES or TEMPORARY_TABLES would crash threads working with temporary tables. Bug fixed :bug:`951588` (*Laurynas Biveinis*).

  * mysqld crash message wasn't pointing to |Percona Server| bugtracker. Bug fixed :bug:`1007254` (*Vadim Tkachenko*).

  * If the tablespace has been created with MySQL 5.0 or older, importing that table could crash |Percona Server| in some cases. Bug fixed :bug:`1000221` (*Alexey Kopytov*). 

  * Server started with :option:`skip-innodb` crashes on `SELECT * FROM INNODB_TABLE_STATS` or `INNODB_INDEX_STATS`. Bug fixed :bug:`896439` (*Stewart Smith*).

  * Fixed typo for :variable:`log_slow_verbosity` in the code. Bug fixed :bug:`987737` (*Stewart Smith*).

  * Removed some patch-based source code management leftovers from the bzr branch migration. Bug fixed :bug:`988383` (*Stewart Smith*).

  * Fixed upstream mysql bug `#60743 <http://bugs.mysql.com/bug.php?id=60743>`_, typo in cmake/dtrace.cmake that was making dtrace unusable. Bug fixed :bug:`1013455` (*Stewart Smith*).

Other bugfixes: bug :bug:`1022481` (*Ignacio Nin*) and bug :bug:`987348` (*Ignacio Nin*).
