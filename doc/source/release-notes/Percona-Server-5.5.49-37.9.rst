.. rn:: 5.5.49-37.9

==============================
 |Percona Server| 5.5.49-37.9
==============================

Percona is glad to announce the release of |Percona Server| 5.5.49-37.9 on May 19th, 2016. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.49-37.9/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.49 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-49.html>`_, including all the bug fixes in it, |Percona Server| 5.5.49-37.9 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.49-37.9 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.49-37.9>`_. 

Bugs Fixed
==========

 |Percona Server| is now built with system ``zlib`` library instead of the older bundled one. Bug fixed :bug:`1108016`.

 ``CREATE TABLE ... LIKE ...`` could create a system table with an unsupported enforced engine. Bug fixed :bug:`1540338`.

 Server will now show more descriptive error message when |Percona Server| fails with ``errno == 22 "Invalid argument"``, if :variable:`innodb_flush_method` was set to ``ALL_O_DIRECT``. Bug fixed :bug:`1578604`.

 ``apt-cache show`` command for ``percona-server-client`` was showing ``innotop`` included as part of the package. Bug fixed :bug:`1201074`.

 ``mysql-systemd`` would fail with PAM authentication and proxies due to regression introduced when fixing :bug:`1534825` in |Percona Server| :rn:`5.5.48-37.8`. Bug fixed :bug:`1558312`.

Other bugs fixed: :bug:`1578625` (upstream :mysqlbug:`81295`), bug fixed :bug:`1553166`, and bug fixed :bug:`1578303` (upstream :mysqlbug:`81324`).
