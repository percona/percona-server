.. _5.7.31-34:

================================================================================
*Percona Server for MySQL* 5.7.31-34
================================================================================
:Date: August 24, 2020
:Installation: `Installing Percona Server for MySQL <https://www.percona.com/doc/percona-server/5.7/installation.html>`_

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 5.7.31-34
includes all the features and bug fixes available in
`MySQL 5.7.31 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-31.html>`_
in addition to enterprise-grade features developed by Percona.

New Features
================================================================================

* :jirabug:`PS-7128`: Document RocksDB variables: :variable:`rocksdb_max_background_compactions`, :variable:`rocksdb_max_background_flushes`, and :variable:`rocksdb_max_bottom_pri_background_compactions`



Improvements
================================================================================

* :jirabug:`PS-7132`: Make default value of :variable:`rocksdb_wal_recovery_mode` compatible with InnoDB
* :jirabug:`PS-7199`: Add :ref:`Coredumper<libcoredumper>` functionality
* :jirabug:`PS-7114`: Enhance crash artifacts (core dumps and stack traces) to provide additional information to the operator



Bugs Fixed
================================================================================

* :jirabug:`PS-7203`: Fixed audit plugin memory leak on replicas when opening tables
* :jirabug:`PS-7043`: Correct constant equality expression is used in LEFT JOIN condition by setting the 'const_table' flag together with setting the row as a NULL-row. (Upstream :mysqlbug:`99499`)
* :jirabug:`PS-7212`: Modify processing to binary compare in order to do native JSON comparison (Upstream :mysqlbug:`100307`)
* :jirabug:`PS-7076`: Modify to not update Cardinality after setting :variable:`tokudb_cardinality_scale_percent`
* :jirabug:`PS-7025`: Fix reading ahead of insert buffer pages by dispatching of buffered AIO transfers (Upstream :mysqlbug:`100086`)
* :jirabug:`PS-7010`: Modify to Lock buffer blocks before sanity check in btr_cur_latch_leaves
* :jirabug:`PS-6995`: Introduce a new optimizer switch to allow the user to reduce the cost of a range scan to determine best execution plan for Primary Key lookup
* :jirabug:`PS-5978`: Remove unneeded check of variable to allow mysqld_safe support --numa-interleave (Thanks to user springlin for reporting this issue)
* :jirabug:`PS-7220`: Fix activity counter update in purge coordinator and workers
* :jirabug:`PS-7234`: Modify PS minimal tarballs to remove COPYING.AGPLv3
* :jirabug:`PS-7204`: Add checks to linkingscript to correct failures in patchelf
* :jirabug:`PS-7075`: Provide :ref:`binary tarball<installing_from_binary_tarball>` with shared libs and glibc suffix
* :jirabug:`PS-7062`: Modify ALTER INSTANCE ROTATE INNODB MASTER KEY to skip writing of redo for compressed encrypted temporary table.
* :jirabug:`PS-5263`: Update handle_binlog_flush_or_sync_error() to set my_ok(thd) after thd->clear_error() to correct assert in THD::send_statement_status (Upstream :mysqlbug:`93770`)
* :jirabug:`PS-4530`: Add documentation that ``ps-admin`` removes jemalloc and THP settings on :ref:`TokuDB uninstall<removing_tokudb>`


