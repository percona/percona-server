.. _5.7.30-33:

================================================================================
*Percona Server for MySQL* 5.7.30-33
================================================================================

:Date: May 20, 2020
:Installation: `Installing Percona Server for MySQL <https://www.percona.com/doc/percona-server/5.7/installation.html>`_

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 5.7.30-33
includes all the features and bug fixes available in
`MySQL 5.7.30 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-30.html>`_
in addition to enterprise-grade features developed by Percona.

Merged MyRocks/RocksDB up to Facebook MySQL production tag fb-prod201907.

New Features
================================================================================

* :jirabug:`PS-6951`: Document new RocksDB variables: rocksdb_delete_cf, rocksdb_enable_iterate_bounds, and rocksdb_enable_remove_orphaned_dropped_cfs
* :jirabug:`PS-4464`: Expose the last global transaction identifier (GTID) executed for a CONSISTENT SNAPSHOT.
* :jirabug:`PS-6926`: Document RocksDB variables: rocksdb_table_stats_recalc_threshold_pct, rocksdb_table_stats_recalc_threshold_count, rocksdb_table_stats_background_thread_nice_value, rocksdb_table_stats_max_num_rows_scanned, rocksdb_table_stats_use_table_scan. rocksdb_table_stats_background_thread_nice_value,  rocksdb_table_stats_max_num_rows_scanned,  rocksdb_table_stats_use_table_scan, and rocksdb_trace_block_cache_access.
* :jirabug:`PS-6901`: Document RocksDB variable: rocksdb_read_free_rpl.
* :jirabug:`PS-6890`: Document RocksDB variable: rocksdb_blind_delete_primary_key.
* :jirabug:`PS-6885`: Document the new variable rocksdb_rollback_on_timeout which allows the rollback of an entire transaction on timeout.
* :jirabug:`PS-6891`: Document RocksDB variable: rocksdb_master_skip_tx_api.
* :jirabug:`PS-6886`: Document variable rocksdb_cache_dump which includes RocksDB block cache content in a core dump.
* :jirabug:`PS-6910`: Document RocksDB variable: rocksdb_stats_level.



Improvements
================================================================================

* :jirabug:`PS-6984`: Update the zstd submodule to v1.4.4.



Bugs Fixed
================================================================================

* :jirabug:`PS-6979`: Modify the processing to call clean up functions to remove CREATE USER statement from the processlist after the statement has completed (Upstream :mysqlbug:`99200`)
* :jirabug:`PS-6860`: Merge innodb_buffer_pool_pages_LRU_flushed into buf_get_total_stat()
* :jirabug:`PS-6811`: Correct service failure of asserting ACL_PROXY_USER when skip-name-resolve=1 and there is a Proxy user (Upstream :mysqlbug:`98908`)
* :jirabug:`PS-6112`: Correct Binlog_snapshot_gtid inconsistency when mysqldump was used with --single-transaction.
* :jirabug:`PS-6945`: Correct tokubackup plugin process exported API to allow large file backups.
* :jirabug:`PS-6856`: Correct binlogs corruptions in PS 5.7.28 and 5.7.29 (Upstream :mysqlbug:`97531`)
* :jirabug:`PS-6946`: Correct tokubackup processing to free memory use from the address and thread sanitizers
* :jirabug:`PS-5893`: Add support for running multiple instances with systemD on Debian.
* :jirabug:`PS-5620`: Modify Docker image to support supplying custom TLS certificates
* :jirabug:`PS-4573`: Implement use of a single config file - mysqld.cnf file.
* :jirabug:`PS-7041`: Correct Compilation error when -DWITH_EDITLINE=bundled is used
* :jirabug:`PS-7020`: Modify MTR tests for Ubuntu 20.04 to include python2 (python 2.6 or higher) and python3
* :jirabug:`PS-6974`: Correct instability in the rocksdb.drop_cf_* tests
* :jirabug:`PS-6969`: Correct instability in the rocksdb.index_stats_large_table
* :jirabug:`PS-6954`: Correct tokudb-backup-plugin to avoid collision between -std=c++11 and -std=gnu++03.
* :jirabug:`PS-6925`: Correct mismatched default socket values for mysqld and mysqld_safe
* :jirabug:`PS-6899`: Correct main.events_bugs and main.events_1 to interpret date 01-01-2020 properly (Upstream :mysqlbug:`98860`)
* :jirabug:`PS-6796`: Correct instability in percona_changed_page_bmp_shutdown_thread
* :jirabug:`PS-6773`: Initialize values in sha256_password_authenticate (Upstream :mysqlbug:`98223`)
* :jirabug:`PS-5844`: Fix a memory leak after 'innodb.alter_crash' in 'prepare_inplace_alter_table_dict()' (Upstream :mysqlbug:`96472`)
* :jirabug:`PS-5735`: Correct 5.7 package to install the charsets on CentOS 7
* :jirabug:`PS-4757`: Remove CHECK_IF_CURL_DEPENDS_ON_RTMP to build keyring_vault for unconditional test
* :jirabug:`PS-4649`: Document PerconaFT in TokuDB which is fractal tree indexing to enhance the B-tree data structure


