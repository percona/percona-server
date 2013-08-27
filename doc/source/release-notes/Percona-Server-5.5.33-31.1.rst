.. rn:: 5.5.33-31.1

==============================
 |Percona Server| 5.5.33-31.1 
==============================

Percona is glad to announce the release of |Percona Server| 5.5.33-31.1 on August 27th, 2013. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.33-31.1/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.33 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-33.html>`_, including all the bug fixes in it, |Percona Server| 5.5.33-31.1 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.33-31.1 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.33-31.1>`_. 

Bugs Fixed
==========

 The buffer pool mutex split patch implemented in |Percona Server| could cause a race condition, involving a dirty compressed page block for which there is an uncompressed page image in the buffer pool, that could lead to a server crash. Bug fixed :bug:`1086680`.

 If binary log was enabled, :ref:`Fake Changes <innodb_fake_changes_page>` transactions were binlogged. This could lead to data corruption issues with deeper replication topologies. Bug fixed :bug:`1190580`.

 Changes made to the ``RPM`` scripts for previous |Percona Server| version caused installer to fail if there were different :term:`datadir` options in multiple configuration files. Bug fixed :bug:`1201036`.

 |Percona Server| ``shared-compat`` package was being built with the 5.1.66 version of the client, which didn't work with ``OpenSSL``. Fixed by building the ``shared-compat`` package with a more recent version. Bug fixed :bug:`1201393`.

 Fixed the upstream bug :mysqlbug:`69639` which caused compile errors for |Percona Server| with ``DTrace version Sun D 1.11`` provided by recent ``SmartOS`` versions. Bug fixed :bug:`1196460`.

 Fixed a regression introduced in |Percona Server| :rn:`5.5.32-31.0`, where server wouldn't be able to start if :ref:`atomic_fio` was enabled. Bug fixed :bug:`1214735`.

 |Percona Server| used to acquire the buffer pool ``LRU`` list mutex in the I/O completion routine for the compressed page flush list flushes where it was not necessary. Bug fixed :bug:`1181269`.

Other bugs fixed: bug fixed :bug:`1189743`, bug fixed :bug:`1188162` and bug fixed :bug:`1203308`.
