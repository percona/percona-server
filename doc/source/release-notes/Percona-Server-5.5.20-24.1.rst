.. rn:: 5.5.20-24.1

==============================
 |Percona Server| 5.5.20-24.1
==============================

Percona is glad to announce the release of |Percona Server| 5.5.20-24.1 on February 9th, 2012 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.20-24.1/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.5.20 <http://dev.mysql.com/doc/refman/5.5/en/news-5-5-20.html>`_, including all the bug fixes in it, |Percona Server| 5.5.20-24.1 is now the current stable release in the 5.5 series. All of |Percona| 's software is open-source and free, all the details of the release can be found in the `5.5.20-24.1 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.20-24.1>`_.


Bug Fixes
=========

  * GCC 4.6 has expanded diagnostics. New warnings reported: :bug:`878164` (*Laurynas
    Biveinis*).
  * Dependency issue while installing libmysqlclient15-dev on Ubuntu systems: :bug:`803151` (*Ignacio Nin*).
  * Dependency issues for libmysqlclient*-dev package(s) on Debian: :bug:`656933` (*Ignacio Nin*).
  * HandlerSocket failed to compile if the package mysql-devel 5.0 is installed on RHEL5 :bug:`922768`  (*Ignacio Nin*).
