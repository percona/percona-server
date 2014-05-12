.. rn:: 5.6.13-60.6

==============================
 |Percona Server| 5.6.13-60.6
==============================

Percona is glad to announce the fourth Release Candidate release of |Percona Server| 5.6.13-60.6 on September 20th, 2013 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.13-60.6/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.6.13 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-13.html>`_, including all the bug fixes in it, |Percona Server| 5.6.13-60.6 is the fourth RC release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.13-60.6 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.13-60.6>`_.

New Features
============
 
 :ref:`innodb_split_buf_pool_mutex` feature has been ported from |Percona Server| 5.5. This feature splits the single global InnoDB buffer pool mutex into several mutexes. The goal of this change is to reduce mutex contention, which can be very impacting when the working set does not fit in memory.

 :ref:`innodb_adaptive_hash_index_partitions_page` feature has been ported from |Percona Server| 5.5. This feature splits the adaptive hash index across several partitions and decreases the AHI latch contention. This feature fixes the upstream bug :mysqlbug:`62018` (:bug:`1216804`).
 
 :ref:`psaas_utility_user` feature has been extended by adding a new :variable:`utility_user_privileges` that allows a comma separated value list of extra access privileges that can be granted to the utility user.

 |Percona Server| now provides additional information in the slow query log when :variable:`log_slow_rate_limit` variable is enabled.

 A new variable :variable:`slow_query_log_always_write_time` has been introduced. It can be used to specify an additional execution time threshold for the slow query log, that, when exceeded, will cause a query to be logged unconditionally, that is, :variable:`log_slow_rate_limit` will not apply to it.

Bugs Fixed
==========

 The unnecessary overhead from persistent InnoDB adaptive hash index latching has been removed, potentially improving stability of the :ref:`innodb_adaptive_hash_index_partitions_page` feature as well. Upstream bug fixed :mysqlbug:`70216`, bug fixed :bug:`1218347`.

 Adaptive hash index memory size was incorrectly calculated in ``SHOW ENGINE INNODB STATUS`` and :table:`XTRADB_INTERNAL_HASH_TABLES`. Bug fixed :bug:`1218330`.

 An unnecessary buffer pool mutex acquisition has been removed, potentially improving performance. Upstream bug fixed :mysqlbug:`69258`, bug fixed :bug:`1219842`.

 Fixed the build warnings caused by :ref:`user_stats` code on non-Linux platforms. Bug fixed :bug:`711817`.

 Adaptive hash indexing partitioning code has been simplified, potentially improving performance. Bug fixed :bug:`1218321`.

Other bugs fixed: upstream bug fixed :mysqlbug:`69617` bug fixed :bug:`1216815`, upstream bug fixed :mysqlbug:`70228` bug fixed :bug:`1220544`. 
