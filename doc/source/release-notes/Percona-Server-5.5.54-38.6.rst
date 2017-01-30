.. rn:: 5.5.54-38.6

============================
|Percona Server| 5.5.54-38.6
============================

Percona is glad to announce the release of |Percona Server| 5.5.54-38.6 on
February 1st, 2017. Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.54-38.6/>`_
and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.54
<http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-54.html>`_, including
all the bug fixes in it, |Percona Server| 5.5.54-38.6 is now the current stable
release in the 5.5 series. All of |Percona|'s software is open-source and free,
all the details of the release can be found in the `5.5.54-38.6 milestone at
Launchpad <https://launchpad.net/percona-server/+milestone/5.5.54-38.6>`_.

Bugs Fixed
==========

 Fixed new compilation warnings with GCC 6. Bugs fixed :bug:`1641612` and
 :bug:`1644183`.

 ``CONCURRENT_CONNECTIONS`` column in the :table:`USER_STATISTICS` table was
 showing incorrect values. Bug fixed :bug:`728082`.

 :ref:`audit_log_plugin` when set to ``JSON`` format was not escaping
 characters properly. Bug fixed :bug:`1548745`.

 ``mysqld_safe`` now limits the use of ``rm`` and ``chown`` to avoid privilege
 escalation. ``chown`` can now be used only for :file:`/var/log` directory. Bug
 fixed :bug:`1660265`.

Other bugs fixed: :bug:`1638897`, :bug:`1644174`, :bug:`1644547`, and
:bug:`1644558`.
