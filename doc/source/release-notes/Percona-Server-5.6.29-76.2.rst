.. rn:: 5.6.29-76.2

==============================
 |Percona Server| 5.6.29-76.2 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.29-76.2 on March 7th, 2016 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.29-76.2/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.29 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-29.html>`_, including all the bug fixes in it, |Percona Server| 5.6.29-76.2 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.29-76.2 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.29-76.2>`_.

Bugs Fixed
==========
 
 With :ref:`expanded_innodb_fast_index_creation` enabled, DDL queries involving |InnoDB| temporary tables would cause later queries on the same tables to produce warnings that their indexes were not found in the index translation table. Bug fixed :bug:`1233431`.

 Package upgrade on *Ubuntu* would run ``mysql_install_db`` even though data directory already existed. Bug fixed :bug:`1457614`.

 Package upgrade on *Ubuntu* and *Debian* could reset the ``GTID`` number sequence when post-install script was restarting the service. Bug fixed :bug:`1507812`. 

 Starting |MySQL| with ``systemctl`` would fail with timeout if the socket was specified with a custom path. Bug fixed :bug:`1534825`.
 
 Write-heavy workload with a small buffer pool could lead to a deadlock when free buffers are exhausted. Bug fixed :bug:`1521905`.

 :file:`libjemalloc.so.1` was missing from binary tarball. Bug fixed :bug:`1537129`.

 ``mysqldumpslow`` script has been removed because it was not compatible with |Percona Server| extended slow query log format. Please use `pt-query-digest <https://www.percona.com/doc/percona-toolkit/2.2/pt-query-digest.html>`_ from |Percona Toolkit| instead. Bug fixed :bug:`856910`.

 When ``cmake/make/make_binary_distribution`` workflow was used to produce binary tarballs it would produce tarballs with ``mysql-...`` naming instead of ``percona-server-...``. Bug fixed :bug:`1540385`.

 Cardinality of partitioned |TokuDB| tables became inaccurate after the changes introduced by :ref:`tokudb_background_analyze_table` feature in |Percona Server| :rn:`5.6.27-76.0`. Bug fixed :tokubug:`925`. 

 Running the ``TRUNCATE TABLE`` while :ref:`tokudb_background_analyze_table` is enabled could lead to a server crash once analyze job tries to access the truncated table. Bug fixed :tokubug:`938`.

 Added proper memory cleanup if for some reason a |TokuDB| table is unable to be opened from a dead closed state. This prevents an assertion from happening the next time the table is attempted to be opened. Bug fixed :tokubug:`917`.

Other bugs fixed: :tokubug:`898`, :bug:`1521120` and :bug:`1534246`.
