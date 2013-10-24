.. rn:: 5.6.13-61.0

==============================
 |Percona Server| 5.6.13-61.0
==============================

Percona is glad to announce the first GA (Generally Available) release of |Percona Server| 5.6.13-61.0 on October 7th, 2013 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.13-61.0/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.6.13 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-13.html>`_, including all the bug fixes in it, |Percona Server| 5.6.13-61.0 is the first GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.13-61.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.13-61.0>`_.

New Features
============
 
 |Percona Server| has implemented a number of :ref:`XtraDB performance improvements for I/O-bound high-concurrency workloads <xtradb_performance_improvements_for_io-bound_highly-concurrent_workloads>`. This feature fixes the upstream bug: :mysqlbug:`68555` (:bug:`1236884`).

 |Percona Server| has implemented a number of performance improvements for :ref:`page_cleaner_tuning`. This feature fixes the upstream bugs: :mysqlbug:`69170` (:bug:`1231918`), :mysqlbug:`70453` (:bug:`1232101`) and :mysqlbug:`68481` (:bug:`1232406`).

 ``ALL_O_DIRECT`` method for :variable:`innodb_flush_method` has been ported from |Percona Server| 5.5.

 :ref:`statement_timeout` feature has been ported from the Twitter branch.

 |Percona Server| has :ref:`extended <extended_select_into_outfile>` the ``SELECT INTO ... OUTFILE`` and ``SELECT INTO DUMPFILE`` to add the support for UNIX sockets and named pipes.
 
Bugs Fixed
==========

 Due to an incompatible upstream change that went in unnoticed, the page cleaner thread would attempt to replay any file operations it encountered. In most cases this were a no-op, but there were race conditions for certain DDL operations that would have resulted in server crash. Bug fixed :bug:`1217002`.

 ``apt-get upgrade`` of |Percona Server| would fail in post-installation step if server failed to start. Bug fixed :bug:`1002500`.

 |Percona Server| 5.6 now ships with memcached plugins. Bug fixed :bug:`1159621`.

 Fixed the ``libssl.so.6`` dependency issues in binary tarballs releases. Bug fixed :bug:`1172916`.

 Error in ``install_layout.cmake`` could cause that some library files, during the build, end up in different directories on x86_64 environment. Bug fixed :bug:`1174300`.
 
 Server would crash if empty string was passed to ``AES_ENCRYPT`` when older ``OpenSSL`` version was used. Upstream bug fixed :mysqlbug:`70489`, bug fixed :bug:`1201033`.
 
 :ref:`innodb_kill_idle_trx` feature didn't work correctly if :ref:`threadpool` was enabled. Bug fixed :bug:`1201440`.

 |Percona Server| :rn:`5.6.12-60.4` would crash if server was started with :ref:`threadpool` feature enabled. Bugs fixed :bug:`1201681`, :bug:`1194097` and :bug:`1201442`.

 Memory leak was introduced by the fix for bug :bug:`1132194`. Bug fixed :bug:`1204873`.

 A server could have crashed under a heavy I/O-bound workload involving compressed InnoDB tables. Bug fixed :bug:`1224432`.
 
 A potential deadlock, involving ``DDL``, ``SELECT``, ``SHOW ENGINE INNODB STATUS``, and ``KILL``, has been fixed. Fixed the upstream bug :mysqlbug:`60682`, bug fixed :bug:`1115048`.
 
 A memory leak in :ref:`psaas_utility_user` feature has been fixed. Bug fixed :bug:`1166638`.

 A server could crash due to a race condition between a :table:`INNODB_CHANGED_PAGES` query and bitmap file delete by ``PURGE CHANGED_PAGE_BITMAP`` or directly on the file system. Bug fixed :bug:`1191580`.

 |Percona Server| could not be built with :ref:`threadpool` feature and ``-DWITH_PERFSCHEMA_ENGINE=OFF`` option. Bug fixed :bug:`1196383`.

 Page cleaner should perform LRU flushing regardless of server activity. Fixed the upstream bug :mysqlbug:`70500`, bug fixed :bug:`1234562`.

 Fixed the upstream bug :mysqlbug:`64556` which could cause an unrelated warning to be raised if a query inside |InnoDB| was interrupted. Bug fixed :bug:`1115158`.
 
Other bugs fixed:  bug fixed :bug:`1131949`, bug fixed :bug:`1191589`, bug fixed :bug:`1229583`, upstream bug fixed :mysqlbug:`70490` bug fixed :bug:`1205196`,upstream bug fixed :mysqlbug:`70417` bug fixed :bug:`1230220`.
