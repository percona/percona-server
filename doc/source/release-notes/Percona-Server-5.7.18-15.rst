.. rn:: 5.7.18-15

==========================
|Percona Server| 5.7.18-15
==========================

Percona is glad to announce the GA (Generally Available) release of |Percona
Server| 5.7.18-15 on May 26, 2017 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.18-15/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.7.18
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-18.html>`_, including
all the bug fixes in it, |Percona Server| 5.7.18-15 is the current GA release
in the |Percona Server| 5.7 series. All of Percona's software is open-source
and free, all the details of the release can be found in the `5.7.18-15
milestone at
Launchpad <https://launchpad.net/percona-server/+milestone/5.7.18-15>`_

Bugs Fixed
==========

 Server would crash when querying partitioning table with a single partition.
 Bug fixed :bug:`1657941` (upstream :mysqlbug:`76418`).

 Running a query on InnoDB table with `ngram full-text parser
 <https://dev.mysql.com/doc/refman/5.7/en/fulltext-search-ngram.html>`_ and a
 ``LIMIT`` clause could lead to a server crash. Bug fixed :bug:`1679025`
 (upstream :mysqlbug:`85835`).
