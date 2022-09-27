.. _changed_in_version:

================================================================================
Changed in Percona Server |version|
================================================================================

|Percona Server| |version| is based on |MySQL| |version| and incorporates many of the
improvements found in |Percona Server| 5.7.

------------------------------------------------------------------------
Features Ported to |Percona Server| |version| from |Percona Server| 5.7
------------------------------------------------------------------------

The features are listed within the following sections:

SHOW ENGINE INNODB STATUS Extensions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- The Redo Log state
- Specifying the InnoDB buffer pool sizes in bytes
- ``innodb_print_lock_wait_timeout_info`` system variable

Performance
^^^^^^^^^^^

- :ref:`prefix_index_queries_optimization`
- :ref:`aio_page_requests`
- :ref:`threadpool`
- :ref:`ps.buffer-pool.free-list.priority-refill`
- :ref:`lru_manager_threads`


Flexibility
^^^^^^^^^^^

- :ref:`innodb_fts_improvements`
- :ref:`improved_memory_engine`
- :ref:`extended_mysqldump`
- :ref:`extended_select_into_outfile`
- :ref:`proxy_protocol_support`
- :ref:`compressed_columns`

Management
^^^^^^^^^^

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
^^^^^^^^^^^

- :ref:`innodb_corrupt_table_action_page`
- :ref:`log_connection_error`

Diagnostics
^^^^^^^^^^^

- :ref:`user_stats`
- :ref:`slow_extended`
- :ref:`show_engines`
- :ref:`process_list`
- :ref:`INFORMATION_SCHEMA.[GLOBAL_]TEMP_TABLES <temp_tables>`
- :ref:`thread_based_profiling`
- :ref:`innodb_fragmentation_count`

Features Removed from |Percona Server| |version|
================================================

Some features, that were present in |Percona Server| 5.7, are removed from
|Percona Server| |version|:

Removed Features
--------------------------------------------------------------------------------

- `Slow Query Log Rotation and Expiration
  <https://www.percona.com/doc/percona-server/5.7/flexibility/slowlog_rotation.html>`_
- `CSV engine mode for standard-compliant quote and comma parsing <https://www.percona.com/doc/percona-server/5.7/flexibility/csv_engine_mode.html>`_
- `Expanded program option modifiers <https://www.percona.com/doc/percona-server/5.7/management/expanded_program_option_modifiers.html>`_
- `The ALL_O_DIRECT InnoDB flush method: it is not compatible with the new
  redo logging implementation <https://www.percona.com/doc/percona-server/5.7/scalability/innodb_io.html>`_
- `XTRADB_RSEG table from INFORMATION_SCHEMA <https://www.percona.com/doc/percona-server/5.7/diagnostics/misc_info_schema_tables.html>`_
- `InnoDB memory size information from SHOW ENGINE INNODB STATUS; the same
  information is available from Performance Schema memory summary tables <https://www.percona.com/doc/percona-server/5.7/diagnostics/innodb_show_status.html>`_
- `Query cache enhancements
  <https://www.percona.com/doc/percona-server/5.7/performance/query_cache_enhance.html#query-cache-enhancements>`_


.. seealso::

   |MySQL| Documentation: Performance Schema Table Description
      https://dev.mysql.com/doc/refman/8.0/en/performance-schema-table-descriptions.html

.. _changed_in_version.removed_syntax:

Removed Syntax
--------------------------------------------------------------------------------

- The ``SET STATEMENT ... FOR ...`` statement that enabled setting a
  variable for a single query. For more information see
  :ref:`Replacing SET STATEMENT FOR with the Upstream Equivalent
  <set-statement-for.upstream.replacing>`.
- The ``LOCK BINLOG FOR BACKUP`` statement due to the introduction of the
  ``log_status`` table in Performance Schema of |MySQL| |version|.


Removed Plugins
--------------------------------------------------------------------------------

- ``SCALABILITY_METRICS``
- ``QUERY_RESPONSE_TIME`` plugins

The ``QUERY_RESPONSE_TIME`` plugins have been removed from |Percona
Server| |version| as the Performance Schema of |MySQL| |version|
provides histogram data for statement execution time.

.. seealso::

   |MySQL| Documentation: Statement Histogram Summary Tables
      https://dev.mysql.com/doc/refman/8.0/en/statement-histogram-summary-tables.html

Removed System variables
--------------------------------------------------------------------------------

- The `innodb_use_global_flush_log_at_trx_commit
  <https://www.percona.com/doc/percona-server/5.7/scalability/innodb_io.html#innodb_use_global_flush_log_at_trx_commit>`_
  system variable which enabled setting the global |MySQL| variable
  `innodb_flush_log_at_trx_commit
  <https://dev.mysql.com/doc/refman/8.0/en/innodb-parameters.html#sysvar_innodb_flush_log_at_trx_commit>`_
- `pseudo_server_id
  <https://www.percona.com/doc/percona-server/5.7/flexibility/per_session_server-id.html#pseudo_server_id>`_
- `max_slowlog_files
  <https://www.percona.com/doc/percona-server/5.7/flexibility/slowlog_rotation.html#max_slowlog_files>`_
- `max_slowlog_size <https://www.percona.com/doc/percona-server/5.7/flexibility/slowlog_rotation.html#max_slowlog_size>`_
- `innodb_show_verbose_locks
  <https://www.percona.com/doc/percona-server/5.7/diagnostics/innodb_show_status.html#innodb_show_verbose_locks>`_:
  showed the records locked in ``SHOW ENGINE INNODB STATUS``
- `NUMA support in mysqld_safe
  <https://www.percona.com/doc/percona-server/5.7/performance/innodb_numa_support.html#improved-numa-support>`_
- `innodb_kill_idle_trx
  <https://www.percona.com/doc/percona-server/LATEST/management/innodb_kill_idle_trx.html>`_
  which was an alias to the ``kill_idle_trx`` system variable
- The `max_binlog_files <https://www.percona.com/doc/percona-server/5.7/flexibility/max_binlog_files.html#max_binlog_files>`_ system variable

Deprecated Storage engine
-----------------------------------------------------

-  The TokuDB Storage Engine was `declared as deprecated <https://www.percona.com/doc/percona-server/8.0/release-notes/Percona-Server-8.0.13-3.html>`__ in Percona Server for MySQL 8.0 and will be disabled in upcoming 8.0 versions. 
    
    We recommend migrating to the MyRocks Storage Engine. 

    For more information, see the Percona blog post: `Heads-Up: TokuDB Support Changes and Future Removal from Percona Server for MySQL 8.0 <https://www.percona.com/blog/2021/05/21/tokudb-support-changes-and-future-removal-from-percona-server-for-mysql-8-0/>`__.