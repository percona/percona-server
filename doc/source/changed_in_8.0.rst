.. _changed_in_8.0:

=============================
Changed in Percona Server 8.0
=============================

|Percona Server| 8.0 is based on |MySQL| 8.0 and incorporates many of the
improvements found in |Percona Server| 8.0.

Features ported to |Percona Server| 8.0 from |Percona Server| 5.7
================================================================================

The following features have been ported to |Percona Server| 8.0 from |Percona Server| 5.7: 

Ported SHOW ENGINE INNODB STATUS Extensions
--------------------------------------------------------------------------------

- The Redo Log state
- Specifying the InnoDB buffer pool sizes in bytes
- ``innodb_print_lock_wait_timeout_info`` system variable

Performance
--------------------------------------------------------------------------------

- :ref:`prefix_index_queries_optimization`
- :ref:`aio_page_requests`
- :ref:`threadpool`
- :ref:`ps.buffer-pool.free-list.priority-refill`
- :ref:`lru_manager_threads`
- :ref:`parallel_doublewrite_buffer`

Flexibility
--------------------------------------------------------------------------------

- :ref:`innodb_fts_improvements`
- :ref:`improved_memory_engine`
- :ref:`extended_mysqldump`
- :ref:`extended_select_into_outfile`
- :ref:`proxy_protocol_support`
- :ref:`compressed_columns`

Management
--------------------------------------------------------------------------------

- :ref:`udf_percona_toolkit`
- :ref:`kill_idle_trx`
- :ref:`changed_page_tracking`
- :ref:`pam_plugin`
- :ref:`expanded_innodb_fast_index_creation`
- :ref:`backup_locks`
- :ref:`audit_log_plugin`
- :ref:`start_transaction_with_consistent_snapshot`
- :ref:`extended_show_grants`
- :ref:`data_at_rest_encryption`

Reliability
--------------------------------------------------------------------------------

- :ref:`innodb_corrupt_table_action_page`
- :ref:`log_connection_error`

Diagnostics
--------------------------------------------------------------------------------

- :ref:`user_stats`
- :ref:`slow_extended`
- :ref:`show_engines`
- :ref:`process_list`
- :ref:`INFORMATION_SCHEMA.[GLOBAL_]TEMP_TABLES <temp_tables>`
- :ref:`thread_based_profiling`
- :ref:`innodb_fragmentation_count`

Features removed from |Percona Server| 8.0
================================================================================

Some features, that were present in |Percona Server| 5.7, are removed from
|Percona Server| 8.0:

Removed Features
--------------------------------------------------------------------------------

- Slow Query Log Rotation and Expiration
- CSV engine mode for standard-compliant quote and comma parsing
- Enforcing a storage engine
- Utility user
- Expanded program option modifiers
- The ``ALL_O_DIRECT`` InnoDB flush method: it is not compatible with the new
  redo logging implementation
- XTRADB_RSEG table from INFORMATION_SCHEMA
- InnoDB memory size information from SHOW ENGINE INNODB STATUS; the same
  information is available from Performance Schema memory summary tables

.. seealso::

   |MySQL| Documentation: Performance Schema Table Description
      https://dev.mysql.com/doc/refman/8.0/en/performance-schema-table-descriptions.html

.. TODO: Redo logging

The enforcement of the storage engine has been removed not to conflict with the
storage engine blacklisting feature in |MySQL| 8.0.

Removed Syntax
--------------------------------------------------------------------------------

- The ``SET STATEMENT ... FOR ...`` statement that enabled setting a variable
  for a single query.
- The ``LOCK TABLES FOR BACKUP`` statement due to the introduction of the
  ``log_status`` table in Performance Schema of |MySQL| 8.0.

.. rubric:: Replacing ``SET STATEMENT FOR`` with the Upstream Equivalent

|Percona Server| 8.0 uses the ``SET_VAR`` as the equvalent introduced in |MySQL|
8.0. ``SET_VAR`` is an optimizer hint that can be applied to session variables.

|Percona Server| 8.0 extends the ``SET_VAR`` hint to support the following:

- The ``OPTIMIZE TABLE`` statement
- MyISAM session variables
- Plugin or Storage Engine variables
- InnoDB Session variables
- The ``ALTER TABLE`` statement
- ``CALL stored_proc()`` statement
- The ``ANALYZE TABLE`` statement
- The ``CHECK TABLE`` statement
- The ``LOAD INDEX`` statement (used for MyISAM)
- The ``CREATE TABLE`` statement

|Percona Server| 8.0 also supports setting the following variables by using ``SET_VAR``:

- innodb_lock_wait_timeout
- innodb_tmpdir
- innodb_ft_user_stopword_table
- block_encryption_mode
- histogram_generation_max_mem_size
- myisam_sort_buffer_size
- myisam_repair_threads
- myisam_stats_method
- preload_buffer_size (used by MyISAM only)
  
.. seealso::

   |MySQL| Documentation: The log_status Table
      https://dev.mysql.com/doc/refman/8.0/en/log-status-table.html
   |MySQL| Documentation: Variable-setting hint syntax
      https://dev.mysql.com/doc/refman/8.0/en/optimizer-hints.html#optimizer-hints-set-var

Removed Plugins
--------------------------------------------------------------------------------

- ``SCALABILITY_METRICS``
- ``QUERY_RESPONSE_TIME_AUDIT``

The ``QUERY_RESPONSE_TIME_AUDIT`` plugin has been removed from |Percona Server|
8.0 as the Performance Schema of |MySQL| 8.0 provides statement execution time
histogram data.

.. seealso::

   |MySQL| Documentation: Statement Histogram Summary Tables
      https://dev.mysql.com/doc/refman/8.0/en/statement-histogram-summary-tables.html

Removed System variables
--------------------------------------------------------------------------------

- The ``innodb_use_global_flush_log_at_trx_commit`` system variable
  which enabled setting the global |MySQL| variable
  `innodb_flush_log_at_trx_commit
  <https://dev.mysql.com/doc/refman/8.0/en/innodb-parameters.html#sysvar_innodb_flush_log_at_trx_commit>`_
- ``pseudo_server_id``
- ``max_slowlog_files``
- ``max_slowlog_size``
- ``innodb_show_verbose_locks``: showed the records locked in ``SHOW ENGINE INNODB STATUS``
- :term:`NUMA` support in ``mysqld_safe``
- ``innodb_kill_idle_trx`` which was an alias to the ``kill_idle_trx`` system variable
- The ``max_binlog_files`` system variable

