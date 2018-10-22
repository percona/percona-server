.. _ps_versions_comparison:

=======================================================
List of features available in |Percona Server| releases
=======================================================

.. tabularcolumns:: |p{5cm}|p{5cm}|p{5cm}|

.. list-table::
   :header-rows: 1

   * - |Percona Server| 5.6
     - |Percona Server| 5.7
     - |Percona Server| 8.0
   * - :ref:`Improved Buffer Pool Scalability <ps56:innodb_split_buf_pool_mutex>`
     - :ref:`Improved Buffer Pool Scalability <ps57:innodb_split_buf_pool_mutex>`
     - :ref:`Improved Buffer Pool Scalability <ps80:innodb_split_buf_pool_mutex>`
   * - :ref:`Improved InnoDB I/O Scalability <ps56:innodb_io_page>`
     - :ref:`Improved InnoDB I/O Scalability <ps57:innodb_io_page>`
     - :ref:`Improved InnoDB I/O Scalability <ps80:innodb_io_page>`
   * - |replaced| [#n-1]_
     - |replaced| [#n-1]_
     - |replaced| [#n-1]_
   * - :ref:`Multiple Adaptive Hash Search Partitions <ps56:innodb_adaptive_hash_index_partitions_page>`
     - :ref:`Multiple Adaptive Hash Search Partitions <ps57:innodb_adaptive_hash_index_partitions_page>`
     - :ref:`Multiple Adaptive Hash Search Partitions <ps80:innodb_adaptive_hash_index_partitions_page>`
   * - |replaced|
     - |replaced|
     - |replaced|
   * - Replaced by the upstream fix [#n-3]_
     - Replaced by the upstream fix
     - Replaced by the upstream fix
   * - :ref:`Atomic write support for Fusion-io devices <ps56:atomic_fio>`
     - :ref:`Atomic write support for Fusion-io devices <ps57:atomic_fio>`
     - :ref:`Atomic write support for Fusion-io devices <ps80:atomic_fio>`
   * - :ref:`Query Cache Enhancements <ps56:query_cache_enhance>`
     - :ref:`Query Cache Enhancements <ps57:query_cache_enhance>`
     - :ref:`Query Cache Enhancements <ps80:query_cache_enhance>`
   * - |replaced| [#n-4]_
     - |replaced|
     - |replaced|
   * - |replaced| [#n-5]_
     - |replaced|
     - |replaced|
   * - :ref:`Improved NUMA support <ps56:innodb_numa_support>`
     - :ref:`Improved NUMA support <ps57:innodb_numa_support>`
     - |-implemented|
   * - :ref:`Thread Pool <ps56:threadpool>`
     - :ref:`Thread Pool <ps56:threadpool>`
     - :ref:`Thread Pool <ps56:threadpool>`
   * - |replaced| [#n-7]_
     - |replaced|
     - |replaced|
   * - |replaced| [#n-8]_
     - |replaced|
     - |replaced|
   * - :ref:`Suppress Warning Messages <ps56:log_warning_suppress>`
     - :ref:`Suppress Warning Messages <ps57:log_warning_suppress>`
     - :ref:`Suppress Warning Messages <ps80:log_warning_suppress>`
   * - |replaced| [#n-9]_
     - |replaced|
     - |replaced|
   * - :ref:`Ability to change database for mysqlbinlog <ps56:mysqlbinlog_change_db>`
     - :ref:`Ability to change database for mysqlbinlog <ps57:mysqlbinlog_change_db>`
     - :ref:`Ability to change database for mysqlbinlog <ps80:mysqlbinlog_change_db>`
   * - :ref:`Fixed Size for the Read Ahead Area <ps56:buff_read_ahead_area>`
     - :ref:`Fixed Size for the Read Ahead Area <ps57:buff_read_ahead_area>`
     - :ref:`Fixed Size for the Read Ahead Area <ps80:buff_read_ahead_area>`      
   * - :ref:`Improved MEMORY Storage Engine <ps56:improved_memory_engine>`
     - :ref:`Improved MEMORY Storage Engine <ps57:improved_memory_engine>`
     - :ref:`Improved MEMORY Storage Engine <ps80:improved_memory_engine>`
   * - :ref:`Restricting the number of binlog files <ps56:maximum_binlog_files>`
     - :ref:`Restricting the number of binlog files <ps57:maximum_binlog_files>`
     - :ref:`Restricting the number of binlog files <ps80:maximum_binlog_files>`
   * - :ref:`Ignoring missing tables in mysqldump <ps56:mysqldump_ignore_create_error>`
     - :ref:`Ignoring missing tables in mysqldump <ps57:mysqldump_ignore_create_error>`
     - :ref:`Ignoring missing tables in mysqldump <ps80:mysqldump_ignore_create_error>`
   * - :ref:`Too Many Connections Warning <ps56:log_connection_error>`
     - :ref:`Too Many Connections Warning <ps57:log_connection_error>`
     - :ref:`Too Many Connections Warning <ps80:log_connection_error>`
   * - :ref:`Handle Corrupted Tables <ps56:innodb_corrupt_table_action_page>`
     - :ref:`Handle Corrupted Tables <ps57:innodb_corrupt_table_action_page>`
     - :ref:`Handle Corrupted Tables <ps80:innodb_corrupt_table_action_page>`
   * - |replaced| [#n-10]_
     - |replaced|
     - |replaced|
   * - :ref:`Lock-Free SHOW SLAVE STATUS <ps56:show_slave_status_nolock>`
     - :ref:`Lock-Free SHOW SLAVE STATUS <ps57:show_slave_status_nolock>`
     - :ref:`Lock-Free SHOW SLAVE STATUS <ps80:show_slave_status_nolock>`
   * - |replaced| [#n-11]_
     - |replaced|
     - |replaced|
   * - |replaced| [#n-12]_
     - |replaced|
     - |replaced|
   * - |replaced| [#n-13]_
     - |replaced|
     - |replaced|
   * - |replaced| [#n-14]_
     - |replaced|
     - |replaced|
   * - :ref:`Expanded Fast Index Creation <ps56:expanded_innodb_fast_index_creation>`
     - :ref:`Expanded Fast Index Creation <ps56:expanded_innodb_fast_index_creation>`
     - :ref:`Expanded Fast Index Creation <ps56:expanded_innodb_fast_index_creation>`
   * - :ref:`Percona Toolkit UDFs <ps56:udf_percona_toolkit>`
     - :ref:`Percona Toolkit UDFs <ps57:udf_percona_toolkit>`
     - :ref:`Percona Toolkit UDFs <ps80:udf_percona_toolkit>`
   * - :ref:`Support for Fake Changes <ps56:innodb_fake_changes_page>`
     - :ref:`Support for Fake Changes <ps57:innodb_fake_changes_page>`
     - :ref:`Support for Fake Changes <ps80:innodb_fake_changes_page>`
   * - :ref:`Kill Idle Transactions <ps56:innodb_kill_idle_trx>`
     - :ref:`Kill Idle Transactions <ps57:innodb_kill_idle_trx>`
     - :ref:`Kill Idle Transactions <ps80:innodb_kill_idle_trx>`
   * - :ref:`XtraDB changed page tracking <ps56:changed_page_tracking>`
     - :ref:`XtraDB changed page tracking <ps57:changed_page_tracking>`
     - :ref:`XtraDB changed page tracking <ps80:changed_page_tracking>`
   * - :ref:`Enforcing Storage Engine <ps56:enforce_engine>`
     - :ref:`Enforcing Storage Engine <ps56:enforce_engine>`
     - |replaced|
   * - :ref:`Utility user <ps56:psaas_utility_user>`
     - :ref:`Utility user <ps57:psaas_utility_user>`
     - :ref:`Utility user <ps80:psaas_utility_user>`
   * - :ref:`Extending the secure-file-priv server option <ps56:secure_file_priv_extended>`
     - :ref:`Extending the secure-file-priv server option <ps57:secure_file_priv_extended>`
     - :ref:`Extending the secure-file-priv server option <ps80:secure_file_priv_extended>`
   * - :ref:`Expanded Program Option Modifiers <ps56:expanded_option_modifiers>`
     - :ref:`Expanded Program Option Modifiers <ps57:expanded_option_modifiers>`
     - :ref:`Expanded Program Option Modifiers <ps80:expanded_option_modifiers>`
   * - :ref:`PAM Authentication Plugin <ps56:pam_plugin>`
     - :ref:`PAM Authentication Plugin <ps57:pam_plugin>`
     - :ref:`PAM Authentication Plugin <ps80:pam_plugin>`
   * - :ref:`Log Archiving for XtraDB <ps56:log_archiving>`
     - :ref:`Log Archiving for XtraDB <ps57:log_archiving>`
     - :ref:`Log Archiving for XtraDB <ps80:log_archiving>`
   * - |replaced| [#n-15]_
     - |replaced|
     - |replaced|
   * - :ref:`User Statistics <ps56:user_stats>`
     - :ref:`User Statistics <ps57:user_stats>`
     - :ref:`User Statistics <ps80:user_stats>`
   * - :ref:`Slow Query Log <ps56:slow_extended>`
     - :ref:`Slow Query Log <ps57:slow_extended>`
     - :ref:`Slow Query Log <ps80:slow_extended>`
   * - :ref:`Count InnoDB Deadlocks <ps56:innodb_deadlocks_page>`
     - :ref:`Count InnoDB Deadlocks <ps57:innodb_deadlocks_page>`
     - :ref:`Count InnoDB Deadlocks <ps80:innodb_deadlocks_page>`
   * - :ref:`Log All Client Commands (syslog) <ps56:mysql_syslog>`
     - :ref:`Log All Client Commands (syslog) <ps57:mysql_syslog>`
     - :ref:`Log All Client Commands (syslog) <ps80:mysql_syslog>`
   * - :ref:`Response Time Distribution <ps56:response_time_distribution>`
     - :ref:`Response Time Distribution <ps57:response_time_distribution>`
     - :ref:`Response Time Distribution <ps80:response_time_distribution>`
   * - :ref:`Show Storage Engines <ps56:show_engines>`
     - :ref:`Show Storage Engines <ps57:show_engines>`
     - :ref:`Show Storage Engines <ps80:show_engines>`
   * - :ref:`Show Lock Names <ps56:innodb_show_lock_names>`
     - :ref:`Show Lock Names <ps57:innodb_show_lock_names>`
     - :ref:`Show Lock Names <ps80:innodb_show_lock_names>`
   * - :ref:`Process List <ps56:process_list>`
     - :ref:`Process List <ps57:process_list>`
     - :ref:`Process List <ps80:process_list>`
   * - :ref:`Misc. INFORMATION_SCHEMA Tables <ps56:misc_info_schema_tables>`
     - :ref:`Misc. INFORMATION_SCHEMA Tables <ps57:misc_info_schema_tables>`
     - :ref:`Misc. INFORMATION_SCHEMA Tables <ps80:misc_info_schema_tables>`
   * - :ref:`Extended Show Engine InnoDB Status <ps56:innodb_show_status>`
     - :ref:`Extended Show Engine InnoDB Status <ps57:innodb_show_status>`
     - :ref:`Extended Show Engine InnoDB Status <ps80:innodb_show_status>`
   * - :ref:`Thread Based Profiling <ps56:thread_based_profiling>`
     - :ref:`Thread Based Profiling <ps57:thread_based_profiling>`
     - :ref:`Thread Based Profiling <ps80:thread_based_profiling>`
   * - :ref:`XtraDB Performance Improvements for I/O-Bound Highly-Concurrent Workloads <ps56:xtradb_performance_improvements_for_io-bound_highly-concurrent_workloads>`
     - :ref:`XtraDB Performance Improvements for I/O-Bound Highly-Concurrent Workloads <ps57:xtradb_performance_improvements_for_io-bound_highly-concurrent_workloads>`
     - :ref:`XtraDB Performance Improvements for I/O-Bound Highly-Concurrent Workloads <ps80:xtradb_performance_improvements_for_io-bound_highly-concurrent_workloads>`
   * - :ref:`Page cleaner thread tuning <ps56:page_cleaner_tuning>`
     - :ref:`Page cleaner thread tuning <ps57:page_cleaner_tuning>`
     - :ref:`Page cleaner thread tuning <ps80:page_cleaner_tuning>`
   * - :ref:`Statement Timeout <ps56:statement_timeout>`
     - :ref:`Statement Timeout <ps57:statement_timeout>`
     - :ref:`Statement Timeout <ps80:statement_timeout>`
   * - :ref:`Extended SELECT INTO OUTFILE/DUMPFILE <ps56:extended_select_into_outfile>`
     - :ref:`Extended SELECT INTO OUTFILE/DUMPFILE <ps57:extended_select_into_outfile>`
     - :ref:`Extended SELECT INTO OUTFILE/DUMPFILE <ps80:extended_select_into_outfile>`
   * - :ref:`Per-query variable statement <ps56:per_query_variable_statement>`
     - :ref:`Per-query variable statement <ps57:per_query_variable_statement>`
     - :ref:`Per-query variable statement <ps80:per_query_variable_statement>`
   * - :ref:`Extended mysqlbinlog <ps56:extended_mysqlbinlog>`
     - :ref:`Extended mysqlbinlog <ps57:extended_mysqlbinlog>`
     - :ref:`Extended mysqlbinlog <ps80:extended_mysqlbinlog>`
   * - :ref:`Slow Query Log Rotation and Expiration <ps56:slowlog_rotation>`
     - :ref:`Slow Query Log Rotation and Expiration <ps57:slowlog_rotation>`
     - :ref:`Slow Query Log Rotation and Expiration <ps80:slowlog_rotation>`
   * - :ref:`Metrics for scalability measurement <ps56:scalability_metrics_plugin>`
     - :ref:`Metrics for scalability measurement <ps57:scalability_metrics_plugin>`
     - :ref:`Metrics for scalability measurement <ps80:scalability_metrics_plugin>`
   * - :ref:`Audit Log <ps56:audit_log_plugin>`
     - :ref:`Audit Log <ps57:audit_log_plugin>`
     - :ref:`Audit Log <ps80:audit_log_plugin>`
   * - :ref:`Backup Locks <ps56:backup_locks>`
     - :ref:`Backup Locks <ps57:backup_locks>`
     - :ref:`Backup Locks <ps80:backup_locks>`
   * - :ref:`CSV engine mode for standard-compliant quote and comma parsing <ps56:csv_engine_mode>`
     - :ref:`CSV engine mode for standard-compliant quote and comma parsing <ps57:csv_engine_mode>`
     - :ref:`CSV engine mode for standard-compliant quote and comma parsing <ps80:csv_engine_mode>`
   * - :ref:`Super read-only <ps56:super-read-only>`
     - :ref:`Super read-only <ps57:super-read-only>`
     - :ref:`Super read-only <ps80:super-read-only>`

Other Reading
=============

* :ref:`changed_in_56`
* :ref:`upgrading_guide`
* `What Is New in MySQL 5.5 <http://dev.mysql.com/doc/refman/5.5/en/mysql-nutshell.html>`_
* `What Is New in MySQL 5.6 <http://dev.mysql.com/doc/refman/5.6/en/mysql-nutshell.html>`_

.. [#n-1] Feature has been deprecated after |Percona Server| 5.5.11-20.2. It has been replaced with the upstream implementation of `innodb_undo_logs <https://dev.mysql.com/doc/refman/5.6/en/innodb-parameters.html#sysvar_innodb_undo_logs>`_ in |MySQL| 5.6.3.
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

.. |replaced| replace:: Replaced with upstream implementation
.. |-implemented| replace:: Feature not implemented
