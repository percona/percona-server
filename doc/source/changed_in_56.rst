.. _changed_in_56:

=============================
Changed in Percona Server 5.6
=============================

|Percona Server| 5.6 is based on MySQL 5.6 and incorporates many of the improvements found in |Percona Server| 5.5.

Some features that were present in |Percona Server| 5.5 have been removed in |Percona Server| 5.6. These are:

 * `fast_index_creation <http://www.percona.com/doc/percona-server/5.5/management/innodb_fast_index_creation.html>`_ (use MySQL 5.6's ALGORITHM= option instead)
 * `HandlerSocket <http://www.percona.com/doc/percona-server/5.5/performance/handlersocket.html>`_ (may return when HandlerSocket supports MySQL 5.6)
 * SHOW [GLOBAL] TEMPORARY TABLES functionality is now only available via the INFORMATION_SCHEMA tables TEMPORARY_TABLES and GLOBAL_TEMPORARY_TABLES.
 * `InnoDB timer-based Concurrency Throttling <http://www.percona.com/doc/percona-server/5.5/performance/innodb_thread_concurrency_timer_based.html>`_
 * `InnoDB Recovery Stats <http://www.percona.com/doc/percona-server/5.5/management/innodb_recovery_patches.html>`_

Some features that were present in |Percona Server| 5.5 have been replaced by a different implementation of the same/similar functionality in |Percona Server| 5.6. These are:

 * SHOW INNODB STATUS section "OLDEST VIEW" has been replaced by the XTRADB_READ_VIEW INFORMATION_SCHEMA table.
 * SHOW INNODB STATUS sections on memory usage for InnoDB/XtraDB hash tables has been replaced by the XTRADB_INTERNAL_HASH_TABLES INFORMATION_SCHEMA table.
 * The INNODB_RSEG table has been renamed to XTRADB_RSEG

Some |Percona Server| 5.5 features have been replaced by similar or equivalent MySQL 5.6 features, so we now keep the MySQL 5.6 implementations in |Percona Server| 5.6. These are:

 * `Show Lock Names <http://www.percona.com/doc/percona-server/5.5/diagnostics/innodb_show_lock_names.html>`_ has been replaced by PERFORMANCE_SCHEMA
 * `Crash-Resistant Replication <http://www.percona.com/doc/percona-server/5.5/reliability/crash_resistant_replication.html>`_ has been replaced by |MySQL| crash safe replication
 * `Improved InnoDB I/O Scalability <http://www.percona.com/doc/percona-server/5.5/scalability/innodb_io_55.html>`_ patches have been replaced by improvements and changes in MySQL 5.6, although Percona may make improvements in the future
 * `InnoDB Data Dictionary Size Limit <http://www.percona.com/doc/percona-server/5.5/management/innodb_dict_size_limit.html>`_ has been replaced by |MySQL| 5.6 using the existing table-definition-cache variable to limit the size of the |InnoDB| data dictionary.
 * `Expand Table Import <http://www.percona.com/doc/percona-server/5.5/management/innodb_expand_import.html>`_ has been replaced by |MySQL| "InnoDB transportable tablespaces"
 * The |InnoDB| data dictionary INFORMATION_SCHEMA tables have been superseded by the |MySQL| implementations 
 * |XtraDB| SYS_STATS persistent table and index statistics has been replaced by the MySQL 5.6 implementation
 * `Dump/Restore of the Buffer Pool <http://www.percona.com/doc/percona-server/5.5/management/innodb_lru_dump_restore.html>`_ is now available in |MySQL| 5.6, so we have replaced the |Percona Server| implementation with the MySQL one.
