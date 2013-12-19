.. Percona Server documentation master file, created by
   sphinx-quickstart on Mon Aug  8 01:24:46 2011.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. _dochome:

================================
 Percona Server - Documentation
================================

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

Installation
============

.. toctree::
   :maxdepth: 1
   :glob:

   installation
   upgrading_guide_51_55


Scalability Improvements
========================

.. toctree::
   :maxdepth: 1
   :glob:

   scalability/innodb_split_buf_pool_mutex
   scalability/innodb_insert_buffer
   scalability/innodb_io_55
   scalability/innodb_adaptive_hash_index_partitions

Performance Improvements
========================

.. toctree::
   :maxdepth: 1
   :glob:

   performance/atomic_fio
   performance/innodb_lazy_drop_table
   performance/innodb_doublewrite_path
   performance/query_cache_enhance
   performance/innodb_fast_checksum
   performance/remove_fcntl_excessive_calls
   performance/innodb_opt_lru_count
   performance/innodb_thread_concurrency_timer_based
   performance/innodb_numa_support
   performance/handlersocket
   performance/threadpool
   performance/binary_group_commit

Flexibility Improvements
========================

.. toctree::
   :maxdepth: 1
   :glob:

   flexibility/innodb_files_extend
   flexibility/log_warnings_suppress
   flexibility/mysql_remove_eol_carret
   flexibility/buff_read_ahead_area
   flexibility/innodb_fast_shutdown
   flexibility/improved_memory_engine
   flexibility/max_binlog_files
   flexibility/mysqldump_ignore_create_error
   flexibility/extended_select_into_outfile
   flexibility/extended_mysqlbinlog

Reliability Improvements
========================

.. toctree::
   :maxdepth: 1
   :glob:

   reliability/crash_resistant_replication
   reliability/log_connection_error
   reliability/error_pad
   reliability/innodb_corrupt_table_action
   reliability/show_slave_status_nolock

Management Improvements
=======================

.. toctree::
   :maxdepth: 1
   :glob:

   management/innodb_recovery_patches
   management/innodb_dict_size_limit
   management/innodb_expand_import
   management/innodb_lru_dump_restore
   management/innodb_fast_index_creation
   management/innodb_expanded_fast_index_creation
   management/sql_no_fcache
   management/udf_percona_toolkit
   management/innodb_fake_changes
   management/innodb_kill_idle_trx
   management/enforce_engine
   management/utility_user
   management/secure_file_priv_extended
   management/expanded_program_option_modifiers
   management/changed_page_tracking
   management/pam_plugin

Diagnostics Improvements
========================

.. toctree::
   :maxdepth: 1
   :glob:

   diagnostics/innodb_stats
   diagnostics/user_stats
   diagnostics/slow_extended_55
   diagnostics/innodb_show_status
   diagnostics/innodb_deadlock_count
   diagnostics/mysql_syslog
   diagnostics/response_time_distribution
   diagnostics/show_engines
   diagnostics/innodb_show_lock_names
   diagnostics/process_list
   diagnostics/misc_info_schema_tables
   diagnostics/thread_based_profiling

Obsolete and Removed Features
=============================
.. toctree::
   :maxdepth: 1
   :glob:

   management/innodb_buffer_pool_shm
   scalability/innodb_extra_rseg

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
   compatibility
   release-notes/release-notes_index
   glossary

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

