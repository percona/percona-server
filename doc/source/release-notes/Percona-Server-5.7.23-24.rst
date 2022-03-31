.. rn:: 5.7.23-24

================================================================================
Percona Server 5.7.23-24
================================================================================

Percona announces the release of `Percona Server for MySQL
<https://www.percona.com/software/percona-server>`_ 5.7.23-24 on November 9,
2018 (downloads are available `here
<https://www.percona.com/downloads/Percona-Server-5.7/>`_ and from the `Percona
Software Repositories
<https://www.percona.com/doc/percona-server/5.7/installation.html#installing-from-binaries>`_).
This release merges changes of `MySQL 5.7.23
<https://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-23.html>`_, including
all the bug fixes in it. Percona Server for MySQL 5.7.23-24 is now the current
GA release in the 5.7 series. All of Percona’s software is open-source and free.

This release introduces InnoDB encryption improvements and merges upstream
MyRocks changes. Also, the usage of column families in MyRocks has been
improved. The InnoDB encryption improvements are in **Alpha** quality and are
not recommended to be used in production.

New Features
================================================================================

- :psbug:`4905`: Upstream MyRocks changes have been merged up to `prod201810` tag
- :psbug:`4976`: `InnoDB Undo Log Encryption` has been implemented
- :psbug:`4946`: Add the ``rocksdb_no_create_column_family`` option to prevent the implicit creation of column families in MyRocks
- :psbug:`4556`: `InnoDB Redo log` has been implemented
- :psbug:`3839`: `InnoDB Data Scrubbing` has been implemented
- :psbug:`3834`: `InnoDB Log Scrubbing` has been implemented

Bugs Fixed
================================================================================

- :psbug:`4723`: ``PURGE CHANGED_PAGE_BITMAPS`` did not work when ``innodb_data_home_dir`` was used
- :psbug:`4937`: ``rocksdb_update_cf_options`` was ignored when specified in ``my.cnf`` or on command line
- :psbug:`1107`: The binlog could be corrupted when tmpdir got full
- :psbug:`4834`: The encrypted system tablespace could have an empty uuid
- :psbug:`3906`: The server instance could crash when running the ``ALTER`` statement

.. rubric:: Other bugs fixed

- :psbug:`4106`: "Assertion ``log.getting_synced`` failed in ``rocksdb::DBImpl::MarkLogsSynced(uint64_t, bool, const rocksdb::Status&)``"
- :psbug:`4930`: "main.percona_log_slow_innodb: Result content mismatch"
- :psbug:`4811`: "5.7 Merge and fixup for old DB-937 introduces possible regression"
- :psbug:`4705`: "crash on snapshot size check in RocksDB"

Find the release notes for Percona Server for MySQL 5.7.23-24 in our `online documentation <https://www.percona.com/doc/percona-server/5.7/release-notes/Percona-Server-5.7.23-24.html>`_. Report
bugs in the `Jira bug tracker <https://jira.percona.com/projects/PS>`_.
