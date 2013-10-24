.. rn:: 5.5.34-32.0

==============================
 |Percona Server| 5.5.34-32.0 
==============================

Percona is glad to announce the release of |Percona Server| 5.5.34-32.0 on October 28th, 2013. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.34-32.0/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.34 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-34.html>`_, including all the bug fixes in it, |Percona Server| 5.5.34-32.0 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.34-32.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.34-32.0>`_. 

New Features
============

 |Percona Server| has :ref:`extended <extended_select_into_outfile>` the ``SELECT INTO ... OUTFILE`` and ``SELECT INTO DUMPFILE`` to add the support for UNIX sockets and named pipes.

 |Percona Server| now provides additional information in the slow query log when :variable:`log_slow_rate_limit` variable is enabled.

 A new variable :variable:`slow_query_log_always_write_time` has been introduced. It can be used to specify an additional execution time threshold for the slow query log, that, when exceeded, will cause a query to be logged unconditionally, that is, :variable:`log_slow_rate_limit` will not apply to it.

 :ref:`psaas_utility_user` feature has been extended by adding a new :variable:`utility_user_privileges` that allows a comma separated value list of extra access privileges that can be granted to the utility user.

Bugs Fixed
==========

 Due to an incompatible upstream change that went in unnoticed, the log tracker thread would attempt to replay any file operations it encountered. In most cases this were a no-op, but there were race conditions for certain ``DDL`` operations that would have resulted in server crash. Bug fixed :bug:`1217002`.

 ``apt-get upgrade`` of |Percona Server| would fail in post-installation step if server failed to start. Bug fixed :bug:`1002500`.

 Fixed the ``libssl.so.6`` dependency issues in binary tarballs releases. Bug fixed :bug:`1172916`.

 Error in ``install_layout.cmake`` could cause that some library files, during the build, end up in different directories on x86_64 environment. Bug fixed :bug:`1174300`.

 |Percona Server| could crash while accessing ``BLOB`` or ``TEXT`` columns in |InnoDB| tables if :ref:`innodb_fake_changes_page` was enabled. Bug fixed :bug:`1188168`.

 Memory leak was introduced by the fix for bug :bug:`1132194`. Bug fixed :bug:`1204873`.

 The unnecessary overhead from persistent |InnoDB| adaptive hash index latching has been removed, potentially improving stability of the :ref:`innodb_adaptive_hash_index_partitions_page` feature as well. Upstream bug fixed :mysqlbug:`70216`, bug fixed :bug:`1218347`.
 
 Fixed the incorrect dependency with ``libmysqlclient18-dev`` from |Percona Server| :rn:`5.5.33-31.1`. Bug fixed :bug:`1237097`.

 A memory leak in :ref:`psaas_utility_user` feature has been fixed. Bug fixed :bug:`1166638`.

 :ref:`expanded_option_modifiers` did not deallocate memory correctly. Bug fixed :bug:`1167487`.

 A server could crash due to a race condition between a :table:`INNODB_CHANGED_PAGES` query and a bitmap file delete by ``PURGE CHANGED_PAGE_BITMAP`` or directly on the file system. Bug fixed :bug:`1191580`.

 |Percona Server| could not be built with :ref:`threadpool` feature and ``-DWITH_PERFSCHEMA_ENGINE=OFF`` option. Bug fixed :bug:`1196383`.

 Building |Percona Server| with ``-DHAVE_PURIFY`` option would result in an error. Fixed by porting the ``close_socket`` function from |MariaDB|. Bug fixed :bug:`1203567`.

 Adaptive hash index memory size was incorrectly calculated in ``SHOW ENGINE INNODB STATUS`` and :variable:`Innodb_mem_adaptive_hash` status variable. Bug fixed :bug:`1218330`.

 Some :ref:`expanded_option_modifiers` didn't have an effect if they were specified in non-normalized way (:variable:`innodb_io_capacity` vs :variable:`innodb-io-capacity`). Bug fixed :bug:`1233294`.

 Enabling :ref:`enforce_engine` feature could lead to error on |Percona Server| shutdown. Bug fixed :bug:`1233354`.

 Storage engine enforcement (:variable:`enforce_storage_engine`) is now ignored when the server is started in either bootstrap or skip-grant-tables mode. Bug fixed :bug:`1236938`.

 Fixed the build warnings caused by :ref:`user_stats` code on non-Linux platforms. Bug fixed :bug:`711817`.

 Adaptive hash indexing partitioning code has been simplified, potentially improving performance. Bug fixed :bug:`1218321`.

Other bugs fixed: bug fixed :bug:`1239630`, bug fixed :bug:`1191589`, bug fixed :bug:`1200162`, bug fixed :bug:`1214449`, and bug fixed :bug:`1190604`.
