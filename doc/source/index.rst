.. Percona Server documentation master file, created by
   sphinx-quickstart on Mon Aug  8 01:24:46 2011.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. _dochome:

============================================
 |Percona Server| |version| - Documentation
============================================

Percona Server for MySQL is a free, fully compatible, enhanced, and open source drop-in replacement for any MySQL database. It provides superior performance, scalability, and instrumentation.

Percona Server for MySQL is trusted by thousands of enterprises to provide better performance and concurrency for their most demanding workloads. It delivers higher value to MySQL server users with optimized performance, greater performance scalability and availability, enhanced backups, and increased visibility.

Introduction
============

.. toctree::
   :maxdepth: 1
   :glob:

   percona_xtradb
   ps-versions-comparison
   feature_comparison
   changed_in_57

Installation
============

.. toctree::
   :maxdepth: 2
   :glob:

   installation
   upgrading_guide_56_57
   installation/post-installation

Scalability Improvements
========================

.. toctree::
   :maxdepth: 1
   :glob:

   scalability/innodb_split_buf_pool_mutex
   scalability/innodb_io

Performance Improvements
========================

.. toctree::
   :maxdepth: 1
   :glob:

   performance/aio_page_requests
   performance/query_cache_enhance
   performance/query_limit_records
   performance/innodb_numa_support
   performance/threadpool
   performance/xtradb_performance_improvements_for_io-bound_highly-concurrent_workloads
   performance/prefix_index_queries_optimization

Flexibility Improvements
========================

.. toctree::
   :maxdepth: 1
   :glob:

   flexibility/log_warnings_suppress
   flexibility/improved_memory_engine
   flexibility/max_binlog_files
   flexibility/extended_mysqldump
   flexibility/extended_select_into_outfile
   flexibility/per_query_variable_statement
   flexibility/extended_mysqlbinlog
   flexibility/slowlog_rotation
   flexibility/csv_engine_mode
   flexibility/proxy_protocol_support
   flexibility/per_session_server-id
   flexibility/compressed_columns
   flexibility/innodb_fts_improvements
   flexibility/binlogging_replication_improvements

Reliability Improvements
========================

.. toctree::
   :maxdepth: 1
   :glob:

   reliability/log_connection_error
   reliability/innodb_corrupt_table_action

Management Improvements
=======================

.. toctree::
   :maxdepth: 1
   :glob:

   management/udf_percona_toolkit
   management/innodb_kill_idle_trx
   management/enforce_engine
   management/utility_user
   management/expanded_program_option_modifiers
   management/changed_page_tracking
   management/pam_plugin
   management/innodb_expanded_fast_index_creation
   management/backup_locks
   management/audit_log_plugin
   management/start_transaction_with_consistent_snapshot
   management/extended_show_grants
   management/ssl-improvement
   management/utility_user
   management/ps-admin

Security Improvements
======================

.. toctree::
   :maxdepth: 1
   :glob:

   security/data-at-rest-encryption
   security/data-masking
   security/ssl-improvement
   security/pam_plugin


Diagnostics Improvements
========================

.. toctree::
   :maxdepth: 1
   :glob:

   diagnostics/user_stats
   diagnostics/slow_extended
   diagnostics/innodb_show_status
   diagnostics/show_engines
   diagnostics/process_list
   diagnostics/misc_info_schema_tables
   diagnostics/thread_based_profiling
   diagnostics/scalability_metrics_plugin
   diagnostics/response_time_distribution
   diagnostics/innodb_fragmentation_count
   diagnostics/libcoredumper
   diagnostics/stacktrace


TokuDB
======

.. toctree::
   :maxdepth: 1
   :glob:

   tokudb/tokudb_intro
   tokudb/tokudb_installation
   tokudb/using_tokudb
   tokudb/fast_updates
   tokudb/tokudb_files_and_file_types
   tokudb/tokudb_file_management
   tokudb/tokudb_background_analyze_table
   tokudb/tokudb_variables
   tokudb/tokudb_status_variables
   tokudb/tokudb_fractal_tree_indexing
   tokudb/tokudb_troubleshooting
   tokudb/tokudb_performance_schema
   tokudb/toku_backup
   tokudb/tokudb_faq
   tokudb/removing_tokudb

Percona MyRocks
===============

.. toctree::
   :maxdepth: 1
   :glob:

   Introduction <myrocks/index>
   Installation <myrocks/install>
   Limitations <myrocks/limitations>
   Differences <myrocks/differences>
   Server Variables <myrocks/variables>
   Status Variables <myrocks/status_variables>
   myrocks/gap_locks_detection
   myrocks/data_loading

Reference
=========

.. toctree::
   :maxdepth: 1
   :glob:

   upstream-bug-fixes
   ps-variables
   development
   trademark-policy
   index_info_schema_tables
   faq
   copyright
   release-notes/release-notes_index
   glossary

* :ref:`genindex`
* :ref:`modindex`
