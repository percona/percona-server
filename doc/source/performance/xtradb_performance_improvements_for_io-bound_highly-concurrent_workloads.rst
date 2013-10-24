.. _xtradb_performance_improvements_for_io-bound_highly-concurrent_workloads:

===============================================================================
 XtraDB Performance Improvements for I/O-Bound Highly-Concurrent Workloads
===============================================================================

In |Percona Server| :rn:`5.6.13-61.0` a number of |XtraDB| performance improvements have been implemented for high-concurrency scenarios.

Priority refill for the buffer pool free list 
=============================================

In highly-concurrent I/O-bound workloads the following situation may happen: 

 1) Buffer pool free lists are used faster than they are refilled by the LRU cleaner thread.
 2) Buffer pool free lists become empty and more and more query and utility (i.e. purge) thread stall, checking whether a free list has became non-empty, sleeping, performing single-page LRU flushes.
 3) The number of free list mutex waiters increases.
 4) When the page cleaner thread (or a single page LRU flush by a query thread) finally produces a free page, it is starved from putting it on the free list as it must acquire the free list mutex too. However, being one thread in up to hundreds, the chances of a prompt acquisition are low.

To avoid this |Percona Server| has implemented priority refill for the buffer pool free list in :rn:`5.6.13-61.0`. This implementation adjusts the free list producer to always acquire the mutex with high priority and free list consumer to always acquire the mutex with low priority. The implementation makes use of :ref:`thread priority lock framework <thread_priority>`.

Even the above implementation does not fully resolve the mutex contentions, as the free list mutex is still being acquired needlessly whenever the free list is empty. This was addressed by delegating all the LRU flushes to the page cleaner thread, never attempting to evict a page or perform a LRU single page flush by query thread, and introducing a backoff algorithm to reduce free list mutex pressure on empty free lists. This is controlled through a new system variable :variable:`innodb_empty_free_list_algorithm`.
 
.. variable:: innodb_empty_free_list_algorithm

   :version 5.6.13-61.0: Introduced.
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :values: legacy, backoff
   :default: backoff

When ``legacy`` option is set, server will used the upstream algorithm and when the ``backoff`` is selected, |Percona| implementation will be used.

Backoff for sync preflushes
===========================

Currently if a log-writing thread finds that the checkpoint age is in the sync preflush zone, it will attempt to advance the checkpoint itself by issuing a flush list flush batch unless one is running already. After the page cleaner tuning, in some cases, this feature hinders more than helps: the cleaner thread knows that the system is in sync preflush state and will perform furious flushing itself. The thread doing its own flushes will only contribute to mutex pressure and use CPU. In such cases it is better for the query threads to wait for any required flushes to complete instead. Whenever a query thread needs to perform a sync preflush to proceed, two options are now available:

 1) the query thread may issue a flush list batch itself and wait for it to complete. This is also used whenever the page cleaner thread is not running.
 2) alternatively the query thread may wait until the flush list flush is performed by the page cleaner thread. The wait is implemented using a tweaked exponential backoff: the thread sleeps for a random progressively-increasing time waiting for the flush list flush to happen. The sleep time counter is periodically reset to avoid runaway sleeps. This algorithm may change in the future. 

The behavior is controlled by a new system variable :variable:`innodb_foreground_preflush`.

.. variable:: innodb_foreground_preflush

   :version 5.6.13-61.0: Introduced.
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :values: sync_preflush, exponential_backoff
   :default: exponential_backoff

Relative Thread Scheduling Priorities for XtraDB
================================================

|Percona Server| has implemented Relative Thread Scheduling Priorities for |XtraDB| in :rn:`5.6.13-61.0`. This feature was implemented because whenever a high number of query threads is running on the server, the cleaner thread and other utility threads must receive more CPU time than a fair scheduling would allocate. New :variable:`innodb_sched_priority_cleaner` option has been introduced that corresponding to Linux ``nice`` values of ``-20..19``, where 0 is 19 (lowest priority) and 39 is -20 (highest priority). When new values are set server will attempt to set the thread nice priority for the specified thread type and return a warning with an actual priority if the attempt failed.

.. note:: 

   This feature implementation is Linux-specific.

.. variable:: innodb_sched_priority_cleaner

   :version 5.6.13-61.0: Introduced.
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :values: 1-39
   :default: 19

This variable is used to set a thread scheduling priority. Values correspond to  Linux ``nice`` values of ``-20..19``, where 0 is 19 (lowest priority) and 39 is -20 (highest priority).

|Percona Server| has introduced several options, only available in builds compiled with ``UNIV_PERF_DEBUG`` C preprocessor define.

.. variable:: innodb_sched_priority_purge

   :version 5.6.13-61.0: Introduced.
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :vartype: Boolean

.. variable:: innodb_sched_priority_io

   :version 5.6.13-61.0: Introduced.
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :vartype: Boolean

.. variable:: innodb_sched_priority_cleaner

   :version 5.6.13-61.0: Introduced.
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :vartype: Boolean

.. variable:: innodb_sched_priority_master
 
   :version 5.6.13-61.0: Introduced.
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :vartype: Boolean

.. _thread_priority:

Thread Priority Locks
=====================

The |InnoDB| worker threads compete for the shared resource accesses with the query threads. Performance experiments show that under high concurrency the worker threads must acquire the shared resources with priority. To this end, a priority mutex and a priority RW lock locking primitives have been implemented, that use the existing sync array code to wake up any high-priority waiting threads before any low-priority waiting threads, as well as reduce any low-priority thread spinning if any high-priority waiters are already present for a given sync object. The following mutexes have been converted to be priority mutexes: dict_sys, LRU list, free list, rseg, log_sys, and internal hash table sync object array mutexes. The following RW locks have been converted to priority RW locks: fsp, page_hash, AHI, index, and purge. To specify which threads are high-priority for shared resource acquisition, |Percona Server| has introduced several tuning options, only available in builds compiled with ``UNIV_PERF_DEBUG`` C preprocessor define.

.. variable:: innodb_priority_purge

   :version 5.6.13-61.0: Introduced.
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :vartype: Boolean

When this option is enabled purge coordinator and worker threads acquire shared resources with priority.

.. variable:: innodb_priority_io

   :version 5.6.13-61.0: Introduced.
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :vartype: Boolean

When this option is enabled I/O threads acquire shared resources with priority.

.. variable:: innodb_priority_cleaner

   :version 5.6.13-61.0: Introduced.
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :vartype: Boolean

When this option is enabled buffer pool cleaner thread acquire shared resources with priority.

.. variable:: innodb_priority_master
 
   :version 5.6.13-61.0: Introduced.
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :vartype: Boolean

When buffer pool cleaner thread acquire shared resources with priority.

.. note::

   These variables are intended for performance experimenting and not regular user tuning.

Other Reading
=============
* :ref:`page_cleaner_tuning`
