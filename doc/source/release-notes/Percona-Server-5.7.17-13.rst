.. rn:: 5.7.17-13

==========================
|Percona Server| 5.7.17-13
==========================

Percona is glad to announce the GA (Generally Available) release of |Percona
Server| 5.7.17-13 on April 5th, 2017 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.17-13/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.7.17
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-17.html>`_, including
all the bug fixes in it, |Percona Server| 5.7.17-12 is the current GA release
in the |Percona Server| 5.7 series. All of |Percona|'s software is open-source
and free, all the details of the release can be found in the `5.7.17-13
milestone at
Launchpad <https://launchpad.net/percona-server/+milestone/5.7.17-13>`_

Bugs Fixed
==========

 *MyRocks* storage engine detection implemented in ``mysqldump`` in |Percona
 Server| :rn:`5.6.17-12` was using a deprecated
 :table:`INFORMATION_SCHEMA.SESSION_VARIABLES` table, causing ``mysqldump``
 failures on servers running with :variable:`show_compatibility_56` variable
 set to ``OFF``. Bug fixed :bug:`1676401`.
