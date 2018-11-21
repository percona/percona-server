.. rn:: 5.6.41-84.1

================================================================================
|Percona Server| 5.6.41-84.1
================================================================================

Percona is glad to announce the release of |Percona Server| 5.6.41-84.1 on
August 17, 2018 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.41-84.1/>`_
and from the :doc:`Percona Software Repositories </installation>`).

This release merges changes of `MySQL 5.6.41
<http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-41.html>`_, including
all the bug fixes in it. Percona Server for MySQL 5.6.41-84.1 is now the current
GA release in the 5.6 series. All of Percona’s software is open-source and free.

Bugs Fixed
================================================================================

- A simple SELECT query on a table with CHARSET=euckr COLLATE=euckr_bin could
  return different results each time it was executed. Bug fixed :psbug:`4513`
  (upstream :mysqlbug:`91091`).
- Percona Server 5.6.39-83.1 could crash when altering an InnoDB table that has
  a full-text search index defined. Bug fixed :psbug:`3851` (upstream
  :mysqlbug:`68987`).

Other Bugs Fixed
================================================================================

- :psbug:`3325` "online upgrade GTID cause binlog damage in high write QPS
  situation"
- :psbug:`3976` "Errors in MTR tests main.variables-big,
  main.information_schema-big, innodb.innodb_bug14676111"
- :psbug:`4506` "Backport fixes from 8.0 for InnoDB memcached Plugin"

Find the release notes for Percona Server for MySQL 5.6.41-84.1 in our `online
documentation
<https://www.percona.com/doc/percona-server/5.5/release-notes/Percona-Server-5.6.41-84.1.html>`_.
Report bugs in the `Jira bug tracker <https://jira.percona.com/projects/PS>`_.
