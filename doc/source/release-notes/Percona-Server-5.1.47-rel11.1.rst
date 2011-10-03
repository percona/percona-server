.. rn:: 5.1.47-rel11.1

===============================
|Percona Server| 5.1.47-rel11.1
===============================

New features
=============

  * |Percona Server| is now based on |MySQL| 5.1.47, and |XtraDB| is now based on |InnoDB| plugin 1.0.8.

  * |XtraDB| now uses the fast recovery code released in |InnoDB| Plugin version 1.0.8, instead of Percona's earlier fast-recovery code.

  * Added the :variable:`percona_innodb_doublewrite_path` place the double-write-buffer into its own file (issue :bug:`584299`).

  * Added the :variable:`suppress_log_warning_1592` option to disable logging of error code 1592.

  * Added the :variable:`microseconds_in_slow_query_log` option to use microsecond precision for the slow query log's timestamps (issue :bug:`358412`).

  * Added the :variable:`use_global_log_slow_control` option to control slow-query logging globally without restarting, similar to :variable:`use_global_long_query_time`.

  * Added the :variable:`query_cache_strip_comments` option to strip comments from query before using it in query cache.

  * Added a global :variable:`innodb_deadlocks` counter to ``SHOW STATUS``, based on `a patch by Eric Bergen <http://ebergen.net/patches/innodb_deadlock_count.patch>`_ (issue :bug:`569288`, issue :bug:`590624`).

  * Added more tests to the |MySQL| test framework.

Fixed bugs
===========

  * :bug:`598576` - query_cache_with_comments feature crash the server

  * :bug:`573100` - Can't compile 5.1.46

  * :bug:`573104` - separate name in ``INNODB_SYS_TABLES`` table

  * :bug:`580324` - Security bug in upstream 

  * :bug:`586579` -|Percona Server| 11 fails to compile with ``CFLAGS=-DUNIV_DEBUG`` 

  * :bug:`569156` - CentOS 5: ``mysql-server`` conflicts with ``mysql-server`` 

  * :bug:`589639` - Recovery process may hang when tablespaces are deleted during the recovery 

  * :bug:`570840` - ``deb`` package conflicts with ``libdbd-mysql-perl`` 
