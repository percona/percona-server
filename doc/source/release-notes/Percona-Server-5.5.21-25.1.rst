.. rn:: 5.5.21-25.1

==============================
 |Percona Server| 5.5.21-25.1
==============================

Percona is glad to announce the release of |Percona Server| 5.5.21-25.1 on March 30, 2012 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.21-25.1/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.5.21 <http://dev.mysql.com/doc/refman/5.5/en/news-5-5-21.html>`_, including all the bug fixes in it, |Percona Server| 5.5.21-25.1 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.21-25.1 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.21-25.1>`_.


Bug Fixes
=========

  * Fixed a memory corruption regression introduced in 5.5.18-23.0. Bug fixed :bug:`915814` (*Alexey Kopytov*).

  * Fixed InnoDB compilation warnings on CentOS 5. Bug fixed :bug:`962940` (*Laurynas Biveinis*).

  * Fixed MySQL upstream bug `#64160 <http://bugs.mysql.com/bug.php?id=64160>`_ that was causing issues on upgrade to 5.5.20 and 5.5.21. Bug fixed :bug:`966844` (*Stewart Smith*).
