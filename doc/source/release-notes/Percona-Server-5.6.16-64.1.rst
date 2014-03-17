.. rn:: 5.6.16-64.1

==============================
 |Percona Server| 5.6.16-64.1 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.16-64.1 on March 17th, 2014 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.16-64.1/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.16 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-16.html>`_, including all the bug fixes in it, |Percona Server| 5.6.16-64.1 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.16-64.1 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.16-64.1>`_.

Bugs Fixed
==========

 After installing the ``auth_socket`` plugin any local user might get root access to the server. If you're using this plugin upgrade is advised. This is a regression, introduced in |Percona Server| :rn:`5.6.11-60.3`. Bug fixed :bug:`1289599`

 The new client and server packages included files with paths that were conflicting with the ones in ``mysql-libs`` package on *CentOS*. Bug fixed :bug:`1278516`.

 A clean installation of ``Percona-Server-server-55`` on *CentOS* would fail due to a typo in ``mysql_install_db`` call. Bug fixed :bug:`1291247`.

 ``libperconaserverclient18.1`` *Debian*/*Ubuntu* packages depended on ``multiarch-support``, which is not available on all the supported distribution versions. Bug fixed :bug:`1291628`.
 
 The |InnoDB| file system mutex was being locked incorrectly if :ref:`atomic_fio` was enabled. Bug fixed :bug:`1287098`.

 Slave I/O thread wouldn't attempt to automatically reconnect to the master after a network time-out (``error: 1159``). Bug fixed :bug:`1268729` (upstream :mysqlbug:`71374`).

Renaming the ``libmysqlclient`` to ``libperconaserverclient``
=============================================================

This release fixes some of the issues caused by the ``libmysqlclient`` rename to ``libperconaserverclient`` in |Percona Server| :rn:`5.6.16-64.0`. The old name was conflicting with the upstream ``libmysqlclient``.

Except for packaging, ``libmysqlclient`` and ``libperconaserverclient`` of the same version do not have any differences. Users who previously compiled software against Percona-provided ``libmysqlclient`` will either need to install the corresponding package of their distribution, such as ``mysql-lib`` for *CentOS* and ``libmysqlclient18`` for *Ubuntu*/*Debian* or recompile against ``libperconaserverclient``. Another workaround option is to create a symlink from ``libperconaserverclient.so.18.0.0`` to ``libmysqlclient.so.18.0.0``.

