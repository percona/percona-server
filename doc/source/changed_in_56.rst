.. _changed_in_56:

=============================
Changed in Percona Server 5.6
=============================

|Percona Server| 5.6 is based on |MySQL| 5.6 and incorporates many of the improvements found in |Percona Server| 5.5.

Features removed from |Percona Server| 5.6 that were available in |Percona Server| 5.5
=======================================================================================

Some features that were present in |Percona Server| 5.5 have been removed in |Percona Server| 5.6. These are:

 * ``SHOW [GLOBAL] TEMPORARY TABLES`` functionality is now only available via the ``INFORMATION_SCHEMA`` tables :table:`TEMPORARY_TABLES` and :table:`GLOBAL_TEMPORARY_TABLES`.
 * `InnoDB timer-based Concurrency Throttling <http://www.percona.com/doc/percona-server/5.5/performance/innodb_thread_concurrency_timer_based.html>`_
 * `InnoDB Recovery Stats <http://www.percona.com/doc/percona-server/5.5/management/innodb_recovery_patches.html>`_
 * Rows_read counters in :ref:`slow_extended` and ``SHOW PROCESSLIST`` had a very fuzzy meaning so they were removed.

Replaced features that were present in |Percona Server| 5.5
===========================================================

Some features that were present in |Percona Server| 5.5 have been replaced by a different implementation of the same/similar functionality in |Percona Server| 5.6. These are:

 * ``SHOW ENGINE INNODB STATUS`` section "OLDEST VIEW" has been replaced by the :table:`XTRADB_READ_VIEW` INFORMATION_SCHEMA table.
 * ``SHOW ENGINE INNODB STATUS`` sections on memory usage for InnoDB/XtraDB hash tables has been replaced by the :table:`XTRADB_INTERNAL_HASH_TABLES` INFORMATION_SCHEMA table.
 * The :table:`INNODB_RSEG` table has been renamed to :table:`XTRADB_RSEG`.
 * :ref:`buff_read_ahead_area` has been implemented differently. Buffer read-ahead area size is now precalculated once per buffer pool instance initialization instead of hardcoding it at 64MB (like it was done in previous |Percona Server| versions).
 * :ref:`response_time_distribution` feature has been implemented as a plugin. It has the following changes from the 5.5 implementation:
   - the plugin requires installation before the feature can be used;
   - variable :variable:`have_response_time_distribution` has been removed. The presence of the feature can be determined by querying ``SHOW PLUGINS`` instead; 
   - replication updates performed by the slave SQL threads are not tracked; 
   - command ``SHOW QUERY_RESPONSE_TIME;`` has been removed in favor of :table:`QUERY_RESPONSE_TIME` table;
   - command ``FLUSH QUERY_RESPONSE_TIME;`` has been replaced with :variable:`query_response_time_flush` variable.

Features available in |Percona Server| 5.5 that have been replaced with |MySQL| 5.6 features
============================================================================================

Some |Percona Server| 5.5 features have been replaced by similar or equivalent |MySQL| 5.6 features, so we now keep the |MySQL| 5.6 implementations in |Percona Server| 5.6. These are:

 * `Crash-Resistant Replication <http://www.percona.com/doc/percona-server/5.5/reliability/crash_resistant_replication.html>`_ has been replaced by |MySQL| crash safe replication
 * `Improved InnoDB I/O Scalability <http://www.percona.com/doc/percona-server/5.5/scalability/innodb_io_55.html>`_ patches have been replaced by improvements and changes in MySQL 5.6, although Percona may make improvements in the future
 * `InnoDB Data Dictionary Size Limit <http://www.percona.com/doc/percona-server/5.5/management/innodb_dict_size_limit.html>`_ has been replaced by |MySQL| 5.6 using the existing table-definition-cache variable to limit the size of the |InnoDB| data dictionary.
 * `Expand Table Import <http://www.percona.com/doc/percona-server/5.5/management/innodb_expand_import.html>`_ has been replaced by |MySQL| "InnoDB transportable tablespaces"
 * The |InnoDB| data dictionary INFORMATION_SCHEMA tables have been superseded by the |MySQL| implementations 
 * |XtraDB| SYS_STATS persistent table and index statistics has been replaced by the MySQL 5.6 implementation
 * `Dump/Restore of the Buffer Pool <http://www.percona.com/doc/percona-server/5.5/management/innodb_lru_dump_restore.html>`_ is now available in |MySQL| 5.6, so we have replaced the |Percona Server| implementation with the |MySQL| `one <http://dev.mysql.com/doc/refman/5.6/en/innodb-performance.html#innodb-preload-buffer-pool>`_. The upstream implementation doesn't have the periodic dump feature, but it's possible to set it up by using the `event scheduler <https://dev.mysql.com/doc/refman/5.6/en/events.html>`_ and the new `innodb_buffer_pool_dump_now <http://dev.mysql.com/doc/refman/5.6/en/innodb-parameters.html#sysvar_innodb_buffer_pool_dump_now>`_ variable. The following example shows how to implement a periodic buffer pool dump every hour: 

   .. code-block:: mysql 

     mysql> CREATE EVENT automatic_bufferpool_dump 
            ON SCHEDULE EVERY 1 HOUR 
            DO 
              SET global innodb_buffer_pool_dump_now=ON;
 * `fast_index_creation <http://www.percona.com/doc/percona-server/5.5/management/innodb_fast_index_creation.html>`_ (replaced by |MySQL| 5.6's `ALGORITHM= option <http://dev.mysql.com/doc/refman/5.6/en/alter-table.html>`_). 
 * :ref:`Fast InnoDB Checksum <ps55:innodb_fast_checksum_page>` has been deprecated after |Percona Server| 5.5.28-29.2 because the :variable:`innodb_checksum_algorithm` variable in |MySQL| 5.6 makes it redundant. If this feature was enabled, turning it off before the upgrade requires table(s) to be dumped and imported, since it will fail to start on data files created when :variable:`innodb_fast_checksum` was enabled. As an alternative you can use :program:`innochecksum` from |MySQL| 5.7 as described in this `blogpost <http://dbadojo.com/2015/07/16/innodb_fast_checksum_mysql56_upgrade/>`_. 
 * :ref:`Handle BLOB End of Line <ps55:mysql_remove_eol_carret>` feature has been replaced by |MySQL| 5.6 `binary-mode <http://dev.mysql.com/doc/refman/5.6/en/mysql-command-options.html#option_mysql_binary-mode>`_ configuration option.
 * |Percona Server| 5.5 implemented ``utf8_general50_ci`` and ``ucs2_general50_ci`` collations as a fix for the upstream bug: :mysqlbug:`27877`. These are now being replaced by |MySQL| 5.6 ``utf8_general_mysql500_ci`` and ``ucs2_general_mysql500_ci`` collations.
 * |Percona Server| ``INFORMATION_SCHEMA`` ``_STATS`` tables in 5.5 have been replaced by new tables in |MySQL| 5.6: ``INNODB_SYS_TABLES``, ``INNODB_SYS_INDEXES``, ``INNODB_SYS_COLUMNS``, ``INNODB_SYS_FIELDS``, ``INNODB_SYS_FOREIGN``, ``INNODB_SYS_FOREIGN_COLS``, ``INNODB_SYS_TABLESTATS`` (although |MySQL| 5.6 does not have ``MYSQL_HANDLES_OPENED``, instead it has ``REF_COUNT``). Following tables haven't been implemented in |MySQL| 5.6 but information is available in other tables: ``INNODB_SYS_STATS`` - use ``MYSQL.INNODB_(INDEX|TABLE)_STATS`` instead, ``INNODB_TABLE_STATS`` - use ``INNODB_SYS_TABLESTATS`` or ``MYSQL.INNODB_TABLE_STATS`` instead, and ``INNODB_INDEX_STATS`` - use ``MYSQL.INNODB_INDEX_STATS`` instead.
 
Features ported from |Percona Server| 5.5 to |Percona Server| 5.6
==================================================================

Following features were ported from |Percona Server| 5.5 to |Percona Server| 5.6: 

 ================================================= ===================
 Feature Ported                                     Version
 ================================================= ===================
 :ref:`threadpool`                                  :rn:`5.6.10-60.2`
 :ref:`atomic_fio`                                  :rn:`5.6.11-60.3`
 :ref:`innodb_io_page`                              :rn:`5.6.11-60.3`
 :ref:`innodb_numa_support`                         :rn:`5.6.11-60.3`
 :ref:`log_warning_suppress`                        :rn:`5.6.11-60.3`
 :ref:`improved_memory_engine`                      :rn:`5.6.11-60.3`
 :ref:`maximum_binlog_files`                        :rn:`5.6.11-60.3`
 :ref:`log_connection_error`                        :rn:`5.6.11-60.3`
 :ref:`error_pad`                                   :rn:`5.6.11-60.3`
 :ref:`show_slave_status_nolock`                    :rn:`5.6.11-60.3`
 :ref:`udf_percona_toolkit`                         :rn:`5.6.11-60.3`
 :ref:`innodb_fake_changes_page`                    :rn:`5.6.11-60.3`
 :ref:`innodb_kill_idle_trx`                        :rn:`5.6.11-60.3`
 :ref:`enforce_engine`                              :rn:`5.6.11-60.3`
 :ref:`psaas_utility_user`                          :rn:`5.6.11-60.3`
 :ref:`secure_file_priv_extended`                   :rn:`5.6.11-60.3`
 :ref:`expanded_option_modifiers`                   :rn:`5.6.11-60.3`
 :ref:`changed_page_tracking`                       :rn:`5.6.11-60.3`
 :ref:`pam_plugin`                                  :rn:`5.6.11-60.3`
 :ref:`user_stats`                                  :rn:`5.6.11-60.3`
 :ref:`slow_extended`                               :rn:`5.6.11-60.3`
 :ref:`innodb_show_status`                          :rn:`5.6.11-60.3`
 :ref:`innodb_deadlocks_page`                       :rn:`5.6.11-60.3`
 :ref:`mysql_syslog`                                :rn:`5.6.11-60.3`
 :ref:`show_engines`                                :rn:`5.6.11-60.3`
 :ref:`thread_based_profiling`                      :rn:`5.6.11-60.3`
 :ref:`buff_read_ahead_area`                        :rn:`5.6.13-60.5`
 :ref:`innodb_split_buf_pool_mutex`                 :rn:`5.6.13-60.6`
 :ref:`innodb_adaptive_hash_index_partitions_page`  :rn:`5.6.13-60.6`
 :ref:`handlersocket_page`                          :rn:`5.6.17-66.0`
 :ref:`response_time_distribution`                  :rn:`5.6.21-69.0`
 ================================================= ===================

List of status variables that are no longer available in |Percona Server| 5.6
=============================================================================

Following status variables available in |Percona Server| 5.5 are no longer present in |Percona Server| 5.6:

.. tabularcolumns:: |p{8cm}|p{7.5cm}|

.. list-table::
   :header-rows: 1

   * - Status Variables
     - Replaced by
   * - ``Com_show_temporary_tables``
     - This variable has been removed together with the "SHOW [GLOBAL] TEMPORARY TABLES" statement, whose call number it was counting. The information about temporary tables is available via the ``INFORMATION_SCHEMA`` tables :table:`TEMPORARY_TABLES` and :table:`GLOBAL_TEMPORARY_TABLES`
   * - ``Flashcache_enabled``
     - information if the Flashcache support has been enabled has not been ported to |Percona Server| 5.6
   * - ``Innodb_adaptive_hash_cells``
     - this variable has not been ported to |Percona Server| 5.6
   * - ``Innodb_adaptive_hash_heap_buffers``
     - this variable has not been ported to |Percona Server| 5.6
   * - ``Innodb_adaptive_hash_hash_searches``      
     - replaced by ``adaptive_hash_searches`` counter in ``INFORMATION_SCHEMA`` ``INNODB_METRICS`` `table <http://dev.mysql.com/doc/refman/5.6/en/innodb-metrics-table.html>`_
   * - ``Innodb_adaptive_hash_non_hash_searches``
     - replaced by ``adaptive_hash_searches_btree`` counter in ``INFORMATION_SCHEMA`` ``INNODB_METRICS`` `table <http://dev.mysql.com/doc/refman/5.6/en/innodb-metrics-table.html>`_
   * - ``Innodb_checkpoint_target_age``
     - replaced by `MySQL 5.6 flushing <http://dev.mysql.com/doc/refman/5.6/en/innodb-performance.html#innodb-lru-background-flushing>`_ implementation
   * - ``Innodb_dict_tables``
     - :ref:`InnoDB Data Dictionary Size Limit <ps55:innodb_dict_size_limit_page>` feature has been replaced by the new MySQL 5.6 `table_definition_cache <https://dev.mysql.com/doc/refman/5.6/en/server-system-variables.html#sysvar_table_definition_cache>`_ implementation
   * - ``Innodb_master_thread_1_second_loops``
     - new |InnoDB| master thread behavior makes this variable redundant
   * - ``Innodb_master_thread_10_second_loops``
     - new |InnoDB| master thread behavior makes this variable redundant
   * - ``Innodb_master_thread_background_loops``
     - new |InnoDB| master thread behavior makes this variable redundant
   * - ``Innodb_master_thread_main_flush_loops``
     - new |InnoDB| master thread behavior makes this variable redundant
   * - ``Innodb_master_thread_sleeps``
     - replaced by ``innodb_master_thread_sleeps`` counter in ``INFORMATION_SCHEMA`` ``INNODB_METRICS`` `table <http://dev.mysql.com/doc/refman/5.6/en/innodb-metrics-table.html>`_
   * - ``binlog_commits``
     - :ref:`Binary Log Group Commit <ps55:binary_group_commit>` feature has been replaced with the |MySQL| 5.6 implementation that doesn't have this status variable.
   * - ``binlog_group_commits``
     - :ref:`Binary Log Group Commit <ps55:binary_group_commit>` feature has been replaced with the |MySQL| 5.6 implementation that doesn't have this status variable.


List of system variables that are no longer available in |Percona Server| 5.6
=============================================================================

Following system variables available in |Percona Server| 5.5 are no longer present in |Percona Server| 5.6:

.. warning::

   |Percona Server| 5.6 won't be able to start if some of these variables are set in the server's configuration file.

.. tabularcolumns:: |p{8cm}|p{7.5cm}|

.. list-table::
   :header-rows: 1

   * - System Variables
     - Feature Comment
   * - :variable:`fast_index_creation`                     
     - replaced by using MySQL's `ALGORITHM option <http://dev.mysql.com/doc/refman/5.6/en/alter-table.html>`_
   * - :variable:`have_flashcache`                         
     - Information if the server has been compiled with the Flashcache support has not been ported to |Percona Server| 5.6
   * - :variable:`have_response_time_distribution`
     - :ref:`Response Time Distribution <ps55:response_time_distribution>` feature has been ported to |Percona Server| 5.6 without this variable
   * - :variable:`innodb_adaptive_flushing_method`         
     - replaced by MySQL 5.6 `flushing <http://dev.mysql.com/doc/refman/5.6/en/innodb-performance.html#innodb-lru-background-flushing>`_ implementation
   * - :variable:`innodb_blocking_buffer_pool_restore`     
     - variable doesn't have direct replacement in |MySQL| 5.6. Feature will be implemented in a `future <https://blueprints.launchpad.net/percona-server/+spec/blocking-buffer-pool-restore>`_ |Percona Server| 5.6 release
   * - :variable:`innodb_buffer_pool_restore_at_startup`   
     - replaced by `innodb_buffer_pool_load_at_startup <http://dev.mysql.com/doc/refman/5.6/en/innodb-parameters.html#sysvar_innodb_buffer_pool_load_at_startup>`_
   * - :variable:`innodb_buffer_pool_shm_checksum`         
     - variable has been deprecated and removed in |Percona Server| 5.5
   * - :variable:`innodb_buffer_pool_shm_key`              
     - variable has been deprecated and removed in |Percona Server| 5.5
   * - :variable:`innodb_checkpoint_age_target`            
     - replaced by `MySQL 5.6 flushing <http://dev.mysql.com/doc/refman/5.6/en/innodb-performance.html#innodb-lru-background-flushing>`_ implementation
   * - :variable:`innodb_dict_size_limit`                  
     - replaced by |MySQL| 5.6 new `table_definition_cache <https://dev.mysql.com/doc/refman/5.6/en/server-system-variables.html#sysvar_table_definition_cache>`_ implementation
   * - :variable:`innodb_doublewrite_file`                 
     - :ref:`Configuration of the Doublewrite Buffer <ps55:innodb_doublewrite_path>` feature containing this variable has not been ported to |Percona Server| 5.6
   * - :variable:`innodb_fast_checksum`                    
     - replaced by `innodb_checksum_algorithm <http://dev.mysql.com/doc/refman/5.6/en/innodb-parameters.html#sysvar_innodb_checksum_algorithm>`_ 
   * - :variable:`innodb_flush_neighbor_pages`             
     - replaced by `innodb_flush_neighbors <http://dev.mysql.com/doc/refman/5.6/en/innodb-parameters.html#sysvar_innodb_flush_neighbors>`_
   * - :variable:`innodb_ibuf_accel_rate`                  
     - :ref:`Configurable Insert Buffer <ps55:innodb_insert_buffer>` feature containing this variable has not been ported to |Percona Server| 5.6 
   * - :variable:`innodb_ibuf_active_contract`             
     - :ref:`Configurable Insert Buffer <ps55:innodb_insert_buffer>` feature containing this variable has not been ported to |Percona Server| 5.6 
   * - :variable:`innodb_ibuf_max_size`                    
     - :ref:`Configurable Insert Buffer <ps55:innodb_insert_buffer>` feature containing this variable has not been ported to |Percona Server| 5.6 
   * - :variable:`innodb_import_table_from_xtrabackup`     
     - replaced by MySQL `transportable tablespaces <http://dev.mysql.com/doc/refman/5.6/en/tablespace-copying.html>`_
   * - :variable:`innodb_lazy_drop_table`                  
     - variable has been deprecated and removed in |Percona Server| 5.5
   * - :variable:`innodb_merge_sort_block_size`            
     - replaced by `innodb_sort_buffer_size <http://dev.mysql.com/doc/refman/5.6/en/innodb-parameters.html#sysvar_innodb_sort_buffer_size>`_
   * - :variable:`innodb_page_size`                        
     - replaced by `innodb_page_size <http://dev.mysql.com/doc/refman/5.6/en/innodb-parameters.html#sysvar_innodb_page_size>`_
   * - :variable:`innodb_read_ahead`                       
     - replaced by MySQL `Read-Ahead Algorithm <http://dev.mysql.com/doc/refman/5.6/en/innodb-performance.html#innodb-performance-read_ahead>`_ implementation, `innodb_random_read_ahead <http://dev.mysql.com/doc/refman/5.6/en/innodb-parameters.html#sysvar_innodb_random_read_ahead>`_
   * - :variable:`innodb_recovery_stats`                   
     - :ref:`InnoDB Recovery Stats <ps55:innodb_recovery_patches>` feature containing this variable has not been ported to |Percona Server| 5.6
   * - :variable:`innodb_recovery_update_relay_log`        
     - replaced by `relay-log-recovery <http://dev.mysql.com/doc/refman/5.6/en/replication-options-slave.html#option_mysqld_relay-log-recovery>`_ 
   * - :variable:`innodb_stats_auto_update`                
     - replaced by `innodb_stats_auto_recalc <http://dev.mysql.com/doc/refman/5.6/en/innodb-parameters.html#sysvar_innodb_stats_auto_recalc>`_
   * - :variable:`innodb_stats_update_need_lock`           
     - variable has not been ported to |Percona Server| 5.6
   * - :variable:`innodb_thread_concurrency_timer_based`   
     - :ref:`InnoDB timer-based Concurrency Throttling <ps55:innodb_thread_concurrency_timer_based_page>` feature containing this variable has not been ported to |Percona Server| 5.6
   * - :variable:`innodb_use_sys_stats_table`              
     - variable has been replaced by `Persistent Optimizer Statistics <https://dev.mysql.com/doc/refman/5.6/en/innodb-performance.html#innodb-persistent-stats>`_ implementation in |MySQL| 5.6
   * - :variable:`log_slow_admin_statements`               
     - the upstream variable has the same functionality
   * - :variable:`log_slow_slave_statements`               
     - the upstream variable has the same functionality
   * - :variable:`optimizer_fix`
     - this variable has been deprecated and removed in |Percona Server| 5.5
   * - :variable:`query_response_time_range_base`          
     - :ref:`Response Time Distribution <ps55:response_time_distribution>` feature containing this variable has been ported to |Percona Server| 5.6, but requires plugin installation in order to work. More information can be found in :ref:`response_time_distribution` documentation.
   * - :variable:`query_response_time_stats`               
     - :ref:`Response Time Distribution <ps55:response_time_distribution>` feature containing this variable has been ported to |Percona Server| 5.6, but requires plugin installation in order to work. More information can be found in :ref:`response_time_distribution` documentation. 

