.. _tokudb_performance_schema:

=======================================
TokuDB Performance Schema Integration
=======================================

.. Important:: 

   Starting with :ref:`8.0.28-19`, the TokuDB storage engine is no longer supported. We have removed the storage engine from the installation packages and disabled the storage engine in our binary builds.

   Starting with :ref:`8.0.26-16`, the binary builds and packages include but disable the TokuDB storage engine plugins. The ``tokudb_enabled`` option and the ``tokudb_backup_enabled`` option control the state of the plugins and have a default setting of ``FALSE``. The result of attempting to load the plugins are the plugins fail to initialize and print a deprecation message.

   We recommend :ref:`migrate-myrocks`. To enable the plugins to migrate to another storage engine, set the ``tokudb_enabled`` and ``tokudb_backup_enabled`` options to ``TRUE`` in your ``my.cnf`` file and restart your server instance. Then, you can load the plugins.

   The TokuDB Storage Engine was `declared as deprecated <https://www.percona.com/doc/percona-server/8.0/release-notes/Percona-Server-8.0.13-3.html>`__ in Percona Server for MySQL 8.0. For more information, see the Percona blog post: `Heads-Up: TokuDB Support Changes and Future Removal from Percona Server for MySQL 8.0 <https://www.percona.com/blog/2021/05/21/tokudb-support-changes-and-future-removal-from-percona-server-for-mysql-8-0/>`__.

*TokuDB* is integrated with `Performance Schema
<https://dev.mysql.com/doc/refman/8.0/en/innodb-performance-schema.html>`_

This integration can be used for profiling additional *TokuDB* operations.

*TokuDB* instruments available in Performance Schema can be seen in
:ref:`PERFORMANCE_SCHEMA.SETUP_INSTRUMENTS` table:

.. code-block:: mysql

  mysql> SELECT * FROM performance_schema.setup_instruments WHERE NAME LIKE "%/fti/%";
  +------------------------------------------------------------+---------+-------+
  | NAME                                                       | ENABLED | TIMED |
  +------------------------------------------------------------+---------+-------+
  | wait/synch/mutex/fti/kibbutz_mutex                         | NO      | NO    |
  | wait/synch/mutex/fti/minicron_p_mutex                      | NO      | NO    |
  | wait/synch/mutex/fti/queue_result_mutex                    | NO      | NO    |
  | wait/synch/mutex/fti/tpool_lock_mutex                      | NO      | NO    |
  | wait/synch/mutex/fti/workset_lock_mutex                    | NO      | NO    |
  | wait/synch/mutex/fti/bjm_jobs_lock_mutex                   | NO      | NO    |
  | wait/synch/mutex/fti/log_internal_lock_mutex               | NO      | NO    |
  | wait/synch/mutex/fti/cachetable_ev_thread_lock_mutex       | NO      | NO    |
  | wait/synch/mutex/fti/cachetable_disk_nb_mutex              | NO      | NO    |
  | wait/synch/mutex/fti/safe_file_size_lock_mutex             | NO      | NO    |
  | wait/synch/mutex/fti/cachetable_m_mutex_key                | NO      | NO    |
  | wait/synch/mutex/fti/checkpoint_safe_mutex                 | NO      | NO    |
  | wait/synch/mutex/fti/ft_ref_lock_mutex                     | NO      | NO    |
  | wait/synch/mutex/fti/ft_open_close_lock_mutex              | NO      | NO    |
  | wait/synch/mutex/fti/loader_error_mutex                    | NO      | NO    |
  | wait/synch/mutex/fti/bfs_mutex                             | NO      | NO    |
  | wait/synch/mutex/fti/loader_bl_mutex                       | NO      | NO    |
  | wait/synch/mutex/fti/loader_fi_lock_mutex                  | NO      | NO    |
  | wait/synch/mutex/fti/loader_out_mutex                      | NO      | NO    |
  | wait/synch/mutex/fti/result_output_condition_lock_mutex    | NO      | NO    |
  | wait/synch/mutex/fti/block_table_mutex                     | NO      | NO    |
  | wait/synch/mutex/fti/rollback_log_node_cache_mutex         | NO      | NO    |
  | wait/synch/mutex/fti/txn_lock_mutex                        | NO      | NO    |
  | wait/synch/mutex/fti/txn_state_lock_mutex                  | NO      | NO    |
  | wait/synch/mutex/fti/txn_child_manager_mutex               | NO      | NO    |
  | wait/synch/mutex/fti/txn_manager_lock_mutex                | NO      | NO    |
  | wait/synch/mutex/fti/treenode_mutex                        | NO      | NO    |
  | wait/synch/mutex/fti/locktree_request_info_mutex           | NO      | NO    |
  | wait/synch/mutex/fti/locktree_request_info_retry_mutex_key | NO      | NO    |
  | wait/synch/mutex/fti/manager_mutex                         | NO      | NO    |
  | wait/synch/mutex/fti/manager_escalation_mutex              | NO      | NO    |
  | wait/synch/mutex/fti/db_txn_struct_i_txn_mutex             | NO      | NO    |
  | wait/synch/mutex/fti/manager_escalator_mutex               | NO      | NO    |
  | wait/synch/mutex/fti/indexer_i_indexer_lock_mutex          | NO      | NO    |
  | wait/synch/mutex/fti/indexer_i_indexer_estimate_lock_mutex | NO      | NO    |
  | wait/synch/mutex/fti/fti_probe_1                           | NO      | NO    |
  | wait/synch/rwlock/fti/multi_operation_lock                 | NO      | NO    |
  | wait/synch/rwlock/fti/low_priority_multi_operation_lock    | NO      | NO    |
  | wait/synch/rwlock/fti/cachetable_m_list_lock               | NO      | NO    |
  | wait/synch/rwlock/fti/cachetable_m_pending_lock_expensive  | NO      | NO    |
  | wait/synch/rwlock/fti/cachetable_m_pending_lock_cheap      | NO      | NO    |
  | wait/synch/rwlock/fti/cachetable_m_lock                    | NO      | NO    |
  | wait/synch/rwlock/fti/result_i_open_dbs_rwlock             | NO      | NO    |
  | wait/synch/rwlock/fti/checkpoint_safe_rwlock               | NO      | NO    |
  | wait/synch/rwlock/fti/cachetable_value                     | NO      | NO    |
  | wait/synch/rwlock/fti/safe_file_size_lock_rwlock           | NO      | NO    |
  | wait/synch/rwlock/fti/cachetable_disk_nb_rwlock            | NO      | NO    |
  | wait/synch/cond/fti/result_state_cond                      | NO      | NO    |
  | wait/synch/cond/fti/bjm_jobs_wait                          | NO      | NO    |
  | wait/synch/cond/fti/cachetable_p_refcount_wait             | NO      | NO    |
  | wait/synch/cond/fti/cachetable_m_flow_control_cond         | NO      | NO    |
  | wait/synch/cond/fti/cachetable_m_ev_thread_cond            | NO      | NO    |
  | wait/synch/cond/fti/bfs_cond                               | NO      | NO    |
  | wait/synch/cond/fti/result_output_condition                | NO      | NO    |
  | wait/synch/cond/fti/manager_m_escalator_done               | NO      | NO    |
  | wait/synch/cond/fti/lock_request_m_wait_cond               | NO      | NO    |
  | wait/synch/cond/fti/queue_result_cond                      | NO      | NO    |
  | wait/synch/cond/fti/ws_worker_wait                         | NO      | NO    |
  | wait/synch/cond/fti/rwlock_wait_read                       | NO      | NO    |
  | wait/synch/cond/fti/rwlock_wait_write                      | NO      | NO    |
  | wait/synch/cond/fti/rwlock_cond                            | NO      | NO    |
  | wait/synch/cond/fti/tp_thread_wait                         | NO      | NO    |
  | wait/synch/cond/fti/tp_pool_wait_free                      | NO      | NO    |
  | wait/synch/cond/fti/frwlock_m_wait_read                    | NO      | NO    |
  | wait/synch/cond/fti/kibbutz_k_cond                         | NO      | NO    |
  | wait/synch/cond/fti/minicron_p_condvar                     | NO      | NO    |
  | wait/synch/cond/fti/locktree_request_info_retry_cv_key     | NO      | NO    |
  | wait/io/file/fti/tokudb_data_file                          | YES     | YES   |
  | wait/io/file/fti/tokudb_load_file                          | YES     | YES   |
  | wait/io/file/fti/tokudb_tmp_file                           | YES     | YES   |
  | wait/io/file/fti/tokudb_log_file                           | YES     | YES   |
  +------------------------------------------------------------+---------+-------+

For *TokuDB*-related objects, following clauses can be used when querying
Performance Schema tables:

 * ``WHERE EVENT_NAME LIKE '%fti%'`` or
 * ``WHERE NAME LIKE '%fti%'``

For example, to get the information about *TokuDB* related events you can query
:ref:`PERFORMANCE_SCHEMA.events_waits_summary_global_by_event_name` like:

.. code-block:: mysql

  mysql> SELECT * FROM performance_schema.events_waits_summary_global_by_event_name WHERE EVENT_NAME LIKE '%fti%';

  +-----------------------------------------+------------+----------------+----------------+----------------+----------------+
  | EVENT_NAME                              | COUNT_STAR | SUM_TIMER_WAIT | MIN_TIMER_WAIT | AVG_TIMER_WAIT | MAX_TIMER_WAIT |
  +-----------------------------------------+------------+----------------+----------------+----------------+----------------+
  | wait/synch/mutex/fti/kibbutz_mutex      |          0 |              0 |              0 |              0 |              0 |
  | wait/synch/mutex/fti/minicron_p_mutex   |          0 |              0 |              0 |              0 |              0 |
  | wait/synch/mutex/fti/queue_result_mutex |          0 |              0 |              0 |              0 |              0 |
  | wait/synch/mutex/fti/tpool_lock_mutex   |          0 |              0 |              0 |              0 |              0 |
  | wait/synch/mutex/fti/workset_lock_mutex |          0 |              0 |              0 |              0 |              0 |
  ...
  | wait/io/file/fti/tokudb_data_file       |         30 |      179862410 |              0 |        5995080 |       68488420 |
  | wait/io/file/fti/tokudb_load_file       |          0 |              0 |              0 |              0 |              0 |
  | wait/io/file/fti/tokudb_tmp_file        |          0 |              0 |              0 |              0 |              0 |
  | wait/io/file/fti/tokudb_log_file        |       1367 |  2925647870145 |              0 |     2140195785 |    12013357720 |
  +-----------------------------------------+------------+----------------+----------------+----------------+----------------+
  71 rows in set (0.02 sec)
