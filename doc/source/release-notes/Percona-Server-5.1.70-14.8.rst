.. rn:: 5.1.70-14.8

==============================
 |Percona Server| 5.1.70-14.8 
==============================

Percona is glad to announce the release of |Percona Server| 5.1.70-14.8 on July 3rd, 2013 (Downloads are available from `Percona Server 5.1.70-14.8 downloads <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.70-14.8/>`_ and from the `Percona Software Repositories <http://www.percona.com/doc/percona-server/5.1/installation.html>`_).

Based on `MySQL 5.1.70 <http://dev.mysql.com/doc/relnotes/mysql/5.1/en/news-5-1-70.html>`_, this release will include all the bug fixes in it. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.1.70-14.8 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.1.70-14.8>`_.

Bugs Fixed
==========

 Prevented a race condition that could lead to a server crash when querying the :table:`INFORMATION_SCHEMA.INNODB_BUFFER_PAGE` table. Bug fixed :bug:`1072573`.

 When an upgrade was performed between major versions (e.g. by uninstalling a 5.1 RPM and then installing a 5.5 one), ``mysql_install_db`` was still called on the existing data directory which lead to re-creation of the ``test`` database. Bug fixed :bug:`1169522`.

 Fixed the upstream bug :mysqlbug:`68354` that could cause server to crash when performing update or join on ``Federated`` and ``MyISAM`` tables with one row, due to a bug in the ``Federated`` storage engine. Bug fixed :bug:`1182572`.
 
Other bug fixes: bug fixed :bug:`1191395`.
