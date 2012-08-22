.. rn:: 5.6.6-60.1

============================
 |Percona Server| 5.6.6-60.1
============================

Percona is glad to announce the ALPHA release of |Percona Server| 5.6.6-60.1 on August 27, 2012 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.6-60.1/>`_ and from the EXPERIMENTAL `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.6.6 <http://dev.mysql.com/doc/refman/5.6/en/news-5-6-6.html>`_, including all the bug fixes in it, |Percona Server| 5.6.6-60.1 is the second ALPHA release in the Percona Server 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.6-60.1 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.6-60.1>`_.

This release does NOT include all the features and fixes in Percona Server 5.5. A list of features included is provided below.

Features Included
=================

  * Bug fix for :bug:`933969`
  * TIME_MS column in INFORMATION_SCHEMA.PROCESSLIST
  * ignore-create-error option to mysqldump
  * INFORMATION_SCHEMA.TEMPORARY_TABLES and GLOBAL_TEMPORARY_TABLES

Features removed
================

  * The optimizer_fix variable from Percona Server 5.5 will not be present in 5.6

Changes Since 5.6.5-60.0
========================

 * fast_index_creation variable has been removed, it is replaced by the MySQL 5.6's ALGORITHM= option to ALTER TABLE
 * HandlerSocket has been removed from the tree. It was not built with the 5.6.5-60.0 binaries and may return when HandlerSocket supports MySQL 5.6.
 * SHOW [GLOBAL] TEMPORARY TABLES functionality is now only available via the TEMPORARY_TABLES and GLOBAL_TEMPORARY_TABLES INFORMATION_SCHEMA tables.
