.. _changed_in_57:

=============================
Changed in Percona Server 5.7
=============================

|Percona Server| 5.7 is based on MySQL 5.7 and incorporates many of the improvements found in |Percona Server| 5.6.

Features removed from |Percona Server| 5.7 that were available in |Percona Server| 5.6
=======================================================================================

.. note:: 

  Percona Server 5.7 won't be able to start if any of variables from these
  features are set in the server's configuration file.

Some features that were present in |Percona Server| 5.6 have been removed in |Percona Server| 5.7. These are:

 * `Handlersocket <https://www.percona.com/doc/percona-server/5.6/performance/handlersocket.html>`_ - This feature might be included in a future release if HandlerSocket starts supporting 5.7.
 * `Support for Fake Changes <https://www.percona.com/doc/percona-server/5.6/management/innodb_fake_changes.html>`_ - Instead of replica prefetching using the fake changes, a 5.7 intra-schema parallel replication replica should be used.
 * ``SHOW ENGINE INNODB STATUS`` no longer prints the count of active Read-Only transactions.
 * `InnoDB redo log archiving <https://www.percona.com/doc/percona-server/5.6/management/log_archiving.html>`_ has been removed due to lack of user uptake of the feature.

Changes in Percona Server 5.6 features 
======================================

 * The minor |Percona Server| version number ("y" in "5.a.b-x.y") has been dropped to simplify |Percona Server| versioning.
 * Performance Schema memory instrumentation support has been added to the :ref:`audit_log_plugin`, :ref:`scalability_metrics_plugin`, and :ref:`pam_plugin`, and to the core server to track memory used by :ref:`user_stats`, :ref:`per_query_variable_statement`, :ref:`changed_page_tracking`, and :ref:`threadpool` features.
 * :ref:`audit_log_plugin` now produces diagnostics in a format consistent with the rest of the server. 
 * The `performance_schema.metadata_locks <https://dev.mysql.com/doc/refman/5.7/en/metadata-locks-table.html>`_ table now displays ``backup`` and ``binlog`` lock information too. The ``object_type`` column has two new valid values: ``backup``, and ``binlog``.
 * :table:`XTRADB_RSEG` table schema has been changed to support new possible InnoDB page sizes. The ``zip_size`` column has been removed and replaced by new columns ``physical_page_size``, ``logical_page_size``, and ``is_compressed``.
 * :table:`XTRADB_READ_VIEW` table no longer contains the ``READ_VIEW_UNDO_NUMBER`` column, which was associated with unused code and always contained zero.
 * Interaction between ``--hidden-`` option modifier and `session_track_system_variables <http://dev.mysql.com/doc/refman/5.7/en/server-system-variables.html#sysvar_session_track_system_variables>`_ has been implemented as follows: any variables with ``--hidden-`` modifier become hidden from the latter variable too. Thus, they should not be present there. Even if you never set ``session_track_system_variables``, care must be taken if a variable contained in its default value (i.e. autocommit) is hidden.
 * Nested ``SET STATEMENT ... FOR SET STATEMENT ... FOR ...`` statements will have different effect in the innermost clause in case the nested clauses set the same variables: in 5.6 the innermost assignment had effect whereas in 5.7 the outermost assignment is effective.
 * :ref:`psaas_utility_user` is treated as a ``SUPER`` user for the purposes of `offline mode <http://dev.mysql.com/doc/refman/5.7/en/server-system-variables.html#sysvar_offline_mode>`_: utility user connections are not dropped if server switches to offline mode and new utility user connections can be established to such server.
 * The server will abort startup with an error message if conflicting :variable:`enforce_storage_engine` and :variable:`disabled_storage_engines` option values are specified, that is, if the enforced storage engine is in the list of disabled storage engines.

Features available in |Percona Server| 5.6 that have been replaced with MySQL 5.7 features
============================================================================================

.. note:: 

  Percona Server 5.7 won't be able to start if any of variables from these
  features are set in the server's configuration file.

Some |Percona Server| 5.6 features have been replaced by similar or equivalent MySQL 5.7 features, so we now keep the MySQL 5.7 implementations in |Percona Server| 5.7. These are:

 * `Lock-Free SHOW SLAVE STATUS NONBLOCKING <https://www.percona.com/doc/percona-server/5.6/reliability/show_slave_status_nolock.html>`_ has been replaced by a regular ``SHOW SLAVE STATUS`` `implementation <http://dev.mysql.com/doc/refman/5.7/en/show-slave-status.html>`_. Oracle implementation forbids calling it from a stored function.
 * Behavior corresponding to :variable:`slow_query_log_timestamp_precision` set to ``microsecond`` is now the default, the variable itself and the behavior corresponding to the variable's ``second`` value is removed.
 * Behavior corresponding to :variable:`slow_query_log_timestamp_always` set to ``TRUE`` is now the default, the variable itself and the behavior corresponding to the variable's ``FALSE`` value is removed.
 * `Statement timeout feature <http://www.percona.com/doc/percona-server/5.6/management/statement_timeout.html>`_ has been replaced by Oracle `Server-side SELECT statement timeouts <http://mysqlserverteam.com/server-side-select-statement-timeouts/>`_ implementation. Differences: the Oracle variable is named `max_statement_time <http://dev.mysql.com/doc/refman/5.7/en/server-system-variables.html#sysvar_max_statement_time>`_ instead of :variable:`max_statement_time`; variable :variable:`have_statement_timeout` variable has been removed removed; the timeouts only apply for `read-only SELECTs <http://dev.mysql.com/doc/refman/5.7/en/select.html>`_.
 * `Atomic write support on fusionIO devices <http://www.percona.com/doc/percona-server/5.6/performance/atomic_fio.html>`_ with NVMFS has been replaced by Oracle implementation. It is no longer required to enable :variable:`innodb_use_atomic_writes` variable, and this variable has been removed. The atomic write support will be enabled, and the doublewrite buffer disabled, on supporting devices automatically. The Oracle implementation does not silently adjust :variable:`innodb_flush_method` to ``O_DIRECT`` if it has a different value. The user must set it to ``O_DIRECT`` explicitly, or atomic writes will not be enabled.
 * `Online GTID migration patch <http://www.percona.com/doc/percona-server/5.6/flexibility/online_gtid_deployment.html>`_ has been replaced by an upstream variable `gtid_mode <http://dev.mysql.com/doc/refman/5.7/en/replication-options-gtids.html#option_mysqld_gtid-mode>`_ made dynamic.
 * The `Error Code Compatibility <https://www.percona.com/doc/percona-server/5.6/reliability/error_pad.html>`_ has been replaced by the multiple start-error-number directive in :file:`sql/share/errmsg-utf8.txt` support. 
 * `Ignoring missing tables in mysqldump <https://www.percona.com/doc/percona-server/5.6/flexibility/mysqldump_ignore_create_error.html>`_ with ``--ignore-create-error`` option has been replaced by the more general upstream option `--ignore-error <http://dev.mysql.com/doc/refman/5.7/en/mysqldump.html#option_mysqldump_ignore-error>`_ option.
 * `innodb_log_block_size <https://www.percona.com/doc/percona-server/5.6/scalability/innodb_io.html#innodb_log_block_size>`_ has been replaced by `innodb_log_write_ahead_size <https://dev.mysql.com/doc/refman/5.7/en/innodb-parameters.html#sysvar_innodb_log_write_ahead_size>`_ variable. To avoid read on write when the storage block size is not equal to 512 bytes, the latter should be set to the same value the former was. If :variable:`innodb_log_block_size` was set to non-default values, new log files must be created during the upgrade. This can be done by cleanly shutting down the service and removing the variable from :file:`my.cnf` configuration and removing the old logs and starting the service again before doing the upgrade.
 * `Extended secure-file-priv server option <https://www.percona.com/doc/percona-server/5.6/management/secure_file_priv_extended.html>`_, which was used to disable ``LOAD DATA INFILE``, ``SELECT INTO OUTFILE`` statements, and ``LOAD_FILE()`` function completely, has been replaced by upstream introducing ``NULL`` as a possible value to this variable. To migrate, any value-less settings must be replaced by ``NULL``.
 * `innodb_sched_priority_cleaner <https://www.percona.com/doc/percona-server/5.6/performance/xtradb_performance_improvements_for_io-bound_highly-concurrent_workloads.html#innodb_sched_priority_cleaner>`_ variable has been removed, as the effect of setting it to 39 (corresponding to nice value of -20), is now enabled by default.
 * `innodb_adaptive_hash_index_partitions <https://www.percona.com/doc/percona-server/5.6/scalability/innodb_adaptive_hash_index_partitions.html#innodb_adaptive_hash_index_partitions>`_ has been replaced by `innodb_adaptive_hash_index_parts <https://dev.mysql.com/doc/refman/5.7/en/innodb-parameters.html#sysvar_innodb_adaptive_hash_index_parts>`_. 
 * In the default server setup (with InnoDB being the only one XA-capable storage engine), ``--tc-heuristic-recover=COMMIT`` is silently converted to ``ROLLBACK``. If TokuDB or another XA-supporting 3rd party storage engine is installed, ``--tc-heuristic-recover=ROLLBACK`` option is unavailable. The default value of ``tc-heuristic-recover`` option in |Percona Server| 5.6 but not in MySQL 5.6 was ``NONE`` as a result of fix for upstream bug :mysqlbug:`70860`. Since Oracle fixed the same bug in 5.7, the default value is ``OFF`` now.
 * `innodb_log_checksum_algorithm <https://www.percona.com/doc/percona-server/5.6/scalability/innodb_io.html#innodb_log_checksum_algorithm>`_ feature has been replaced by `innodb_log_checksums <http://dev.mysql.com/doc/refman/5.7/en/innodb-parameters.html#sysvar_innodb_log_checksums>`_ option. In particular, to get the effect of setting the :variable:`innodb_log_checksum_algorithm` to ``crc32``, :variable:`innodb_log_checksums` should be set to ``ON``, which is a default setting for this variable.
 * `innodb_buffer_pool_populate <https://www.percona.com/doc/percona-server/5.6/performance/innodb_numa_support.html#innodb_buffer_pool_populate>`_ server option and `numa_interleave <https://www.percona.com/doc/percona-server/5.6/performance/innodb_numa_support.html#numa_interleave>`_  ``mysql_safe.sh`` option have been replaced by `innodb_numa_interleave <http://dev.mysql.com/doc/refman/5.7/en/innodb-parameters.html#sysvar_innodb_numa_interleave>`_ server option. Note that :variable:`flush_caches` option still remains.
 * `Ability to change database for mysqlbinlog <https://www.percona.com/doc/percona-server/5.6/flexibility/mysqlbinlog_change_db.html>`_ implementation has been replaced from MariaDB one with MySQL `rewrite-db <http://dev.mysql.com/doc/refman/5.7/en/mysqlbinlog.html#option_mysqlbinlog_rewrite-db>`_ one. The feature is mostly identical with two differences: 1) multiple rewrite rules must be given as separate options, and the ability to list them in a single rule, separated by commas, is lost. That is, any ``--rewrite-db='a->b,c->d'`` occurrences must be replaced with ``--rewrite-db='a->b' --rewrite-db='c->d'``. 2) Whitespace around database names is not ignored.
 * `INFORMATION_SCHEMA.PROCESSLIST.TID column <https://www.percona.com/doc/percona-server/5.6/diagnostics/process_list.html>`_ has been has been replaced by `PERFORMANCE_SCHEMA.THREADS.THREAD_OS_ID column <http://dev.mysql.com/doc/refman/5.7/en/threads-table.html>`_. If running under thread pool, ``THREAD_OS_ID`` column will always be ``NULL``, whereas in the 5.6 implementation ``TID`` column showed either ``NULL`` or the assigned worker thread id at the moment.
 * `innodb_foreground_preflush server <https://www.percona.com/doc/percona-server/5.6/performance/xtradb_performance_improvements_for_io-bound_highly-concurrent_workloads.html#innodb_foreground_preflush>`_ variable has been removed as the upstream implemented a similar feature without a controlling option.
 * `Log All Client Commands (syslog) <http://www.percona.com/doc/percona-server/5.6/diagnostics/mysql_syslog.html>`_ feature has been replaced by Oracle `mysql Logging <http://dev.mysql.com/doc/refman/5.7/en/mysql-logging.html>`_ implementation.
 * Support for `Multiple user level locks per connection <https://www.percona.com/doc/percona-server/5.6/scalability/multiple_user_level_locks.html>`_ has been replaced by Oracle implementation, which is based on the same contributed patch by *Kostja Osipov*.
 * `super-read-only option <https://www.percona.com/doc/percona-server/5.6/management/super_read_only.html>`_ has been replaced by Oracle `super_read_only <http://dev.mysql.com/doc/refman/5.7/en/server-system-variables.html#sysvar_super_read_only>`_ variable implementation.
 * Mutex names in ``SHOW ENGINE INNODB MUTEX`` have been replaced by Oracle mutex name implementation.
 * |Percona Server| now uses packaging similar to the upstream MySQL version. Most important change is that for *Debian*/*Ubuntu* upgrades you now need to run ``mysql_upgrade`` manually.

List of status variables that are no longer available in |Percona Server| 5.7
=============================================================================

Following status variables available in |Percona Server| 5.6 are no longer present in |Percona Server| 5.7:

.. tabularcolumns:: |p{7cm}|p{8.5cm}|

.. list-table::
   :header-rows: 1

   * - Status Variables
     - Replaced by
   * - ``Com_purge_archived``
     - InnoDB redo log archiving has been removed due to lack of user uptake of the feature.
   * - ``Com_purge_archived_before_date``
     - InnoDB redo log archiving has been removed due to lack of user uptake of the feature.
   * - ``read_views_memory``
     - transaction descriptors replaced by the upstream implementation
   * - ``descriptors_memory``
     - transaction descriptors replaced by the upstream implementation
   * - ``innodb_mem_total``
     - This variable was always zero in 5.6 with the default :variable:`innodb_use_sys_malloc` setting
   * - ``innodb_deadlocks``
     - Information now available in ``INFORMATION_SCHEMA.INNODB_METRICS`` table (``lock_deadlocks``)
   * - ``Innodb_ibuf_merges``
     - Information now available in ``INFORMATION_SCHEMA.INNODB_METRICS`` table (``ibuf_merges``)
   * - ``Innodb_ibuf_merged_deletes``
     - Information now available in ``INFORMATION_SCHEMA.INNODB_METRICS`` table (``ibuf_merges_delete``)
   * - ``Innodb_ibuf_merged_delete_marks``
     - Information now available in ``INFORMATION_SCHEMA.INNODB_METRICS`` table (``ibuf_merges_delete_mark``)
   * - ``Innodb_ibuf_discarded_deletes``
     - Information now available in ``INFORMATION_SCHEMA.INNODB_METRICS`` table (``ibuf_merges_discard_delete``)
   * - ``Innodb_ibuf_discarded_delete_marks``
     - Information now available in ``INFORMATION_SCHEMA.INNODB_METRICS`` table (``ibuf_merges_discard_delete_mark``)
   * - ``Innodb_ibuf_discarded_inserts``
     - Information now available in ``INFORMATION_SCHEMA.INNODB_METRICS`` table (``ibuf_merges_discard_insert``)
   * - ``Innodb_ibuf_merged_inserts``
     - Information now available in ``INFORMATION_SCHEMA.INNODB_METRICS`` table (``ibuf_merges_insert``)
   * - ``Innodb_ibuf_size``
     - Information now available in ``INFORMATION_SCHEMA.INNODB_METRICS`` table (``ibuf_size``)
   * - ``Innodb_s_lock_os_waits``
     - Information now available in ``INFORMATION_SCHEMA.INNODB_METRICS`` table (``innodb_rwlock_s_os_waits``)
   * - ``Innodb_s_lock_spin_rounds``
     - Information now available in ``INFORMATION_SCHEMA.INNODB_METRICS`` table (``innodb_rwlock_s_spin_rounds``)
   * - ``Innodb_s_lock_spin_waits``
     - Information now available in ``INFORMATION_SCHEMA.INNODB_METRICS`` table (``innodb_rwlock_s_spin_waits``)
   * - ``Innodb_x_lock_os_waits``
     - Information now available in ``INFORMATION_SCHEMA.INNODB_METRICS`` table (``innodb_rwlock_x_os_waits``)
   * - ``Innodb_x_lock_spin_rounds``
     - Information now available in ``INFORMATION_SCHEMA.INNODB_METRICS`` table (``innodb_rwlock_x_spin_rounds``)
   * - ``Innodb_x_lock_spin_waits``
     - Information now available in ``INFORMATION_SCHEMA.INNODB_METRICS`` table (``innodb_rwlock_x_spin_waits``)
   * - ``Innodb_current_row_locks``
     - Information now available in ``INFORMATION_SCHEMA.INNODB_METRICS`` table (``lock_row_lock_current_waits``)
   * - ``Innodb_history_list_length``
     - Information now available in ``INFORMATION_SCHEMA.INNODB_METRICS`` table (``trx_rseg_history_len``)
   * - ``Innodb_mutex_os_waits``
     - ``SHOW ENGINE INNODB MUTEX`` presents the same information, but per-mutex instead of whole system aggregation
   * - ``Innodb_mutex_spin_rounds``
     - ``SHOW ENGINE INNODB MUTEX`` presents the same information, but per-mutex instead of whole system aggregation
   * - ``Innodb_mutex_spin_waits``
     - ``SHOW ENGINE INNODB MUTEX`` presents the same information, but per-mutex instead of whole system aggregation


List of system variables that are no longer available in |Percona Server| 5.7
=============================================================================

Following system variables available in |Percona Server| 5.6 are no longer present in |Percona Server| 5.7:

.. warning::

   |Percona Server| 5.7 won't be able to start if some of these variables are set in the server's configuration file.

.. tabularcolumns:: |p{8cm}|p{7.5cm}|

.. list-table::
   :header-rows: 1

   * - System Variables
     - Feature Comment
   * - :variable:`gtid_deployment_step`                     
     - Replaced by an upstream variable `gtid_mode <http://dev.mysql.com/doc/refman/5.7/en/replication-options-gtids.html#option_mysqld_gtid-mode>`_ made dynamic.
   * - :variable:`innodb_fake_changes`                     
     - Instead of replica prefetching using the fake changes, a 5.7 intra-schema parallel replication replica should be used.
   * - :variable:`innodb_locking_fake_changes`                     
     - Instead of replica prefetching using the fake changes, a 5.7 intra-schema parallel replication replica should be used.
   * - :variable:`innodb_log_archive`                     
     - InnoDB redo log archiving has been removed due to lack of user uptake of the feature.
   * - :variable:`innodb_log_arch_dir`                     
     - InnoDB redo log archiving has been removed due to lack of user uptake of the feature.
   * - :variable:`innodb_log_arch_expire_sec`                     
     - InnoDB redo log archiving has been removed due to lack of user uptake of the feature.
   * - :variable:`innodb_log_block_size`
     - Replaced by upstream `innodb_log_write_ahead_size <https://dev.mysql.com/doc/refman/5.7/en/innodb-parameters.html#sysvar_innodb_log_write_ahead_size>`_ variable. To avoid read on write when the storage block size is not equal to 512 bytes, the latter should be set to the same value the former was. If :variable:`innodb_log_block_size` was set to non-default values, new log files must be created during the upgrade. This can be done by cleanly shutting down the service and removing the variable from :file:`my.cnf` configuration and removing the old logs and starting the service again before doing the upgrade.
   * - :variable:`max_statement_time`                     
     - Replaced by upstream `max_execution_time <http://dev.mysql.com/doc/refman/5.7/en/server-system-variables.html#sysvar_max_statement_time)>`_ variable in `Server-side SELECT statement timeouts <http://mysqlserverteam.com/server-side-select-statement-timeouts/>`_ implementation. 
   * - :variable:`have_statement_timeout`                     
     - Variable has been removed due to upstream feature implementation
   * - :variable:`innodb_use_atomic_writes`                     
     - Variable has been removed due to upstream feature implementation
   * - :variable:`innodb_adaptive_hash_index_partitions`                     
     - Replaced by upstream variable `innodb_adaptive_hash_index_parts <https://dev.mysql.com/doc/refman/5.7/en/innodb-parameters.html#sysvar_innodb_adaptive_hash_index_parts>`_

Features ported from |Percona Server| 5.6 to |Percona Server| 5.7
==================================================================

Following features were ported from |Percona Server| 5.6 to |Percona Server| 5.7: 

.. tabularcolumns:: |p{12cm}|p{3.5cm}|

.. list-table::
   :header-rows: 1

   * - Feature Ported
     - Version
   * - :ref:`innodb_split_buf_pool_mutex`                 
     - :rn:`5.7.10-1`
   * - :ref:`innodb_io_page`                                   
     - :rn:`5.7.10-1`
   * - :ref:`query_cache_enhance`                         
     - :rn:`5.7.10-1`
   * - :ref:`innodb_numa_support`                         
     - :rn:`5.7.10-1`
   * - :ref:`threadpool`                                  
     - :rn:`5.7.10-1`
   * - :ref:`xtradb_performance_improvements_for_io-bound_highly-concurrent_workloads` 
     - :rn:`5.7.10-1`
   * - :ref:`log_warning_suppress`                         
     - :rn:`5.7.10-1`
   * - :ref:`improved_memory_engine`                         
     - :rn:`5.7.10-1`
   * - :ref:`maximum_binlog_files`                         
     - :rn:`5.7.10-1`
   * - :ref:`extended_select_into_outfile`                         
     - :rn:`5.7.10-1`
   * - :ref:`per_query_variable_statement`                         
     - :rn:`5.7.10-1`
   * - :ref:`extended_mysqlbinlog`                         
     - :rn:`5.7.10-1`
   * - :ref:`slowlog_rotation`                         
     - :rn:`5.7.10-1`
   * - :ref:`csv_engine_mode`                         
     - :rn:`5.7.10-1`
   * - :ref:`proxy_protocol_support`                         
     - :rn:`5.7.10-1`
   * - :ref:`per_session_server-id`                         
     - :rn:`5.7.10-1`
   * - :ref:`log_connection_error`                         
     - :rn:`5.7.10-1`
   * - :ref:`innodb_corrupt_table_action_page`                         
     - :rn:`5.7.10-1`
   * - :ref:`udf_percona_toolkit`                         
     - :rn:`5.7.10-1`
   * - :ref:`innodb_kill_idle_trx`                         
     - :rn:`5.7.10-1`
   * - :ref:`enforce_engine`                         
     - :rn:`5.7.10-1`
   * - :ref:`psaas_utility_user`                         
     - :rn:`5.7.10-1`
   * - :ref:`expanded_option_modifiers`                         
     - :rn:`5.7.10-1`
   * - :ref:`changed_page_tracking`                         
     - :rn:`5.7.10-1`
   * - :ref:`pam_plugin`                         
     - :rn:`5.7.10-1`
   * - :ref:`expanded_innodb_fast_index_creation`                         
     - :rn:`5.7.10-1`
   * - :ref:`backup_locks`                         
     - :rn:`5.7.10-1`
   * - :ref:`audit_log_plugin`                         
     - :rn:`5.7.10-1`
   * - :ref:`start_transaction_with_consistent_snapshot`                         
     - :rn:`5.7.10-1`
   * - :ref:`extended_show_grants`                         
     - :rn:`5.7.10-1`
   * - :ref:`user_stats`                         
     - :rn:`5.7.10-1`
   * - :ref:`slow_extended`                         
     - :rn:`5.7.10-1`
   * - :ref:`innodb_show_status`                         
     - :rn:`5.7.10-1`
   * - :ref:`show_engines`                         
     - :rn:`5.7.10-1`
   * - :ref:`process_list`                         
     - :rn:`5.7.10-1`
   * - :ref:`misc_info_schema_tables`                         
     - :rn:`5.7.10-1`
   * - :ref:`thread_based_profiling`                         
     - :rn:`5.7.10-1`
   * - :ref:`scalability_metrics_plugin`                         
     - :rn:`5.7.10-1`
   * - :ref:`response_time_distribution`                         
     - :rn:`5.7.10-1`

