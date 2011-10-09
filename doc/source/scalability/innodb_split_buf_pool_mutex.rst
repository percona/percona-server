.. _innodb_split_buf_pool_mutex:

==================================
 Improved Buffer Pool Scalability
==================================

The |InnoDB| buffer pool is a well known point of contention when many queries are executed concurrently. In |XtraDB|, the global mutex protecting the buffer pool has been split into several mutexes to decrease contention.

This feature splits the single global InnoDB buffer pool mutex into several mutexes:

.. list-table::
   :widths: 20 40
   :header-rows: 1

   * - Name
     - Protects
   * - buf_pool_mutex
     - flags about IO
   * - LRU_list_mutex
     - LRU list of blocks in buffer pool
   * - flush_list_mutex
     - flush list of dirty blocks to flush
   * - page_hash_latch	 
     - hash table to search blocks in buffer pool
   * - free_list_mutex	 
     - list of free blocks in buffer pool
   * - zip_free_mutex	 
     - lists of free area to treat compressed pages
   * - zip_hash_mutex	 
     - hash table to search compressed pages

The goal of this change is to reduce mutex contention, which can be very impacting when the working set does not fit in memory.

Other Information
=================

Detecting Mutex Contention
--------------------------

You can detect when you suffer from mutex contention in the buffer pool by reading the information provided in the SEMAPHORES section of the output of SHOW INNODB STATUS:

Under normal circumstances this section should look like this:

.. code-block:: guess

   SEMAPHORES
   ----------
   OS WAIT ARRAY INFO: reservation count 50238, signal count 17465
   Mutex spin waits 0, rounds 628280, OS waits 31338
   RW-shared spins 38074, OS waits 18900; RW-excl spins 0, OS waits 0

If you have a high-concurrency workload this section may look like this:

.. code-block:: guess

   1 ----------
   2 SEMAPHORES
   3 ----------
   4 OS WAIT ARRAY INFO: reservation count 36255, signal count 12675
   5 --Thread 10607472 has waited at buf/buf0rea.c line 420 for 0.00 seconds the semaphore:
   6 Mutex at 0x358068 created file buf/buf0buf.c line 597, lock var 0
   7 waiters flag 0
   8 --Thread 3488624 has waited at buf/buf0buf.c line 1177 for 0.00 seconds the semaphore:
   9 Mutex at 0x358068 created file buf/buf0buf.c line 597, lock var 0
   10 waiters flag 0
   11 --Thread 6896496 has waited at btr/btr0cur.c line 442 for 0.00 seconds the semaphore:
   12 S-lock on RW-latch at 0x8800244 created in file buf/buf0buf.c line 547
   13 a writer (thread id 14879600) has reserved it in mode  exclusive
   14 number of readers 0, waiters flag 1
   15 Last time read locked in file btr/btr0cur.c line 442
   16 Last time write locked in file buf/buf0buf.c line 1797
   [...]
   17 Mutex spin waits 0, rounds 452650, OS waits 22573
   18 RW-shared spins 27550, OS waits 13682; RW-excl spins 0, OS waits 0


Note that in the second case you will see indications that threads are waiting for a mutex created in the file buf/buf0buf.c (lines 5 to 7 or 8 to 10). Such an indication is a sign of buffer pool contention.
