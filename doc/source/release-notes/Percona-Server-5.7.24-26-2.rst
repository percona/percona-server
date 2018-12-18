.. rn:: 5.7.24-26-2

========================
Percona Server |release|
========================

Percona is glad to announce the release of Percona Server |release| on
December 18, 2018. Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.24-26/>`_
and from the :doc:`Percona Software Repositories </installation>`.

This release is based on `MySQL 5.7.24
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-24.html>`_
and includes all the bug fixes in it. |Percona Server| |release| is
now the current GA (Generally Available) release in the 5.7 series.

All software developed by Percona is open-source and free.

Bugs Fixed
==========

* When uninstalling |Percona Server| packages on *CentOS 7* default configuration
  file :file:`my.cnf` would get removed as well. This fix makes the backup of
  the configuration file instead of removing it. Bug fixed :psbug:`5092`.

.. |release| replace:: 5.7.24-26
