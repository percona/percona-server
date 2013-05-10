.. _innodb_show_status:

======================
 Show |InnoDB| Status
======================

This feature reorganizes the output of ``SHOW INNODB STATUS`` for a better readability and prints the amount of memory used by the internal hash tables. In addition, new variables are available to control the output.

This feature modified the ``SHOW INNODB STATUS`` command as follows:

  * ``TRANSACTION`` section was moved to the end of the output, so that important information is not overlooked when the there is a large amount of it.

  * Added two variables to control ``SHOW INNODB STATUS`` information presented (bugfix for `#29123 <http://bugs.mysql.com/bug.php?id=29126>`_):

    * :variable:`innodb_show_verbose_locks` - Whether to show records locked

    * :variable:`innodb_show_locks_held` - Number of locks held to print for each |InnoDB| transaction

  * Added extended information about |InnoDB| internal hash table sizes (in bytes) in the ``BUFFER POOL AND MEMORY`` section; also added buffer pool size in bytes.

  * Added additional LOG section information (beginning in release 5.5.8-20.0).

Version Specific Information
============================

  * 5.5.8-20.0
    Added status variables showing information from ``SHOW INNODB STATUS``.

  * 5.5.8-20.0
    Added additional information in the LOG section.

  * 5.5.10-20.1:
    Renamed status variable :variable:`innodb_row_lock_numbers` to :variable:`innodb_current_row_locks`.

Other Information
=================

  * Author / Origin:
    Baron Schwartz, http://lists.mysql.com/internals/35174


System Variables
================

.. variable:: innodb_show_verbose_locks

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: ULONG
     :default: 0
     :range: 0 - 1

Specifies to show records locked in ``SHOW INNODB STATUS``. The default is 0, which means only the higher-level information about the lock (which table and index is locked, etc.) is printed. If set to 1, then traditional |InnoDB| behavior is enabled: the records that are locked are dumped to the output.

.. variable:: innodb_show_locks_held

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: ULONG
     :default: 10
     :range: 0 - 1000

Specifies the number of locks held to print for each |InnoDB| transaction in SHOW INNODB STATUS.


Status Variables
================

The status variables here contain information available in the output of ``SHOW INNODB STATUS``, organized by the sections ``SHOW INNODB STATUS`` displays. If you are familiar with the output of ``SHOW INNODB STATUS``, you will probably already recognize the information these variables contain.


BACKGROUND THREAD
-----------------

The following variables contain information in the BACKGROUND THREAD section of the output from ``SHOW INNODB STATUS``. An example of that output is:

 Insert an example of BACKGROUND THREAD section output here.

.. variable:: innodb_master_thread_1_second_loops

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_master_thread_10_second_loops

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_master_thread_background_loops

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_master_thread_main_flush_loops

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_master_thread_sleeps

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_background_log_sync

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

SEMAPHORES
----------

The following variables contain information in the SEMAPHORES section of the output from ``SHOW INNODB STATUS``. An example of that output is: ::

  ----------
  SEMAPHORES
  ----------
  OS WAIT ARRAY INFO: reservation count 9664, signal count 11182
  Mutex spin waits 20599, rounds 223821, OS waits 4479
  RW-shared spins 5155, OS waits 1678; RW-excl spins 5632, OS waits 2592
  Spin rounds per wait: 10.87 mutex, 15.01 RW-shared, 27.19 RW-excl

.. variable:: innodb_mutex_os_waits

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_mutex_spin_rounds

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_mutex_spin_waits

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_s_lock_os_waits

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_s_lock_spin_rounds

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_s_lock_spin_waits

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_x_lock_os_waits

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_x_lock_spin_rounds

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_x_lock_spin_waits

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

INSERT BUFFER AND ADAPTIVE HASH INDEX
-------------------------------------

The following variables contain information in the INSERT BUFFER AND ADAPTIVE HASH INDEX section of the output from SHOW |InnoDB| STATUS. An example of that output is: ::

  -------------------------------------
  INSERT BUFFER AND ADAPTIVE HASH INDEX
  -------------------------------------
  Ibuf: size 1, free list len 6089, seg size 6091,
  44497 inserts, 44497 merged recs, 8734 merges
  Hash table size 276707, node heap has 1 buffer(s)
  0.00 hash searches/s, 0.00 non-hash searches/s

.. variable:: innodb_ibuf_discarded_delete_marks

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_ibuf_discarded_deletes

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_ibuf_discarded_inserts

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_ibuf_free_list

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_ibuf_merged_delete_marks

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_ibuf_merged_deletes

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_ibuf_merged_inserts

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_ibuf_merges

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_ibuf_segment_size

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_ibuf_size

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_adaptive_hash_cells

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_adaptive_hash_heap_buffers

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_adaptive_hash_hash_searches

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_adaptive_hash_non_hash_searches

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

LOG
---

The following variables contain information in the LOG section of the output from ``SHOW INNODB STATUS``. An example of that output is: ::

  ---
  LOG
  ---
  Log sequence number 28219393219
  Log flushed up to 28219393219
  Last checkpoint at 28212583337
  Max checkpoint age 7782360
  Checkpoint age target 7539162
  Modified age 6809882
  Checkpoint age 6809882
  0 pending log writes, 0 pending chkp writes
  8570 log i/o's done, 2000.00 log i/o's/second

.. variable:: innodb_lsn_current

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_lsn_flushed

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_lsn_last_checkpoint

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_checkpoint_age

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_checkpoint_max_age

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_checkpoint_target_age

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

BUFFER POOL AND MEMORY
----------------------

The following variables contain information in the BUFFER POOL AND MEMORY section of the output from ``SHOW INNODB STATUS``. An example of that output is: ::

  ----------------------
  BUFFER POOL AND MEMORY
  ----------------------
  Total memory allocated 137625600; in additional pool allocated 0
  Internal hash tables (constant factor + variable factor)
      Adaptive hash index 3774352 (2213656 + 1560696)
      Page hash 139144
      Dictionary cache 629811 (554864 + 74947)
      File system 83536 (82672 + 864)
      Lock system 380792 (332872 + 47920)
      Recovery system 0 (0 + 0)
      Threads 84040 (82696 + 1344)
  Dictionary memory allocated 74947
  Buffer pool size 8192
  Buffer pool size, bytes 134217728
  Free buffers 0
  Database pages 8095
  Old database pages 2968
  Modified db pages 5914
  Pending reads 0
  Pending writes: LRU 0, flush list 129, single page 0
  Pages made young 372084, not young 0
  2546000.00 youngs/s, 0.00 non-youngs/s
  Pages read 103356, created 154787, written 979572
  469000.00 reads/s, 78000.00 creates/s, 138000.00 writes/s
  Buffer pool hit rate 994 / 1000, young-making rate 34 / 1000 not 0 / 1000
  Pages read ahead 0.00/s, evicted without access 15000.00/s

.. variable:: innodb_mem_adaptive_hash

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_mem_dictionary

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_mem_total

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_buffer_pool_pages_LRU_flushed

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_buffer_pool_pages_made_not_young

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_buffer_pool_pages_made_young

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_buffer_pool_pages_old

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

TRANSACTIONS
------------

The following variables contain information in the TRANSACTIONS section of the output from ``SHOW INNODB STATUS``. An example of that output is: ::

  ------------
  TRANSACTIONS
  ------------
  Trx id counter F561FD
  Purge done for trx's n:o < F561EB undo n:o < 0
  History list length 19
  LIST OF TRANSACTIONS FOR EACH SESSION:
  ---TRANSACTION 0, not started, process no 993, OS thread id 140213152634640
  mysql thread id 15933, query id 32109 localhost root
  show innodb status
  ---TRANSACTION F561FC, ACTIVE 29 sec, process no 993, OS thread id 140213152769808 updating or deleting
  mysql tables in use 1, locked 1

.. variable:: innodb_history_list_length

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_max_trx_id

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_oldest_view_low_limit_trx_id

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_purge_trx_id

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_purge_undo_no

     :version 5.5.8-20.0: Introduced.
     :vartype: Numeric
     :scope: Global

.. variable:: innodb_current_row_locks

     :version 5.5.8-20.0: Introduced.
     :version 5.5.10-20.1: Renamed.
     :vartype: Numeric
     :scope: Global

 This variable was named :variable:`innodb_row_lock_numbers` in release 5.5.8-20.0.


Other reading
=============

  * `SHOW INNODB STATUS walk through <http://www.mysqlperformanceblog.com/2006/07/17/show-innodb-status-walk-through/>`_

  * `Table locks in SHOW INNODB STATUS <http://www.mysqlperformanceblog.com/2010/06/08/table-locks-in-show-innodb-status/>`_
