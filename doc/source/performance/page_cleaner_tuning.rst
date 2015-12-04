.. _page_cleaner_tuning:

============================
 Page cleaner thread tuning
============================

|Percona Server| has implemented page cleaner thread improvements in :rn:`5.6.13-61.0` release. 

Pseudo-parallel flushing
========================

Usage of the of multiple buffer pool instances is `not uniform <http://mikaelronstrom.blogspot.com/2010/09/multiple-buffer-pools-in-mysql-55.html>`_. The non-uniform buffer pool instance use means non-uniform free list depletion levels, which in turn causes transient stalls or single page LRU flushes for the query threads, increasing latency, and LRU/free list mutex contention. |Percona Server| has added heuristics that reduce the occurrence of depleted free lists and associated mutex contentions. Instead of issuing all the chunk-size LRU flush requests for the 1st instance, then for the 2nd instance, etc, the requests are issued to all instances in a pseudo-parallel manner: the 1st chunk for the 1st instance, the 1st chunk for the 2nd instance, etc., the 2nd chunk for the 1st instance, the 2nd chunk for the 2nd instance, etc. Moreover, if a particular instance has a nearly depleted free list (currently defined as <10% of (:variable:`innodb_lru_scan_depth`);, might be changed in the future), then the server keeps on issuing requests for that instance until it's not depleted anymore, or the flushing limit for it has been reached.

Furious Flushing
================

In certain situations it makes sense for InnoDB to flush pages as fast as possible instead of abiding to :variable:`innodb_io_capacity` setting and, starting with 5.6, :variable:`innodb_lru_scan_depth`. This is known as "furious flushing". Oracle |MySQL| 5.6 page cleaner flushes may perform furious flushing for the flush list up to :variable:`innodb_io_capacity_max` I/Os per second. We have extended this flush list flushing so that page cleaner thread sleep is skipped whenever the checkpoint age is in the sync preflush zone, allowing issuing more than :variable:`innodb_io_capacity_max` per second if needed.

For the LRU flushing, Oracle |MySQL| 5.6 does not have furious flushing, which may cause single page LRU flushes in the case of empty free lists. We have implemented the LRU list furious flushing by allowing the page cleaner thread to sleep less if the free lists are nearly depleted. The sleep time is determined as follows: if free list is filled less than 1% over all buffer pool instances: no sleep; less than 5%: 50ms shorter sleep time than the previous iteration; between 5% and 20%: no change; more than 20%: 50ms longer sleep time.

Timeouts
========

|Percona Server| has implemented time limits for LRU and flush list flushes in a single page cleaner thread iteration. The thread assumes that one iteration of its main loop (LRU and flush list flushes) complete under 1 second, but under heavy load we have observed iterations taking up to 40 seconds. Such situations confuse the heuristics, and an LRU or a flush taking a long time prevents the other kind of flush from running, which in turn may cause query threads to perform sync preflushes or single page LRU flushes depending on the starved flush type. If a LRU flush timeout happens, the current flushing pass over all buffer pool instances is still completed in order to ensure that all the instances have received at least a bit of flushing. In order to implement this for flush list flushes, the flush requests for each buffer pool instance were broken up to chunks too.

Adaptive Flushing Tuning
========================

With the tuned page cleaner heuristics, adaptive flushing may become too aggressive and maintain a consistently lower checkpoint age than a similarly-configured stock server. This results in a performance loss due to reduced write combining. To address this |Percona Server| has implemented new LSN age factor formula for page cleaner adaptive flushing.  that can be controlled with :variable:`innodb_cleaner_lsn_age_factor` variable. 

.. variable:: innodb_cleaner_lsn_age_factor

   :version 5.6.13-61.0: Introduced.
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :values: legacy, high_checkpoint
   :default: high_checkpoint

This variable is used to specify which algorithm should be used for page cleaner adaptive flushing. When ``legacy`` option is set, server will use the upstream algorithm and when the ``high_checkpoint`` is selected, |Percona| implementation will be used.

Tuning Variables
================

|Percona Server| has introduced several tuning options, only available in builds compiled with ``UNIV_PERF_DEBUG`` or ``UNIV_DEBUG`` C preprocessor define. These options are experimental, thus their name, allowed values, their semantics, and UNIV_PERF_DEBUG presence may change at any future release. Their default values are used for the corresponding variables in regular (that is, no ``UNIV_PERF_DEBUG`` defined) builds.

.. variable:: innodb_cleaner_max_lru_time 

   :version 5.6.13-61.0: Introduced.
   :default: 1000 (miliseconds)

This variable is used to specify the timeout for the LRU flush of one page cleaner thread iteration.

.. variable:: innodb_cleaner_max_flush_time 

   :version 5.6.13-61.0: Introduced.
   :default: 1000 (miliseconds)

This variable is used to specify the timeout for the flush list flush.

.. variable:: innodb_cleaner_lru_chunk_size 

   :version 5.6.13-61.0: Introduced.
   :default: 100

This variable replaces the hardcoded 100 constant as a chunk size for the LRU flushes.

.. variable:: innodb_cleaner_flush_chunk_size 

   :version 5.6.13-61.0: Introduced.
   :default: 100

This variable is used for specifying the chunk size for the flush list flushes.

.. variable:: innodb_cleaner_free_list_lwm 

   :version 5.6.13-61.0: Introduced.
   :default: 10
   :values: 0-100

This variable is used to specify the percentage of free list length below which LRU flushing will keep on iterating on the same buffer pool instance to prevent empty free list.

.. variable:: innodb_cleaner_eviction_factor

   :version 5.6.13-61.0: Introduced.
   :vartype: Boolean
   :values: ON/OFF
   :default: OFF

This variable is used for choosing between flushed and evicted page counts for LRU flushing heuristics. If enabled, makes LRU tail flushing to use evicted instead of flushed page counts for its heuristics.

Other reading
=============
* :ref:`xtradb_performance_improvements_for_io-bound_highly-concurrent_workloads`
* :mysqlbug:`68481` - InnoDB LRU flushing for MySQL 5.6 needs work
