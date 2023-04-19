.. _8.0.20-11:

================================================================================
*Percona Server for MySQL* 8.0.20-11
================================================================================

:Date: July 21, 2020
:Installation: `Installing Percona Server for MySQL <https://www.percona.com/doc/percona-server/8.0/installation.html>`_

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 8.0.20-11
includes all the features and bug fixes available in
`MySQL 8.0.20 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/8.0/en/news-8-0-20.html>`_
in addition to enterprise-grade features developed by Percona.

As of 8.0.20-11, the Percona Parallel Doublewrite buffer implementation has been removed and has been replaced with the Oracle MySQL implementation.

New Features
================================================================================

* :jirabug:`PS-7128`: Add RocksDB variables: :ref:`rocksdb_max_background_compactions`, :ref:`rocksdb_max_background_flushes`, and :ref:`rocksdb_max_bottom_pri_background_compactions`
* :jirabug:`PS-7039`: Add RocksDB variable: :ref:`rocksdb_validate_tables`
* :jirabug:`PS-6951`: Add RocksDB variables: :ref:`rocksdb_delete_cf`, :ref:`rocksdb_enable_iterate_bounds`, and :ref:`rocksdb_enable_remove_orphaned_dropped_cfs`
* :jirabug:`PS-6926`: Add RocksDB variables: :ref:`rocksdb_table_stats_recalc_threshold_pct`, :ref:`rocksdb_table_stats_recalc_threshold_count`, :ref:`rocksdb_table_stats_background_thread_nice_value`, :ref:`rocksdb_table_stats_max_num_rows_scanned`, :ref:`rocksdb_table_stats_use_table_scan`.
* :jirabug:`PS-6910`: Add RocksDB variable: :ref:`rocksdb_stats_level`.
* :jirabug:`PS-6902`: Add RocksDB variable: :ref:`rocksdb_enable_insert_with_update_caching`.
* :jirabug:`PS-6901`: Add RocksDB variable: :ref:`rocksdb_read_free_rpl`.
* :jirabug:`PS-6891`: Add RocksDB variable: :ref:`rocksdb_master_skip_tx_api`.
* :jirabug:`PS-6890`: Add RocksDB variable: :ref:`rocksdb_blind_delete_primary_key`.
* :jirabug:`PS-6886`: Add RocksDB variable: :ref:`rocksdb_cache_dump`.
* :jirabug:`PS-6885`: Add RocksDB variable: :ref:`rocksdb_rollback_on_timeout`.



Improvements
================================================================================

* :jirabug:`PS-6994`: Implement rocksdb_validate_tables functionality in Percona Server 8.X
* :jirabug:`PS-6984`: Update the zstd submodule to v1.4.4.
* :jirabug:`PS-5764`: Introduce SEQUENCE_TABLE() table-level SQL function



Bugs Fixed
================================================================================

* :jirabug:`PS-7019`: Correct query results for LEFT JOIN with GROUP BY (Upstream :mysqlbug:`99398`)
* :jirabug:`PS-6979`: Modify the processing to call clean up functions to remove CREATE USER statement from the processlist after the statement has completed (Upstream :mysqlbug:`99200`)
* :jirabug:`PS-6860`: Merge innodb_buffer_pool_pages_LRU_flushed into buf_get_total_stat()
* :jirabug:`PS-7038`: Set innodb-parallel-read_threads=1 to prevent kill process from hanging (Thanks to user wavelet123 for reporting this issue)
* :jirabug:`PS-6945`: Correct tokubackup plugin process exported API to allow large file backups. (Thanks to user prohaska7 for reporting this issue)
* :jirabug:`PS-7000`: Fix newer collations for proper space padding in MyRocks
* :jirabug:`PS-6991`: Modify package to include missing development files (Thanks to user larrabee for reporting this issue)
* :jirabug:`PS-6946`: Correct tokubackup processing to free memory use from the address and thread sanitizers (Thanks to user prohaska7 for reporting this issue)
* :jirabug:`PS-5893`: Add support for running multiple instances with systemD on Debian. (Thanks to user sasha for reporting this issue)
* :jirabug:`PS-5620`: Modify Docker image to support supplying custom TLS certificates (Thanks to user agarner for reporting this issue)
* :jirabug:`PS-7168`: Determine if file per tablespace using table flags to prevent assertion
* :jirabug:`PS-7161`: Fixed 'CreateTempFile' gunit test to support both 'HAVE_O_TMPFILE'-style
* :jirabug:`PS-7142`: Set 'KEYRING_VAULT_PLUGIN_OPT' value when required
* :jirabug:`PS-7138`: Correct file reference for ps-admin broken in tar.gz package
* :jirabug:`PS-7127`: Provide mechanism to grant dynamic privilege to the utility user.
* :jirabug:`PS-7118`: Add ability to set LOWER_CASE_TABLE_NAMES option before initializing data directory
* :jirabug:`PS-7116`: Port MyRocks fix of Index Condition Pushdown (ICP)
* :jirabug:`PS-7075`: Provide binary tarball with shared libs and glibc suffix
* :jirabug:`PS-6974`: Correct instability in the rocksdb.drop_cf_* tests
* :jirabug:`PS-6969`: Correct instability in the rocksdb.index_stats_large_table
* :jirabug:`PS-6105`: Modify innodb.mysqld_core_dump_without_buffer_pool_dynamic test to move assertion to correct location
* :jirabug:`PS-5735`: Correct package to install the charsets on CentOS 7
* :jirabug:`PS-4757`: Remove CHECK_IF_CURL_DEPENDS_ON_RTMP to build keyring_vault for unconditional test
* :jirabug:`PS-7131`: Improve resume_encryption_cond conditional variable handling to avoid missed signals
* :jirabug:`PS-7100`: Fix rocksdb_read_free_rpl test to properly count rows corresponding to broken index entries
* :jirabug:`PS-7082`: Correct link displayed on \help client command
* :jirabug:`PS-7169`: Set rocksdb_validate_tables to disabled RocksDB while upgrading the server from 5.7 to 8.0.20