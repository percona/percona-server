.. rn:: 5.6.5-60.0

============================
 |Percona Server| 5.6.5-60.0
============================

Percona is glad to announce the ALPHA release of |Percona Server| 5.6.5-60.0 on August 15, 2012 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.5-60.0/>`_ and from the EXPERIMENTAL `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.6.5 <http://dev.mysql.com/doc/refman/5.6/en/news-5-6-5.html>`_, including all the bug fixes in it, |Percona Server| 5.6.5-60.0 is the first ALPHA release in the 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.5-60.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.5-60.0>`_.

This release does NOT include all the features and fixes in Percona Server 5.5. A list of features included is provided below.

Features Included
=================

  * Bug fix for :bug:`933969`
  * TIME_MS column in INFORMATION_SCHEMA.PROCESSLIST
  * ignore-create-error option to mysqldump
  * control online alter index
  * SHOW TEMP patch

Features removed
================

  * The optimizer_fix variable from Percona Server 5.5 will not be present in 5.6
