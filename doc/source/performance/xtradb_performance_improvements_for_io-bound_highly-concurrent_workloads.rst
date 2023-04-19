.. _xtradb_performance_improvements_for_io-bound_highly-concurrent_workloads:

=========================================================================
XtraDB Performance Improvements for I/O-Bound Highly-Concurrent Workloads
=========================================================================

.. _ps.buffer-pool.free-list.priority-refill:

Priority refill for the buffer pool free list
=============================================

In highly-concurrent I/O-bound workloads the following situation may happen:

 1. Buffer pool free lists are used faster than they are refilled by the LRU
    cleaner thread.

 2. Buffer pool free lists become empty and more and more query and utility
    (i.e. purge) threads stall, checking whether a buffer pool free list has
    became non-empty, sleeping, performing single-page LRU flushes.

 3. The number of buffer pool free list mutex waiters increases.

 4. When the LRU manager thread (or a single page LRU flush by a query thread)
    finally produces a free page, it is starved from putting it on the buffer
    pool free list as it must acquire the buffer pool free list mutex too.
    However, being one thread in up to hundreds, the chances of a prompt
    acquisition are low.

This is addressed by delegating all the LRU flushes to the to the LRU manager
thread, never attempting to evict a page or perform a LRU single page flush by
a query thread, and introducing a backoff algorithm to reduce buffer pool free
list mutex pressure on empty buffer pool free lists. This is controlled through
a new system variable :ref:`innodb_empty_free_list_algorithm`.

.. _innodb_empty_free_list_algorithm:

.. rubric:: ``innodb_empty_free_list_algorithm``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config File
     - Yes
   * - Scope
     - Global
   * - Dynamic
     - Yes
   * - Data type
     - legacy, backoff
   * - Default
     - legacy

When ``legacy`` option is set, server will use the upstream algorithm and when
the ``backoff`` is selected, *Percona* implementation will be used.

.. _lru_manager_threads:

Multi-threaded LRU flusher
==========================

*Percona Server for MySQL* features a true multi-threaded LRU flushing. In this scheme,
each buffer pool instance has its own dedicated LRU manager thread that is
tasked with performing LRU flushes and evictions to refill the free list of that
buffer pool instance. Existing multi-threaded flusher no longer does any LRU
flushing and is tasked with flush list flushing only.

* All threads still synchronize on each coordinator thread iteration. If a
  particular flushing job is stuck on one of the worker threads, the rest will
  idle until the stuck one completes.
* The coordinator thread heuristics focus on flush list adaptive flushing
  without considering the state of free lists, which might be in need of urgent
  refill for a subset of buffer pool instances on a loaded server.
* LRU flushing is serialized with flush list flushing for each buffer pool
  instance, introducing the risk that the right flushing mode will not happen
  for a particular instance because it is being flushed in the other mode.

The following *InnoDB* metrics are no longer accounted, as their semantics do
not make sense under the current LRU flushing design:
``buffer_LRU_batch_flush_avg_time_slot``, ``buffer_LRU_batch_flush_avg_pass``,
``buffer_LRU_batch_flush_avg_time_thread``,
``buffer_LRU_batch_flush_avg_time_est``.

The need for *InnoDB* recovery thread writer threads is also removed,
consequently all associated code is deleted.

.. _doublewrite_buffer:

Doublewrite buffer
===========================

As of *Percona Server for MySQL* 8.0.20-11, the parallel doublewrite buffer is replaced with the `MySQL implementation <https://dev.mysql.com/doc/refman/8.0/en/innodb-doublewrite-buffer.html>`_.

.. _innodb_parallel_doublewrite_path:

.. rubric:: ``innodb_parallel_doublewrite_path``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Scope
     - Global
   * - Dynamic
     - No
   * - Data type
     - String
   * - Default
     - ``xb_doublewrite``

As of *Percona Server for MySQL* 8.0.20-11, this variable is considered **deprecated** and has no effect. You should use `innodb_doublewrite_dir <https://dev.mysql.com/doc/refman/8.0/en/innodb-parameters.html#sysvar_innodb_doublewrite_dir>`_.

This variable is used to specify the location of the parallel doublewrite file.
It accepts both absolute and relative paths. In the latter case they are
treated as relative to the data directory.

*Percona Server for MySQL* has introduced several options, only available in builds
compiled with ``UNIV_PERF_DEBUG`` C preprocessor define.

.. _innodb_sched_priority_master:

.. rubric:: ``innodb_sched_priority_master``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global
   * - Dynamic
     - Yes
   * - Data type
     - Boolean

This variable can be added to the configuration file.

Other Reading
=============

* Bug :mysqlbug:`74637` - make dirty page flushing more adaptive
* Bug :mysqlbug:`67808` - in innodb engine, double write and multi-buffer pool
  instance reduce concurrency
* Bug :mysqlbug:`69232` - buf_dblwr->mutex can be splited into two
