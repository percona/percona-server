.. rn:: 5.5.19-24.0

==============================
 |Percona Server| 5.5.19-24.0
==============================

Percona is glad to announce the release of |Percona Server| 5.5.19-24.0 on January 13th, 2012 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.19-24.0/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.5.19 <http://dev.mysql.com/doc/refman/5.5/en/news-5-5-19.html>`_, including all the bug fixes in it, |Percona Server| 5.5.19-24.0 is now the current stable release in the 5.5 series. All of |Percona| 's software is open-source and free, all the details of the release can be found in the `5.5.19-24.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.18-24.0>`_.


New Features
============

  * ``innodb_flush_neighbor_pages`` can now be set to none (0), legacy (1) or seq (2). The new option (2) is to flush pages that are only real neighbors.

  * Improvements to XtraDB's sync flush algorithm.

    * On the buffer flush list flushing (buf_flush_list): instead of
      actually starting the flush, first take the buffer flush list mutex
      and check the oldest buffer page LSN against the LSN we want to
      flush up to. If it is younger, just skip flushing.
    * On the buffer preflush to advance the oldest modified LSN
      (log_preflush_pool_modified_pages) on synchronous flushes: instead
      of issuing buf_flush_list once and then waiting for it to complete,
      flush in a loop with sleep until it fails to flush any more pages
      without any actual completion check (which would be
      buf_flush_wait_batch_end())
    * When the decision is made whether to issue sync flush and how much
      advance the oldest unflushed LSN, adjust the calculation to advance
      by exactly the amount required to reach the sync flush threshold
      age.
    * Add new debug-only server variable innodb_flush_checkpoint_debug
      with possible values 0 (default; does nothing) and 1 (log preflush
      and checkpointing do not happen). Adjust the sys_vars.all_vars test
      to ignore it. Adjust percona_server_variables_debug test.


Bug Fixes
=========

  * Test suite fixes: :bug:`849921` and :bug:`911237` (*Laurynas Biveinis*)
