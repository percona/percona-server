.. rn:: 5.5.61-38.13

================================================================================
|Percona Server| |release|
================================================================================

Percona announces the release of `Percona Server for MySQL
<https://www.percona.com/software/mysql-database/percona-server>`_ |release| on
August 17, 2018 (downloads are available `here
<https://www.percona.com/downloads/Percona-Server-5.5/>`_ and from the `Percona
Software Repositories
<https://www.percona.com/doc/percona-server/5.6/installation.html#installing-from-binaries>`_). This
release merges changes of `MySQL 5.5.61
<https://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-61.html>`_, including
all the bug fixes in it. Percona Server for MySQL |release| is now the current
GA release in the 5.5 series. All of Percona’s software is open-source and free.

Bugs Fixed
================================================================================

- The ``--innodb-optimize-keys`` option of the ``mysqldump`` utility fails when
  a column name is used as a prefix of a column which has the ``AUTO_INCREMENT``
  attribute. Bug fixed :psbug:`4524`.

Other Bugs Fixed:

- :psbug:`4566`: "``stack-use-after-scope`` in ``reinit_io_cache()``" (upstream :mysqlbug:`91603`)
- :psbug:`4581`: "``stack-use-after-scope`` in ``_db_enter_()`` / ``mysql_select_db()``" (upstream :mysqlbug:`91604`)
- :psbug:`4600`: "``stack-use-after-scope`` in ``_db_enter_()`` / ``get_upgrade_info_file_name()``" (upstream :mysqlbug:`91617`)
- :psbug:`3976`: "Errors in MTR tests ``main.variables-big``, ``main.information_schema-big``, ``innodb.innodb_bug14676111``"

Find the release notes for Percona Server for MySQL |release| in our `online
documentation
<https://www.percona.com/doc/percona-server/5.5/release-notes/Percona-Server-5.5.61-38.13.html>`_. Report
bugs in the `Jira bug tracker <https://jira.percona.com/projects/PS>`_.


.. |release| replace:: 5.5.61-38.13
