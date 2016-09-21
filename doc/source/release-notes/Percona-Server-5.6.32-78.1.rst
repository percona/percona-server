.. rn:: 5.6.32-78.1

============================
|Percona Server| 5.6.32-78.1
============================

Percona is glad to announce the release of |Percona Server| 5.6.32-78.1 on
September 21st, 2016 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.32-78.1/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.32
<http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-32.html>`_, including
all the bug fixes in it, |Percona Server| 5.6.32-78.1 is the current GA release
in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source
and free, all the details of the release can be found in the `5.6.32-78.1
milestone at Launchpad
<https://launchpad.net/percona-server/+milestone/5.6.32-78.1>`_.

Bugs Fixed
==========

 Limiting ``ld_preload`` libraries to be loaded from specific directories in
 ``mysqld_safe`` didn't work correctly for relative paths. Bug fixed
 :bug:`1624247`.

 Fixed possible privilege escalation that could be used when running ``REPAIR
 TABLE`` on a ``MyISAM`` table. Bug fixed :bug:`1624397`.

 The general query log and slow query log cannot be written to files ending in
 :file:`.ini` and :file:`.cnf` anymore. Bug fixed :bug:`1624400`.

 Implemented restrictions on symlinked files (:file:`error_log`,
 :file:`pid_file`) that can't be used with ``mysqld_safe``. Bug fixed
 :bug:`1624449`.

Other bugs fixed: :bug:`1553938`.
