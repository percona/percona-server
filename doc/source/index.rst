.. Percona Server documentation master file, created by
   sphinx-quickstart on Mon Aug  8 01:24:46 2011.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. _dochome:

===================================
 Percona Server 5.7 - Documentation
===================================

|Percona Server| is an enhanced drop-in replacement for |MySQL|. With |Percona Server|,

  * Your queries will run faster and more consistently.

  * You will consolidate servers on powerful hardware.

  * You will delay sharding, or avoid it entirely.

  * You will save money on hosting fees and power.

  * You will spend less time tuning and administering.

  * You will achieve higher uptime.

  * You will troubleshoot without guesswork.

Does this sound too good to be true? It's not. |Percona Server| offers breakthrough performance, scalability, features, and instrumentation. Its self-tuning algorithms and support for extremely high-performance hardware make it the clear choice for companies who demand the utmost performance and reliability from their database server.

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
   management/data_at_rest_encryption

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

TokuDB
======

.. toctree::
   :maxdepth: 1
   :glob:

   tokudb/tokudb_intro
   tokudb/tokudb_installation
   tokudb/using_tokudb
   tokudb/tokudb_files_and_file_types
   tokudb/tokudb_file_management
   tokudb/tokudb_background_analyze_table
   tokudb/tokudb_variables
   tokudb/tokudb_status_variables
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


