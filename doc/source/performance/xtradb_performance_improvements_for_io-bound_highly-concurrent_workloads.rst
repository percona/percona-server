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

Other Reading
=============
* :ref:`page_cleaner_tuning`
