.. rn:: 5.5.59-38.11

=============================
|Percona Server| 5.5.59-38.11
=============================

Percona is glad to announce the release of |Percona Server| 5.5.59-38.11 on
January 30th, 2018. Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.59-38.11/>`_
and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.59
<http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-59.html>`_, including
all the bug fixes in it, |Percona Server| 5.5.59-38.11 is now the current
stable release in the 5.5 series. All of |Percona|'s software is open-source
and free.

Bugs Fixed
==========

 With :variable:`innodb_large_prefix` set to ``1``, Blackhole storage engine
 was incompatible with InnoDB table definitions, thus adding new indexes would
 cause replication errors on the slave. Fixed :psbug:`1126` (upstream
 :mysqlbug:`53588`).

 A GCC 7 warning fix introduced introduced regression in |Percona Server|
 :rn:`5.5.58-38.10` that lead to a wrong SQL query built to access the remote
 server when Federated storage engine was used.
 Bug fixed :bug:`1134`.

 |Percona Server| 5.5 embedded server builds were broken.
 Bug fixed :psbug:`2893`.

 |Percona Server| now uses *TraviCI* for additional tests.
 Bug fixed :psbug:`3777`.

Other bugs fixed: :psbug:`257` and :psbug:`2415`.
