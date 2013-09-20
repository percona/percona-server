.. rn:: 5.6.13-60.5

==============================
 |Percona Server| 5.6.13-60.5
==============================

Percona is glad to announce the third Release Candidate release of |Percona Server| 5.6.13-60.5 on August 28th, 2013 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.13-60.5/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.6.13 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-13.html>`_, including all the bug fixes in it, |Percona Server| 5.6.13-60.5 is the third RC release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.13-60.5 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.13-60.5>`_.

Ported Features
===============

 :ref:`buff_read_ahead_area` has been ported from |Percona Server| 5.5

Bug Fixes
==========

 If binary log was enabled, :ref:`Fake Changes <innodb_fake_changes_page>` transactions were binlogged. This could lead to data corruption issues with deeper replication topologies. Bug fixed :bug:`1190580`.

 Querying :table:`INFORMATION_SCHEMA.PARTITIONS` could cause key distribution statistics for partitioned tables to be reset to those corresponding to the last partition. Fixed the upstream bug :mysqlbug:`69179`. Bug fixed :bug:`1192354`.

 Changes made to the ``RPM`` scripts for previous |Percona Server| version caused installer to fail if there were different :term:`datadir` options in multiple configuration files. Bug fixed :bug:`1201036`.

 Fixed the upstream bug :mysqlbug:`42415` that would cause ``UPDATE/DELETE`` statements with the ``LIMIT`` clause to be unsafe for Statement Based Replication even when ``ORDER BY`` primary key was present. Fixed by implementing an algorithm to do more elaborate analysis on the nature of the query to determine whether the query will cause uncertainty for replication or not. Bug fixed :bug:`1132194`.

 When an upgrade was performed between major versions (e.g. by uninstalling a 5.1 RPM and then installing a 5.5 one), ``mysql_install_db`` was still called on the existing data directory which lead to re-creation of the ``test`` database. Bug fixed :bug:`1169522`.

 Fixed the upstream bug :mysqlbug:`69639` which caused compile errors for |Percona Server| with ``DTrace version Sun D 1.11`` provided by recent ``SmartOS`` versions. Bug fixed :bug:`1196460`.
 
 Fixed a regression introduced in |Percona Server| :rn:`5.6.12-60.4`, where server wouldn't be able to start if :ref:`atomic_fio` was enabled. Bug fixed :bug:`1214735`.

Other bugs fixed: bug fixed :bug:`1188162`, bug fixed :bug:`1203308` and bug fixed :bug:`1189743`.
