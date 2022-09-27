.. Percona Server documentation master file, created by
   sphinx-quickstart on Mon Aug  8 01:24:46 2011.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. _dochome:

================================================================================
|Percona Server| |version| - Documentation
================================================================================

|Percona Server| is a free, fully compatible, enhanced, and open source drop-in replacement for any MySQL database. It provides superior performance, scalability, and instrumentation.

|Percona Server| is trusted by thousands of enterprises to provide better performance and concurrency for their most demanding workloads. It delivers higher value to MySQL server users with optimized performance, greater performance scalability and availability, enhanced backups, and increased visibility.

Introduction
================================================================================

.. toctree::
   :maxdepth: 1
   :glob:

   percona_xtradb
   ps-versions-comparison
   feature_comparison
   changed_in_version

Installation
================================================================================

.. toctree::
   :maxdepth: 2
   :glob:

   installation
   installation/post-installation

In-place upgrades
================================================================================

.. toctree::
   :maxdepth: 2
   :glob:

   upgrading_guide
   upgrading_using_percona_repos
   upgrading_tokudb_myrocks
   upgrading_using_standalone_packages

Run in Docker
================================================================================

.. toctree::
   :maxdepth: 1
   :glob:

   installation/docker

Scalability Improvements
================================================================================

.. toctree::
   :maxdepth: 1
   :glob:

   scalability/innodb_io

Performance Improvements
================================================================================

.. toctree::
   :maxdepth: 1
   :glob:

   performance/adaptive_network_buffers
   performance/aio_page_requests
   performance/threadpool
   performance/xtradb_performance_improvements_for_io-bound_highly-concurrent_workloads
   performance/prefix_index_queries_optimization
   performance/query_limit_records
   performance/jemalloc-profiling
   performance/procfs-plugin


Flexibility Improvements
================================================================================

.. toctree::
   :maxdepth: 1
   :glob:

   flexibility/binlogging_replication_improvements  
   flexibility/compressed_columns
    flexibility/extended_mysqlbinlog  
    flexibility/extended_mysqldump
   flexibility/extended_select_into_outfile  
   flexibility/extended_set_var
   flexibility/improved_memory_engine   
   flexibility/log_warnings_suppress
   flexibility/binlog_space
   flexibility/proxy_protocol_support
   flexibility/sequence_table
   flexibility/slowlog_rotation

Reliability Improvements
================================================================================

.. toctree::
   :maxdepth: 1
   :glob:

   reliability/log_connection_error
   reliability/innodb_corrupt_table_action

Management Improvements
================================================================================

.. toctree::
   :maxdepth: 1
   :glob:

   management/udf_percona_toolkit
   management/kill_idle_trx
   management/changed_page_tracking
   management/enforce_engine
   management/pam_plugin
   management/innodb_expanded_fast_index_creation
   management/backup_locks
   management/audit_log_plugin
   management/start_transaction_with_consistent_snapshot
   management/extended_show_grants
   management/utility_user

Security Improvements
================================================================================

.. toctree::
  :maxdepth: 1
  :glob:

  security/pam_plugin
  security/simple-ldap
  security/simple-ldap-variables
  security/selinux
  security/apparmor
  security/data-at-rest-encryption
  security/vault
  security/using-keyring-plugin
  security/using-kmip
  security/encryption-functions
  security/using-amz-kms
  security/rotating-master-key
  security/encrypting-tables
  security/encrypting-tablespaces
  security/encrypting-system-tablespace
  security/encrypting-temporary-files
  security/encrypting-binlogs
  security/encrypting-redo-log
  security/encrypting-undo-tablespace
  security/encrypting-threads
  security/encrypting-doublewrite-buffers
  security/verifying-encryption
  security/ssl-improvement
  security/data-masking
  security/system-variables

Diagnostics Improvements
================================================================================

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
   diagnostics/innodb_fragmentation_count
   diagnostics/stacktrace
   diagnostics/libcoredumper

Percona MyRocks
================================================================================

.. toctree::
   :maxdepth: 1
   :glob:

   MyRocks Introduction <myrocks/index>
   MyRocks Installation <myrocks/install>
   MyRocks Supported Features <myrocks/added-features>
   MyRocks Limitations <myrocks/limitations>
   MyRocks Differences <myrocks/differences>
   MyRocks Information Schema Tables <myrocks/information-schema-tables>
   MyRocks Server Variables <myrocks/variables>
   MyRocks Status Variables <myrocks/status_variables>
   MyRocks Gap Locks Detection <myrocks/gap_locks_detection>
   MyRocks Data Loading <myrocks/data_loading>
   MyRocks ZenFS <myrocks/zenfs>
   
TokuDB
================================================================================

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
   tokudb/tokudb_faq
   tokudb/removing_tokudb

Release notes
================================================================================

.. toctree::
   :maxdepth: 1
   :glob:

   release-notes/release-notes_index

Reference
================================================================================

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
   glossary

* :ref:`genindex`
* :ref:`modindex`
