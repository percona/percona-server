.. rn:: 5.5.54-38.7

============================
|Percona Server| 5.5.54-38.7
============================

Percona is glad to announce the release of |Percona Server| 5.5.54-38.7 on
March 22nd, 2017. Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.54-38.7/>`_
and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.54
<http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-54.html>`_, including
all the bug fixes in it, |Percona Server| 5.5.54-38.7 is now the current stable
release in the 5.5 series. All of |Percona|'s software is open-source and free,
all the details of the release can be found in the `5.5.54-38.7 milestone at
Launchpad <https://launchpad.net/percona-server/+milestone/5.5.54-38.7>`_.

Bugs Fixed
==========

 Log tracking initialization did not find last valid bitmap data correctly,
 potentially resulting in needless redo log retracking or hole in the tracked
 LSN range. Bug fixed :bug:`1658055`.

Other bugs fixed: :bug:`1652912`, and :bug:`1655587`.
