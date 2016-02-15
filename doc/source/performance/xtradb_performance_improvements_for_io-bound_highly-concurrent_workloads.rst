.. _xtradb_performance_improvements_for_io-bound_highly-concurrent_workloads:

===============================================================================
 XtraDB Performance Improvements for I/O-Bound Highly-Concurrent Workloads
===============================================================================

Priority refill for the buffer pool free list 
=============================================

In highly-concurrent I/O-bound workloads the following situation may happen: 

 1) Buffer pool free lists are used faster than they are refilled by the LRU cleaner thread.
 2) Buffer pool free lists become empty and more and more query and utility (i.e. purge) threads stall, checking whether a buffer pool free list has became non-empty, sleeping, performing single-page LRU flushes.
 3) The number of buffer pool free list mutex waiters increases.
 4) When the LRU manager thread (or a single page LRU flush by a query thread) finally produces a free page, it is starved from putting it on the buffer pool free list as it must acquire the buffer pool free list mutex too. However, being one thread in up to hundreds, the chances of a prompt acquisition are low.

To avoid this |Percona Server| has implemented priority refill for the buffer pool buffer pool free list. This implementation adjusts the buffer pool free list producer to always acquire the mutex with high priority and buffer pool free list consumer to always acquire the mutex with low priority. The implementation makes use of :ref:`thread priority lock framework <thread_priority>`.

Even the above implementation does not fully resolve the buffer pool free list mutex contention, as the mutex is still being acquired needlessly whenever the buffer pool free list is empty. This is addressed by delegating all the LRU flushes to the to the LRU manager thread, never attempting to evict a page or perform a LRU single page flush by a query thread, and introducing a backoff algorithm to reduce buffer pool free list mutex pressure on empty buffer pool free lists. This is controlled through a new system variable :variable:`innodb_empty_free_list_algorithm`.
 
.. variable:: innodb_empty_free_list_algorithm

   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :values: legacy, backoff
   :default: backoff

When ``legacy`` option is set, server will used the upstream algorithm and when the ``backoff`` is selected, |Percona| implementation will be used.

.. _lru_manager_threads:

Multi-threaded LRU flusher
==========================

|Percona Server| :rn:`5.7.10-3` has introduced a true multi-threaded LRU flushing. In this scheme, each buffer pool instance has its own dedicated LRU manager thread that is tasked with performing LRU flushes and evictions to refill the free list of that buffer pool instance. Existing multi-threaded flusher no longer does any LRU flushing and is tasked with flush list flushing only.

This has been done to address the shortcomings of the existing MySQL 5.7 multi-threaded flusher:

* All threads still synchronize on each coordinator thread iteration. If a particular flushing job is stuck on one of the worker threads, the rest will idle until the stuck one completes.
* The coordinator thread heuristics focus on flush list adaptive flushing without considering the state of free lists, which might be in need of urgent refill for a subset of buffer pool instances on a loaded server.
* LRU flushing is serialized with flush list flushing for each buffer pool instance, introducing the risk that the right flushing mode will not happen for a particular instance because it is being flushed in the other mode.

The following |InnoDB| metrics are no longer accounted, as their semantics do not make sense under the current LRU flushing design: ``buffer_LRU_batch_flush_avg_time_slot``, ``buffer_LRU_batch_flush_avg_pass``, ``buffer_LRU_batch_flush_avg_time_thread``, ``buffer_LRU_batch_flush_avg_time_est``.

The need for |InnoDB| recovery thread writer threads is also removed, consequently all associated code is deleted.

|Percona Server| has introduced several options, only available in builds compiled with ``UNIV_PERF_DEBUG`` C preprocessor define.

.. variable:: innodb_sched_priority_master
 
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :vartype: Boolean


Version Specific Information
============================

  * :rn:`5.7.10-1`

        * Feature partially ported from |Percona Server| 5.6

  * :rn:`5.7.10-3` 

        * Implemented support for multi-threaded LRU

Other Reading
=============
* :ref:`page_cleaner_tuning`

* Bug :mysqlbug:`74637` - make dirty page flushing more adaptive
