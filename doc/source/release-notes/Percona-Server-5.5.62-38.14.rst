.. rn:: 5.5.62-38.14

================================================================================
|Percona Server| |release|
================================================================================

Percona announces the release of `Percona Server for MySQL
<https://www.percona.com/software/mysql-database/percona-server>`_ |release| on
November 21, 2018 (downloads are available `here
<https://www.percona.com/downloads/Percona-Server-5.5/>`_ and from the `Percona
Software Repositories
<https://www.percona.com/doc/percona-server/5.5/installation.html#installing-from-binaries>`_). This
release merges changes of MySQL 5.5.62, including all the bug fixes in
it. Percona Server for MySQL |release| is now the current GA release in the 5.5
series. All of Percona’s software is open-source and free.

Note that Percona Server for MySQL |release| is the last release of the 5.5
series. This series goes EOL on December 1st, 2018.

Improvements
================================================================================

- :psbug:`4790`: The accuracy of user statistics has been improved

Bugs Fixed
================================================================================

- The binary log could be corrupted when the disk partition used for temporary
  files (tmpdir system variable) had little free space. Bug fixed :psbug:`1107`
- ``PURGE CHANGED_PAGE_BITMAPS`` did not work when the ``innodb_data_home_dir`` system
  variable was used. Bug fixed :psbug:`4723`

.. rubric:: Other Bugs Fixed

- :psbug:`4773`: Percona Server sources can't be compiled without server.
- :psbug:`4781`: ``sql_yacc.yy`` uses ``SQLCOM_SELECT`` instead of ``SQLCOM_SHOW_XXXX_STATS``

Find the release notes for Percona Server for MySQL |release| in our `online
documentation
<https://www.percona.com/doc/percona-server/5.5/release-notes/Percona-Server-5.5.62-38.14.html>`_. Report
bugs in the `Jira bug tracker <https://jira.percona.com/projects/PS>`_.


.. |release| replace:: 5.5.62-38.14
