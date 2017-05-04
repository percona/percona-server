.. rn:: 5.5.52-38.3

============================
|Percona Server| 5.5.52-38.3
============================

Percona is glad to announce the release of |Percona Server| 5.5.52-38.3 on
October 4th, 2016. Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.52-38.3/>`_
and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.52
<http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-52.html>`_, including
all the bug fixes in it, |Percona Server| 5.5.52-38.3 is now the current stable
release in the 5.5 series. All of |Percona|'s software is open-source and free,
all the details of the release can be found in the `5.5.52-38.3 milestone at
Launchpad <https://launchpad.net/percona-server/+milestone/5.5.52-38.3>`_.

Bugs Fixed
==========

 ``mysql_upgrade`` now does not binlog its actions by default. To restore the
 previous behavior, use ``--write-binlog`` option. Bug fixed :bug:`1065841`
 (upstream :mysqlbug:`56155`).

 :ref:`audit_log_plugin` would hang when trying to write a log record of
 :variable:`audit_log_buffer_size` length. Bug fixed :bug:`1588439`.

 After fixing bug :bug:`1540338`, system table engine validation check is no
 longer skipped for tables that don't have an explicit ``ENGINE`` clause in
 a ``CREATE TABLE`` statement. If |MySQL| upgrade statements are replicated,
 and slave does not have the |MyISAM| set as a default storage engine, then
 the ``CREATE TABLE mysql.server`` statement would attempt to create an
 |InnoDB| table and fail because ``mysql_system_tables.sql`` script omitted
 explicit engine setting for this table. Bug fixed :bug:`1600056`.

 :ref:`audit_log_plugin` malformed record could be written after
 :variable:`audit_log_flush` was set to ``ON`` in ``ASYNC`` and ``PERFORMANCE``
 modes. Bug fixed :bug:`1613650`.

 :table:`INFORMATION_SCHEMA.TABLES` (or other schema info table) table query
 running in parallel with :table:`INFORMATION_SCHEMA.GLOBAL_TEMPORARY_TABLES`
 query may result in TABLES-query thread context having a mutex locked twice,
 or unlocked twice, or left locked, resulting in crashes or hangs. Bug fixed
 :bug:`1614849`.

Other bugs fixed: :bug:`1626002` (upstream :mysqlbug:`83073`), :bug:`904714`,
:bug:`1098718`, :bug:`1610102`, :bug:`1610110`, :bug:`1613663`, :bug:`1613728`,
:bug:`1613986`, :bug:`1614885`, :bug:`1615959`, :bug:`1616091`, :bug:`1616753`,
:bug:`1616768`, :bug:`1616937`, :bug:`1617150`, :bug:`1617323`, :bug:`1618478`,
:bug:`1618718`, :bug:`1618811`, :bug:`1618819`, :bug:`1619547`, :bug:`1619572`,
:bug:`1619665`, :bug:`1620200`, :bug:`1626458`, :bug:`1626500`, and
:bug:`1628417`.
