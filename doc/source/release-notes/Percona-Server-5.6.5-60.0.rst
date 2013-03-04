.. rn:: 5.6.5-60.0

============================
 |Percona Server| 5.6.5-60.0
============================

Percona is glad to announce the ALPHA release of |Percona Server| 5.6.5-60.0 on August 15, 2012 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.5-60.0/>`_ and from the EXPERIMENTAL `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.6.5 <http://dev.mysql.com/doc/refman/5.6/en/news-5-6-5.html>`_, including all the bug fixes in it, |Percona Server| 5.6.5-60.0 is the first ALPHA release in the 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.5-60.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.5-60.0>`_.

This release does NOT include all the features and fixes in Percona Server 5.5. A list of features included is provided below.

Features Included
=================

  * Fixed upstream |MySQL| bug :mysqlbug:`49336`, mysqlbinlog couldn't handle ``stdin`` when "|" was used. Bug fixed: :bug:`933969` (*Sergei Glushchenko*). 
  * Added the ``TIME_MS`` column in the :table:`PROCESSLIST` table
  * :ref:`mysqldump_ignore_create_error` feature has been ported from |Percona Server| 5.5
  * :variable:`fast_index_creation` feature has been ported from |Percona Server| 5.5
  * :ref:`temp_tables` information have been added to the ``INFORMATION_SCHEMA`` database

Features removed
================

  * The :variable:`optimizer_fix` variable from |Percona Server| 5.5 will not be present in 5.6
