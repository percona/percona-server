.. _xtradb_performance_improvements_for_io-bound_highly-concurrent_workloads:

=========================================================================
XtraDB Performance Improvements for I/O-Bound Highly-Concurrent Workloads
=========================================================================

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
a new system variable :variable:`innodb_empty_free_list_algorithm`.

.. variable:: innodb_empty_free_list_algorithm

   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :values: legacy, backoff
   :default: backoff

When ``legacy`` option is set, server will used the upstream algorithm and when
the ``backoff`` is selected, Percona implementation will be used.

.. _lru_manager_threads:

Multi-threaded LRU flusher
==========================

|Percona Server| :rn:`5.7.10-3` has introduced a true multi-threaded LRU
flushing. In this scheme, each buffer pool instance has its own dedicated LRU
manager thread that is tasked with performing LRU flushes and evictions to
refill the free list of that buffer pool instance. Existing multi-threaded
flusher no longer does any LRU flushing and is tasked with flush list flushing
only.

This has been done to address the shortcomings of the existing MySQL 5.7
multi-threaded flusher:

* All threads still synchronize on each coordinator thread iteration. If a
  particular flushing job is stuck on one of the worker threads, the rest will
  idle until the stuck one completes.
* The coordinator thread heuristics focus on flush list adaptive flushing
  without considering the state of free lists, which might be in need of urgent
  refill for a subset of buffer pool instances on a loaded server.
* LRU flushing is serialized with flush list flushing for each buffer pool
  instance, introducing the risk that the right flushing mode will not happen
  for a particular instance because it is being flushed in the other mode.

The following InnoDB metrics are no longer accounted, as their semantics do
not make sense under the current LRU flushing design:
``buffer_LRU_batch_flush_avg_time_slot``, ``buffer_LRU_batch_flush_avg_pass``,
``buffer_LRU_batch_flush_avg_time_thread``,
``buffer_LRU_batch_flush_avg_time_est``.

The need for InnoDB recovery thread writer threads is also removed,
consequently all associated code is deleted.

.. _parallel_doublewrite_buffer:

Parallel doublewrite buffer
===========================

The legacy doublewrite buffer is shared between all the buffer pool instances
and all the flusher threads. It collects all the page write requests into a
single buffer, and, when the buffer fills, writes it out to disk twice,
blocking any new write requests until the writes complete. This becomes a
bottleneck with increased flusher parallelism, limiting the effect of extra
cleaner threads. In addition, single page flushes, if they are performed, are
subject to above and also contend on the doublewrite mutex.

To address these issues |Percona Server| :rn:`5.7.11-4` has introduced private
doublewrite buffers for each buffer pool instance, for each batch flushing mode
(LRU or flush list). For example, with four buffer pool instances, there will
be eight doublewrite shards. Only one flusher thread can access any shard at a
time, and each shard is added to and flushed completely independently from the
rest. This does away with the mutex and the event wait does not block other
threads from proceeding anymore, it only waits for the asynchronous I/O to
complete. The only inter-thread synchronization is between the flusher thread
and I/O completion threads.

The new doublewrite buffer is contained in a new file, where all the shards are
contained, at different offsets. This file is created on startup, and removed
on a clean shutdown. If it's found on a crashed instance startup, its contents
are read and any torn pages are restored. If it's found on a clean instance
startup, the server startup is aborted with an error message.

The location of the doublewrite file is governed by a new
:variable:`innodb_parallel_doublewrite_path` global, read-only system variable.
It defaults to :file:`xb_doublewrite` in the data directory. The variable
accepts both absolute and relative paths. In the latter case they are treated
as relative to the data directory. The doublewrite file is not a tablespace
from InnoDB internals point of view.

The legacy InnoDB doublewrite buffer in the system tablespace continues to
address doublewrite needs of single page flushes, and they are free to use the
whole of that buffer (128 pages by default) instead of the last eight pages as
currently used. Note that single page flushes will not happen in |Percona
Server| unless :variable:`innodb_empty_free_list_algorithm` is set to
``legacy`` value.

The existing system tablespace is not touched in any way for this feature
implementation, ensuring that cleanly-shutdown instances may be freely moved
between different server flavors.

Interaction with :variable:`innodb_flush_method`
------------------------------------------------

Regardless of :variable:`innodb_flush_method` setting, the parallel doublewrite
file is opened with ``O_DIRECT`` flag to remove OS caching, then its access is
further governed by the exact value set: if it's set to ``O_DSYNC``, the
parallel doublewrite is opened with ``O_SYNC`` flag too. Further, if it's one
of ``O_DSYNC``, ``O_DIRECT_NO_FSYNC``, or ``ALL_O_DIRECT``, then the
doublewrite file is not flushed after a batch of writes to it is completed.
With other :variable:`innodb_flush_method` values the doublewrite buffer is
flushed only if setting ``O_DIRECT`` has failed.

.. variable:: innodb_parallel_doublewrite_path

   :cli: Yes
   :scope: Global
   :dyn: No
   :vartype: String
   :default: ``xb_doublewrite``

This variable is used to specify the location of the parallel doublewrite file.
It accepts both absolute and relative paths. In the latter case they are
treated as relative to the data directory.

|Percona Server| has introduced several options, only available in builds
compiled with ``UNIV_PERF_DEBUG`` C preprocessor define.

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

  * :rn:`5.7.11-4`

        * Implemented support for parallel doublewrite buffer

Other Reading
=============

* :ref:`page_cleaner_tuning`

* Bug :mysqlbug:`74637` - make dirty page flushing more adaptive

* Bug :mysqlbug:`67808` - in innodb engine, double write and multi-buffer pool
  instance reduce concurrency

* Bug :mysqlbug:`69232` - buf_dblwr->mutex can be splited into two
