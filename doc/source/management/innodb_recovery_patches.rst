.. _innodb_recovery_patches:

================================
 Fast |InnoDB| Recovery Process
================================

This feature implements several changes related to the recovery process (bug fixes and speed changes). The |InnoDB| plugin subsequently implemented similar functionality, so the feature is currently unused. The system variable it implemented now does nothing and has been kept to maintain compatibility.

  * ``recv_apply_hashed_log_recs()`` may hang up when meets DB_TABLESPACE_DELETED pages

  * Insert buffer operation may destroy the page during its recovery process adjustment of smaller sleep period in loop

  * ``buf_flush_insert_sorted_into_flush_list()`` change to don't sort (new variable ``innodb_fast_recovery`` (default false(~1.0.5), true(1.0.6~)) - if set true, sorting is enabled.)

Output statistics of recovery process after it ::

  -------------------
  RECOVERY STATISTICS
  -------------------
  Recovery time: 18 sec. (1 turns)
  
  Data page IO statistics
    Requested pages: 9126
    Read pages:      9126
    Written pages:   7957
    (Dirty blocks):  1156
    Grouping IO [times]:
          number of pages,
                  read request neighbors (in 32 pages chunk),
                          combined read IO,
                                  combined write IO
            1,    32,     335,    548
            2,    0,      121,    97
            3,    7,      49,     44
            4,    4,      43,     26
  ....
           64,    0,      2,      25
  
  Recovery process statistics
    Checked pages by doublewrite buffer: 128
    Overwritten pages from doublewrite:  0
    Recovered pages by io_thread:        9145
    Recovered pages by main thread:      0
    Parsed log records to apply:         2572491
              Sum of the length:         71274689
    Applied log records:                 2376356
              Sum of the length:         68098300
    Pages which are already new enough:  93
    Oldest page's LSN:                   926917970
    Newest page's LSN:                   1526578232


Other Information
=================

Is the ``buf_flush_insert_sorted_into_flush_list()`` change correct?
--------------------------------------------------------------------

|InnoDB| recovers the each pages not in order of the oldest un-flushed change``s :term:`LSN`. But flush_list must has un-flushed blocks in order of the :term:`LSN`. So, normal |InnoDB| does bubble sorting for each inserting block to ``flush_lsn`` … It is very expensive operation…

Then, why the order must be kept?
---------------------------------

|InnoDB| should get the LSN of the oldest un-flushed change in all blocks for checkpointing. It is the oldest un-flushed change's :term:`LSN` of the last block in the ``flush_list``.

So, it may be correct that the last blocks' ``oldest_modification`` only have to keep the oldest ``oldest_modification`` in the buffer pool.

This change is simple. If the new page's ``oldest_modification`` is... ::

  [newer than any oldest_modification in flushlist]

add to first of the ``flush_list`` ::

  [older than any oldest_modification in flushlist]

add to last of the ``flush_list`` ::

  [else]

overwrite ``oldest_modification`` by the oldest ``oldest_modification`` in ``flush_list``

add to last of the ``flush_list``

These operation should not break consistency of ``flush_list``. However, it may cause same-LSN-aged cluster of many pages and much flushing operation. But anyway, the most of the flushing should be done during the recovery process. ::

  --- innodb_plugin-1.0.3_orig/buf/buf0flu.c      2009-07-07 18:03:24.000000000 +0900
  +++ innodb_plugin-1.0.3_tmp/buf/buf0flu.c       2009-07-07 18:06:47.000000000 +0900
  @@ -108,6 +108,17 @@
          prev_b = NULL;
          b = UT_LIST_GET_FIRST(buf_pool->flush_list);
  
  +       if (b == NULL || b->oldest_modification < block->page.oldest_modification) {
  +               UT_LIST_ADD_FIRST(flush_list, buf_pool->flush_list, &block->page);
  +      } else {
  +               b = UT_LIST_GET_LAST(buf_pool->flush_list);
  +               if (b->oldest_modification < block->page.oldest_modification) {
  +                       /* align oldest_modification not to sort */
  +                       block->page.oldest_modification = b->oldest_modification;
  +               }
  +               UT_LIST_ADD_LAST(flush_list, buf_pool->flush_list, &block->page);
  +       }
  +/*
          while (b && b->oldest_modification > block->page.oldest_modification) {
                  ut_ad(b->in_flush_list);
                  prev_b = b;
  @@ -120,6 +131,7 @@
                  UT_LIST_INSERT_AFTER(flush_list, buf_pool->flush_list,
                                       prev_b, &block->page);
          }
  +*/
  
   #if defined UNIV_DEBUG || defined UNIV_BUF_DEBUG
          ut_a(buf_flush_validate_low());

System Variables
================

One new system variable was introduced by this feature.


.. variable:: innodb_fast_recovery

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: BOOL
     :default: FALSE
     :range: TRUE/FALSE
     :default: false(~1.0.5), true(1.0.6~)) - if set true, the change is enabled.

.. variable:: innodb_recovery_stats

     :cli: No
     :conf:
     :scope:
     :dyn:
     :vartype: BOOL
     :default: FALSE
     :range: TRUE/FALSE



Other reading
=============

  * `How to estimate time it takes InnoDB to recover? <http://www.mysqlperformanceblog.com/2007/05/09/how-to-estimate-time-it-takes-innodb-to-recover/>`_

  * `InnoDB recovery - is large buffer pool always better? <http://www.mysqlperformanceblog.com/2007/07/17/innodb-recovery-is-large-buffer-pool-always-better/>`_

  * `What is the longest part of InnoDB recovery process? <http://www.mysqlperformanceblog.com/2007/12/20/what-is-the-longest-part-of-innodb-recovery-process/>`_

  * `Improving InnoDB recovery time <http://www.mysqlperformanceblog.com/2009/07/07/improving-innodb-recovery-time/>`_

  * `How long is recovery from 8G innodb_log_file <http://www.mysqlperformanceblog.com/2010/12/22/how-long-is-recovery-from-8g-innodb_log_file/>`_
