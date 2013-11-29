.. _ps_versions_comparison:

=======================================================
List of features available in |Percona Server| releases
=======================================================

.. list-table::
   :header-rows: 1

   * - |Percona Server| 5.1
     - |Percona Server| 5.5
     - |Percona Server| 5.6
   * - :ref:`Improved Buffer Pool Scalability <ps51:innodb_split_buf_pool_mutex>`
     - :ref:`Improved Buffer Pool Scalability <ps55:innodb_split_buf_pool_mutex>`
     - :ref:`Improved Buffer Pool Scalability <ps56:innodb_split_buf_pool_mutex>`
   * - :ref:`Configurable Insert Buffer <ps51:innodb_insert_buffer>`
     - :ref:`Configurable Insert Buffer <ps55:innodb_insert_buffer>`
     - Feature not implemented
   * - :ref:`Improved InnoDB I/O Scalability <ps51:innodb_io_page>`
     - :ref:`Improved InnoDB I/O Scalability <ps55:innodb_io_55_page>`
     - :ref:`Improved InnoDB I/O Scalability <ps56:innodb_io_page>`
   * - :ref:`More Concurrent Transactions Available <ps51:innodb_expand_undo_slots>`
     - :ref:`More Concurrent Transactions Available <ps55:innodb_extra_rseg>`
     - Replaced by the upstream implementation [#n-1]_
   * - Feature not implemented
     - :ref:`innodb_adaptive_hash_index_partitions_page`
     - :ref:`Multiple Adaptive Hash Search Partitions <ps56:innodb_adaptive_hash_index_partitions_page>`
   * - :ref:`Dedicated Purge Thread <ps51:innodb_purge_thread>`
     - Replaced by the upstream implementation [#n-2]_
     - Replaced by the upstream implementation [#n-2]_
   * - :ref:`Drop table performance <ps51:innodb_lazy_drop_table_page>`
     - :ref:`Drop table performance <ps55:innodb_lazy_drop_table_page>`
     - Replaced by the upstream fix [#n-3]_
   * - Feature not implemented
     - :ref:`Atomic write support for Fusion-io devices <ps55:atomic_fio>`
     - :ref:`Atomic write support for Fusion-io devices <ps56:atomic_fio>`
   * - :ref:`Configuration of the Doublewrite Buffer <ps51:innodb_doublewrite_path>`
     - :ref:`Configuration of the Doublewrite Buffer <ps55:innodb_doublewrite_path>`
     - Feature not implemented
   * - :ref:`Query Cache Enhancements <ps51:query_cache_enhance>`
     - :ref:`Query Cache Enhancements <ps55:query_cache_enhance>`
     - :ref:`Query Cache Enhancements <ps56:query_cache_enhance>`
   * - :ref:`Fast InnoDB Checksum <ps51:innodb_fast_checksum_page>` [#n-4]_
     - :ref:`Fast InnoDB Checksum <ps55:innodb_fast_checksum_page>` [#n-4]_
     - Replaced by the upstream implementation [#n-4]_
   * - :ref:`Reduced Buffer Pool Mutex Contention <ps51:innodb_opt_lru_count>`
     - :ref:`Reduced Buffer Pool Mutex Contention <ps55:innodb_opt_lru_count>`
     - :ref:`Reduced Buffer Pool Mutex Contention <ps56:innodb_opt_lru_count>`
   * - :ref:`InnoDB timer-based Concurrency Throttling <ps51:innodb_thread_concurrency_timer_based_page>`
     - :ref:`InnoDB timer-based Concurrency Throttling <ps55:innodb_thread_concurrency_timer_based_page>`
     - Replaced by the upstream implementation [#n-5]_
   * - :ref:`HandlerSocket <ps51:handlersocket_page>`
     - :ref:`HandlerSocket <ps55:handlersocket_page>`
     - Feature not implemented [#n-6]_
   * - Feature not implemented
     - :ref:`Improved NUMA support <ps55:innodb_numa_support>`
     - :ref:`Improved NUMA support <ps56:innodb_numa_support>`
   * - Feature not implemented
     - :ref:`Thread Pool <ps55:threadpool>`
     - :ref:`Thread Pool <ps56:threadpool>`
   * - Feature not implemented
     - :ref:`Binary Log Group Commit <ps55:binary_group_commit>`
     - Replaced by the upstream implementation [#n-7]_
   * - :ref:`Support of Multiple Page Sizes <ps51:innodb_files_extend>` [#n-8]_
     - :ref:`Support of Multiple Page Sizes <ps55:innodb_files_extend>` [#n-8]_
     - Replaced by the upstream implementation [#n-8]_
   * - :ref:`Suppress Warning Messages <ps51:log_warnings_suppress_page>`
     - :ref:`Suppress Warning Messages <ps55:log_warning_suppress>`
     - :ref:`Suppress Warning Messages <ps56:log_warning_suppress>`
   * - :ref:`Handle BLOB End of Line <ps51:mysql_remove_eol_carret>`
     - :ref:`Handle BLOB End of Line <ps55:mysql_remove_eol_carret>`
     - Replaced by the upstream implementation [#n-9]_
   * - :ref:`Ability to change database for mysqlbinlog <ps51:mysqlbinlog_change_db>` 
     - Feature not implemented
     - Feature not implemented
   * - :ref:`Replication Stop Recovery <ps51:replication_skip_single_statement>`
     - Feature not implemented
     - Feature not implemented
   * - :ref:`Fixed Size for the Read Ahead Area <ps51:buff_read_ahead_area>`      
     - :ref:`Fixed Size for the Read Ahead Area <ps55:buff_read_ahead_area>`      
     - :ref:`Fixed Size for the Read Ahead Area <ps56:buff_read_ahead_area>`      
   * - :ref:`Fast Shutdown <ps51:innodb_fast_shutdown>`
     - Feature not implemented
     - Feature not implemented
   * - Feature not implemented
     - :ref:`Improved MEMORY Storage Engine <ps55:improved_memory_engine>`
     - :ref:`Improved MEMORY Storage Engine <ps56:improved_memory_engine>`
   * - Feature not implemented
     - :ref:`Restricting the number of binlog files <ps55:maximum_binlog_files>`
     - :ref:`Restricting the number of binlog files <ps56:maximum_binlog_files>`
   * - :ref:`Ignoring missing tables in mysqldump <ps51:mysqldump_ignore_create_error>`
     - :ref:`Ignoring missing tables in mysqldump <ps55:mysqldump_ignore_create_error>`
     - :ref:`Ignoring missing tables in mysqldump <ps56:mysqldump_ignore_create_error>`
   * - :ref:`Too Many Connections Warning <ps51:log_connection_error>`
     - :ref:`Too Many Connections Warning <ps55:log_connection_error>`
     - :ref:`Too Many Connections Warning <ps56:log_connection_error>`
   * - :ref:`Error Code Compatibility <ps51:error_pad>`
     - :ref:`Error Code Compatibility <ps55:error_pad>`
     - :ref:`Error Code Compatibility <ps56:error_pad>`
   * - :ref:`Handle Corrupted Tables <ps51:innodb_corrupt_table_action_page>`
     - :ref:`Handle Corrupted Tables <ps55:innodb_corrupt_table_action_page>`
     - :ref:`Handle Corrupted Tables <ps56:innodb_corrupt_table_action_page>`
   * - :ref:`Crash-Resistant Replication <ps51:innodb_recovery_update_relay_log_page>`
     - :ref:`Crash-Resistant Replication <ps55:innodb_recovery_update_relay_log_page>`
     - Replaced by the upstream implementation [#n-10]_
   * - :ref:`Lock-Free SHOW SLAVE STATUS <ps51:show_slave_status_nolock>`
     - :ref:`Lock-Free SHOW SLAVE STATUS <ps55:show_slave_status_nolock>`
     - :ref:`Lock-Free SHOW SLAVE STATUS <ps56:show_slave_status_nolock>`
   * - :ref:`Fast InnoDB Recovery Process <ps51:innodb_recovery_patches>`
     - :ref:`Fast InnoDB Recovery Stats <ps55:innodb_recovery_patches>`
     - Feature not implemented
   * - :ref:`InnoDB Data Dictionary Size Limit <ps51:innodb_dict_size_limit_page>`
     - :ref:`InnoDB Data Dictionary Size Limit <ps55:innodb_dict_size_limit_page>`
     - Replaced by the upstream implementation [#n-11]_
   * - :ref:`Expand Table Import <ps51:innodb_expand_import_page>`
     - :ref:`Expand Table Import <ps55:innodb_expand_import_page>`
     - Replaced by the upstream implementation [#n-12]_
   * - :ref:`Dump/Restore of the Buffer Pool <ps51:innodb_lru_dump_restore>`
     - :ref:`Dump/Restore of the Buffer Pool <ps55:innodb_lru_dump_restore>`
     - Replaced by the upstream implementation [#n-13]_
   * - :ref:`Fast Index Creation <ps51:innodb_fast_index_creation>`
     - :ref:`Fast Index Creation <ps55:innodb_fast_index_creation>`
     - Replaced by the upstream implementation [#n-14]_
   * - :ref:`Expanded Fast Index Creation <ps51:expanded_innodb_fast_index_creation>`
     - :ref:`Expanded Fast Index Creation <ps55:expanded_innodb_fast_index_creation>`
     - :ref:`Expanded Fast Index Creation <ps56:expanded_innodb_fast_index_creation>`
   * - :ref:`Prevent Caching to FlashCache <ps51:sql_no_fcache>`
     - :ref:`Prevent Caching to FlashCache <ps55:sql_no_fcache>`
     - Feature not implemented
   * - :ref:`Percona Toolkit UDFs <ps51:udf_percona_toolkit>`
     - :ref:`Percona Toolkit UDFs <ps55:udf_percona_toolkit>`
     - :ref:`Percona Toolkit UDFs <ps56:udf_percona_toolkit>`
   * - :ref:`Support for Fake Changes <ps51:innodb_fake_changes_page>`
     - :ref:`Support for Fake Changes <ps55:innodb_fake_changes_page>`
     - :ref:`Support for Fake Changes <ps56:innodb_fake_changes_page>`
   * - :ref:`Kill Idle Transactions <ps51:innodb_kill_idle_trx>`
     - :ref:`Kill Idle Transactions <ps55:innodb_kill_idle_trx>`
     - :ref:`Kill Idle Transactions <ps56:innodb_kill_idle_trx>`
   * - :ref:`XtraDB changed page tracking <ps51:changed_page_tracking>`
     - :ref:`XtraDB changed page tracking <ps55:changed_page_tracking>`
     - :ref:`XtraDB changed page tracking <ps56:changed_page_tracking>`
   * - Feature not implemented
     - :ref:`Enforcing Storage Engine <ps55:enforce_engine>`
     - :ref:`Enforcing Storage Engine <ps56:enforce_engine>`
   * - Feature not implemented
     - :ref:`Utility user <ps55:psaas_utility_user>`
     - :ref:`Utility user <ps56:psaas_utility_user>`
   * - Feature not implemented
     - :ref:`Extending the secure-file-priv server option <ps55:secure_file_priv_extended>`
     - :ref:`Extending the secure-file-priv server option <ps56:secure_file_priv_extended>`
   * - Feature not implemented
     - :ref:`Expanded Program Option Modifiers <ps55:expanded_option_modifiers>`
     - :ref:`Expanded Program Option Modifiers <ps56:expanded_option_modifiers>`
   * - Feature not implemented
     - :ref:`PAM Authentication Plugin <ps55:pam_plugin>`
     - :ref:`PAM Authentication Plugin <ps56:pam_plugin>`
   * - Feature not implemented
     - Feature not implemented
     - :ref:`Log Archiving for XtraDB <ps56:log_archiving>`
   * - :ref:`InnoDB Statistics <ps51:innodb_stats>`
     - :ref:`InnoDB Statistics <ps55:innodb_stats>`
     - Replaced by the upstream implementation [#n-15]_
   * - :ref:`User Statistics <ps51:user_stats>`
     - :ref:`User Statistics <ps55:user_stats>`
     - :ref:`User Statistics <ps56:user_stats>`
   * - :ref:`Slow Query Log <ps51:slow_extended>`
     - :ref:`Slow Query Log <ps55:slow_extended_55>`
     - :ref:`Slow Query Log <ps56:slow_extended>`
   * - :ref:`Count InnoDB Deadlocks <ps51:innodb_deadlock_count>`
     - :ref:`Count InnoDB Deadlocks <ps55:innodb_deadlocks_page>`
     - :ref:`Count InnoDB Deadlocks <ps56:innodb_deadlocks_page>`
   * - :ref:`Log All Client Commands (syslog) <ps51:mysql_syslog>`
     - :ref:`Log All Client Commands (syslog) <ps55:mysql_syslog>`
     - :ref:`Log All Client Commands (syslog) <ps56:mysql_syslog>`
   * - :ref:`Response Time Distribution <ps51:response_time_distribution>`
     - :ref:`Response Time Distribution <ps55:response_time_distribution>`
     - Feature not implemented
   * - :ref:`Show Storage Engines <ps51:show_engines>`
     - :ref:`Show Storage Engines <ps55:show_engines>`
     - :ref:`Show Storage Engines <ps56:show_engines>`
   * - :ref:`Show Lock Names <ps51:innodb_show_lock_names>`
     - :ref:`Show Lock Names <ps55:innodb_show_lock_names>`
     - :ref:`Show Lock Names <ps56:innodb_show_lock_names>`
   * - :ref:`Process List <ps51:process_list>`
     - :ref:`Process List <ps55:process_list>`
     - :ref:`Process List <ps56:process_list>`
   * - `Misc. INFORMATION_SCHEMA Tables <http://www.percona.com/doc/percona-server/5.1/diagnostics/misc_info_schema_tables.html>`_
     - :ref:`Misc. INFORMATION_SCHEMA Tables <ps55:misc_info_schema_tables>`
     - :ref:`Misc. INFORMATION_SCHEMA Tables <ps56:misc_info_schema_tables>`
   * - Feature not implemented
     - :ref:`Extended Show Engine InnoDB Status <ps55:innodb_show_status>`
     - :ref:`Extended Show Engine InnoDB Status <ps56:innodb_show_status>`
   * - Feature not implemented
     - :ref:`Thread Based Profiling <ps55:thread_based_profiling>`
     - :ref:`Thread Based Profiling <ps56:thread_based_profiling>`
   * - Feature not implemented
     - Feature not implemented
     - :ref:`XtraDB Performance Improvements for I/O-Bound Highly-Concurrent Workloads <ps56:xtradb_performance_improvements_for_io-bound_highly-concurrent_workloads>`
   * - Feature not implemented
     - Feature not implemented
     - :ref:`Page cleaner thread tuning <ps56:page_cleaner_tuning>`
   * - Feature not implemented
     - Feature not implemented
     - :ref:`Statement Timeout <ps56:statement_timeout>`
   * - Feature not implemented
     - :ref:`Extended SELECT INTO OUTFILE/DUMPFILE <ps55:extended_select_into_outfile>`
     - :ref:`Extended SELECT INTO OUTFILE/DUMPFILE <ps56:extended_select_into_outfile>`
   * - Feature not implemented
     - Feature not implemented
     - :ref:`Per-query variable statement <ps56:per_query_variable_statement>`
   

Other Reading
=============

* :ref:`changed_in_56`
* :ref:`upgrading_guide`
* :ref:`Percona Sever In-Place Upgrading Guide: From 5.5 to 5.6 <ps56:upgrading_guide>`
* `Upgrading from MySQL 5.1 to 5.5 <http://dev.mysql.com/doc/refman/5.5/en/upgrading-from-previous-series.html>`_
* `What Is New in MySQL 5.5 <http://dev.mysql.com/doc/refman/5.5/en/mysql-nutshell.html>`_
* `What Is New in MySQL 5.6 <http://dev.mysql.com/doc/refman/5.6/en/mysql-nutshell.html>`_

Footnotes
=========

.. [#n-1] Feature has been deprecated after |Percona Server| 5.5.11-20.2. It has replaced by the upstream implementation of `innodb_undo_logs <https://dev.mysql.com/doc/refman/5.6/en/innodb-parameters.html#sysvar_innodb_undo_logs>`_ in |MySQL| 5.6.3.
.. [#n-2] Feature has not been ported from |Percona Server| 5.1 version. It has been replaced by the upstream `Improved Purge Scheduling <https://dev.mysql.com/doc/refman/5.6/en/innodb-performance.html#innodb-improved-purge-scheduling>`_ implementation.
.. [#n-3] Feature has been has been removed and its controlling variable ``innodb_lazy_drop_table`` has been deprecated from |Percona Server| 5.5.30-30.2. Feature has been removed because the upstream ``DROP TABLE`` implementation has been improved when bugs :mysqlbug:`56332` and :mysqlbug:`51325` were fixed.
.. [#n-4] Feature has been deprecated after |Percona Server| 5.1.66-14.2 and |Percona Server| 5.5.28-29.2. It has been replaced by the upstream `innodb_checksum_algorithm <http://dev.mysql.com/doc/refman/5.6/en/innodb-parameters.html#sysvar_innodb_checksum_algorithm>`_ implementation released in |MySQL| 5.6.3.
.. [#n-5] Feature has been replaced by the upstream implementation `innodb-performance-thread_concurrency <https://dev.mysql.com/doc/refman/5.6/en/innodb-performance.html#innodb-performance-thread_concurrency>`_ in |MySQL| 5.6
.. [#n-6] Feature will be implemented in one of the future |Percona Server| 5.6 releases. 
.. [#n-7] `Binary Log Group Commit <http://mysqlmusings.blogspot.se/2012/06/binary-log-group-commit-in-mysql-56.html>`_ feature has been replaced with the |MySQL| 5.6 implementation. 
.. [#n-8] Feature has been deprecated in the |Percona Server| 5.1.68-14.6 and |Percona Server| 5.5.30-30.2. It has been replaced by the upstream `innodb_page_size <http://dev.mysql.com/doc/refman/5.6/en/innodb-parameters.html#sysvar_innodb_page_size>`_ version released in |MySQL| 5.6.4.
.. [#n-9] Feature has been replaced by the |MySQL| 5.6 `binary-mode <http://dev.mysql.com/doc/refman/5.6/en/mysql-command-options.html#option_mysql_binary-mode>`_ configuration option.
.. [#n-10] Feature has been replaced by the |MySQL| 5.6 `relay-log-recovery <http://dev.mysql.com/doc/refman/5.6/en/replication-options-slave.html#option_mysqld_relay-log-recovery>`_ implementation.
.. [#n-11] Feature has been replaced by the |MySQL| 5.6 `table_definition_cache <https://dev.mysql.com/doc/refman/5.6/en/server-system-variables.html#sysvar_table_definition_cache>`_ implementation.
.. [#n-12] Feature has been replaced by the |MySQL| 5.6 `Improved Tablespace Management <https://dev.mysql.com/doc/refman/5.6/en/innodb-performance.html#innodb-tablespace-management>`_ implementation.
.. [#n-13] Feature has been replaced by the |MySQL| 5.6 `preloading the InnoDB buffer pool at startup <https://dev.mysql.com/doc/refman/5.6/en/innodb-performance.html#innodb-preload-buffer-pool>`_ implementation.
.. [#n-14] Feature has been replaced by the 5.6' `ALGORITHM= option <http://dev.mysql.com/doc/refman/5.6/en/alter-table.html>`_ implementation. 
.. [#n-15] Feature has been replaced by the |MySQL| 5.6 `Persistent Optimizer Statistics for InnoDB Tables <https://dev.mysql.com/doc/refman/5.6/en/innodb-performance.html#innodb-persistent-stats>`_ implementation.
