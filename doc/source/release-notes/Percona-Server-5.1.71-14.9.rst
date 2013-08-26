.. rn:: 5.1.71-14.9

==============================
 |Percona Server| 5.1.71-14.9 
==============================

Percona is glad to announce the release of |Percona Server| 5.1.71-14.9 on August 26th, 2013 (Downloads are available from `Percona Server 5.1.71-14.9 downloads <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.71-14.9/>`_ and from the `Percona Software Repositories <http://www.percona.com/doc/percona-server/5.1/installation.html>`_).

Based on `MySQL 5.1.71 <http://dev.mysql.com/doc/relnotes/mysql/5.1/en/news-5-1-71.html>`_, this release will include all the bug fixes in it. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.1.71-14.9 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.1.71-14.9>`_.

Bugs Fixed
==========

 The buffer pool mutex split patch implemented in |Percona Server| could cause a race condition, involving a dirty compressed page block for which there is an uncompressed page image in the buffer pool, that could lead to a server crash. Bug fixed :bug:`1086680`.

 The buffer pool mutex split patch implemented in |Percona Server| could cause server crash with an assertion error on read-write workloads with compressed tables in debug builds. Bug fixed :bug:`1103850`.

 If binary log was enabled, :ref:`Fake Changes <innodb_fake_changes_page>` transactions were binlogged. This could lead to data corruption issues with deeper replication topologies. Bug fixed :bug:`1190580`.

 Changes made to the ``RPM`` scripts for previous |Percona Server| version caused installer to fail if there were different :term:`datadir` options in multiple configuration files. Bug fixed :bug:`1201036`.

 |Percona Server| used to acquire the buffer pool ``LRU`` list mutex in the I/O completion routine for the compressed page flush list flushes where it was not necessary. Bug fixed :bug:`1181269`.

Other bug fixes: bug fixed :bug:`1188162` and bug fixed :bug:`1203308`.
