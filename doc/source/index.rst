.. Percona Server documentation master file, created by
   sphinx-quickstart on Mon Aug  8 01:24:46 2011.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. _dochome:

===================================
 Percona Server 5.6 - Documentation
===================================

.. note::

 Please note: |Percona Server| 5.6 is RELEASE CANDIDATE quality software. It should *NOT* be used in production environments.

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
   changed_in_56

Installation
============

.. toctree::
   :maxdepth: 1
   :glob:

   installation
   upgrading_guide_55_56

Scalability Improvements
========================

.. toctree::
   :maxdepth: 1
   :glob:

   scalability/innodb_io

Performance Improvements
========================

.. toctree::
   :maxdepth: 1
   :glob:

   performance/atomic_fio
   performance/query_cache_enhance
   performance/innodb_numa_support
   performance/threadpool

Flexibility Improvements
========================

.. toctree::
   :maxdepth: 1
   :glob:

   flexibility/log_warnings_suppress
   flexibility/improved_memory_engine
   flexibility/max_binlog_files
   flexibility/mysqldump_ignore_create_error

Reliability Improvements
========================

.. toctree::
   :maxdepth: 1
   :glob:

   reliability/log_connection_error
   reliability/error_pad
   reliability/innodb_corrupt_table_action
   reliability/show_slave_status_nolock

Management Improvements
=======================

.. toctree::
   :maxdepth: 1
   :glob:

   management/udf_percona_toolkit
   management/innodb_fake_changes
   management/innodb_kill_idle_trx
   management/enforce_engine
   management/utility_user
   management/secure_file_priv_extended
   management/expanded_program_option_modifiers
   management/changed_page_tracking
   management/pam_plugin
   management/innodb_expanded_fast_index_creation
   management/log_archiving

Diagnostics Improvements
========================

.. toctree::
   :maxdepth: 1
   :glob:

   diagnostics/user_stats
   diagnostics/slow_extended
   diagnostics/innodb_show_status
   diagnostics/innodb_deadlock_count
   diagnostics/mysql_syslog
   diagnostics/show_engines
   diagnostics/innodb_show_lock_names
   diagnostics/process_list
   diagnostics/misc_info_schema_tables
   diagnostics/thread_based_profiling

Reference
=========

.. toctree::
   :maxdepth: 1
   :glob:

   upstream-bug-fixes
   development
   trademark-policy
   index_info_schema_tables
   faq
   compatibility
   release-notes/release-notes_index
   glossary

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`


