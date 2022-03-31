.. rn:: 5.7.11-4

=========================
|Percona Server| 5.7.11-4
=========================

Percona is glad to announce the GA (Generally Available) release of |Percona
Server| 5.7.11-4 on March 15th, 2016 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.11-4/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.7.11
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-11.html>`_, including
all the bug fixes in it, |Percona Server| 5.7.11-4 is the current GA release in
the |Percona Server| 5.7 series. All of Percona's software is open-source and
free, all the details of the release can be found in the `5.7.11-4 milestone at
Launchpad <https://launchpad.net/percona-server/+milestone/5.7.11-4>`_

New Features
============

 |Percona Server| has implemented :ref:`parallel_doublewrite_buffer`.

 :ref:`tokudb_background_analyze_table` feature is now enabled by default
 (:variable:`tokudb_analyze_in_background` is set to ``ON`` by default).
 Variable :variable:`tokudb_auto_analyze` default value has been changed from
 ``0`` to ``30``. (:tokubug:`935`)

 :ref:`log_warning_suppress` feature has been removed from |Percona Server| 5.7
 because MySQL 5.7.11 has implemented a new system variable,
 `log_statements_unsafe_for_binlog
 <https://dev.mysql.com/doc/refman/5.7/en/replication-options-binary-log.html#sysvar_log_statements_unsafe_for_binlog>`_,
 which implements the same effect.

Bugs Fixed
==========

 If ``pid-file`` option wasn't specified with the full path, *Ubuntu*/*Debian*
 ``sysvinit`` script wouldn't notice the server is actually running and it will
 timeout or in some cases even hang. Bug fixed :bug:`1549333`.

 Buffer pool may fail to remove dirty pages for a particular tablesspace from
 the flush list, as requested by, for example, ``DROP TABLE`` or ``TRUNCATE
 TABLE`` commands. This could lead to a crash. Bug fixed :bug:`1552673`.

 :ref:`audit_log_plugin` worker thread may crash on write call writing fewer
 bytes than requested. Bug fixed :bug:`1552682` (upstream :mysqlbug:`80606`).

 |Percona Server| 5.7 ``systemd`` script now takes the last option specified in
 :file:`my.cnf` if the same option is specified multiple times. Previously it
 would try to take all values which would break the script and server would
 fail to start. Bug fixed :bug:`1554976`.

 ``mysqldumpslow`` script has been removed because it was not compatible with
 |Percona Server| extended slow query log format. Please use `pt-query-digest
 <https://www.percona.com/doc/percona-toolkit/2.2/pt-query-digest.html>`_ from
 |Percona Toolkit| instead. Bug fixed :bug:`856910`.

Other bugs fixed: :bug:`1521120`, :bug:`1549301` (upstream :mysqlbug:`80496`),
and :bug:`1554043` (upstream :mysqlbug:`80607`).

