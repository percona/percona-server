.. rn:: 5.6.16-64.2

==============================
 |Percona Server| 5.6.16-64.2 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.16-64.2 on March 25th, 2014 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.16-64.2/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.16 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-16.html>`_, including all the bug fixes in it, |Percona Server| 5.6.16-64.2 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.16-64.2 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.16-64.2>`_.

Bugs Fixed
==========

 The upgrade to |Percona Server| :rn:`5.6.16-64.1` would silently comment out any options in :file:`my.cnf` that have paths specified that contain ``share/mysql``. Bug fixed :bug:`1293867`.

 |Percona Server| could fail to start after upgrade if the :option:`lc-messages-dir` option was set in the :file:`my.cnf` configuration file. Bug fixed :bug:`1294067`.

 Dependency on ``mysql-common`` package, introduced in |Percona Server| :rn:`5.6.16-64.0` could lead to wrongly chosen packages for upgrade, spurious removes and installs with some combination of packages installed which use the mysql libraries. Bug fixed :bug:`1294211`.

 These three bugs were fixed by removing the dependency on ``mysql-common`` package.

 :ref:`udf_percona_toolkit` were missing from *Debian*/*Ubuntu* packages, this regression was introduced in |Percona Server| :rn:`5.6.16-64.0`. Bug fixed :bug:`1296416`.

 |Percona Server| installer will create the symlinks from ``libmysqlclient`` to ``libperconaserverclient`` during the installation on *CentOS*. This was implemented in order to provide the backwards compatibility after the ``libmysqlclient`` library has been renamed to ``libperconaserverclient`` .

