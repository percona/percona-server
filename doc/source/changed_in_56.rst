.. _changed_in_56:

=============================
Changed in Percona Server 5.6
=============================

|Percona Server| 5.6 is based on MySQL 5.6 and incorporates many of the improvements found in |Percona Server| 5.5.

Some features that were present in |Percona Server| 5.5 have been removed in |Percona Server| 5.6. These are:

 * optimizer_fix
 * fast_index_creation (use MySQL 5.6's ALGORITHM= option instead)
 * HandlerSocket (may return when HandlerSocket supports MySQL 5.6)
 * SHOW [GLOBAL] TEMPORARY TABLES functionality is now only available via the INFORMATION_SCHEMA tables TEMPORARY_TABLES and GLOBAL_TEMPORARY_TABLES.
 * thread concurrency timer based
 * innodb_recovery_stats
 * innodb_show_lock_name.patch - replaced by PERFORMANCE_SCHEMA

Some features that were present in |Percona Server| 5.5 have been replaced by a different implementation of the same/similar functionality in |Percona Server| 5.6. These are:

 * SHOW INNODB STATUS section "OLDEST VIEW" has been replaced by the XTRADB_READ_VIEW INFORMATION_SCHEMA table.
 * SHOW INNODB STATUS sections on memory usage for InnoDB/XtraDB hash tables has been replaced by the XTRADB_INTERNAL_HASH_TABLES INFORMATION_SCHEMA table.
 * The INNODB_RSEG table has been renamed to XTRADB_RSEG

Some |Percona Server| 5.5 features have been replaced by similar or equivalent MySQL 5.6 features, so we now keep the MySQL 5.6 implementations in |Percona Server| 5.6. These are:

 * innodb_recovery_update_relay_log (replaced by MySQL crash safe replication)
 * innodb_io_patches (replaced by improvements and changes in MySQL 5.6, although Percona may make improvements in the future)
 * innodb_dict_size_limit has been replaced by MySQL 5.6 using the existing table-definition-cache variable to limit the size of the InnoDB data dictionary.
 * split innodb buffer pool mutex. This has been replaced by improvements in upstream InnoDB
 * expanded IMPORT TABLESPACE has been replaced by MySQL "InnoDB transportable tablespaces"
 * The InnoDB data dictionary INFORMATION_SCHEMA tables have been superseded by the MySQL implementations 
 * XtraDB SYS_STATS persistent table and index statistics has been replaced by the MySQL 5.6 implementation
 * lru dump/restore is now available in MySQL 5.6, so we have replaced the Percona Server implementation with the MySQL one.
