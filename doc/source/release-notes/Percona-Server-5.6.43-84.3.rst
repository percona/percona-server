.. rn:: 5.6.43-84.3

================================================================================
|Percona Server| |release|
================================================================================

Percona is glad to announce the release of |Percona Server| |release| on |date|
(Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.43-84.3/>`_
and from the :doc:`Percona Software Repositories </installation>`).

This release merges changes of `MySQL 5.6.43
<http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-43.html>`_, including
all the bug fixes in it. |Percona Server| |release| is now the current GA
release in the 5.6 series. All of Percona’s software is open-source and free.

Bugs Fixed
================================================================================

- A sequence of LOCK TABLES FOR BACKUP and STOP SLAVE SQL_THREAD could cause
  replication to be blocked and not possible to be restarted normally. Bug fixed
  :psbug:`4758` (upstream :mysqlbug:`93649`).
- **http** was replaced with **https** in http://bugs.percona.com in server
  crash messages. Bug fixed :psbug:`4855`.
- Wrong query results could be received in semi-join subqueries with
  materialization-scan that allowed inner tables of different semijoin
  nests to interleave. Bug fixed :psbug:`4907` (upstream bug
  :mysqlbug:`92809`).
- The audit logs could be corrupted when the :option:`audit_log_rotations`
  option was changed at runtime. Bug fixed :psbug:`4950`.
- There was a typo in ``mysqld_safe.sh``: **trottling** was replaced with
  **throttling**. Bug fixed :psbug:`240`. Thanks to Michael Coburn for the
  patch.

Other bugs fixed:
:psbug:`2477`,
:psbug:`3535`,
:psbug:`3568`,
:psbug:`3672`,
:psbug:`3673`,
:psbug:`4989`,
:psbug:`5100`,
:psbug:`5118`,
:psbug:`5163`,
:psbug:`5268`,
:psbug:`5270`,
:psbug:`5271`.

This release also contains fixes for the following CVE issues:
CVE-2019-2534,
CVE-2019-2529,
CVE-2019-2482,
CVE-2019-2455,
CVE-2019-2503,
CVE-2018-0734.

Find the release notes for |Percona Server| |release| in our `online
documentation
<https://www.percona.com/doc/percona-server/5.6/release-notes/Percona-Server-5.6.43-84.3.html>`_.
Report bugs in the `Jira bug tracker <https://jira.percona.com/projects/PS>`_.

.. |release| replace:: 5.6.43-84.3
.. |date| replace:: January 29, 2019
