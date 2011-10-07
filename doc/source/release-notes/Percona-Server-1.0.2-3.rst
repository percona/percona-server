.. rn:: 1.0.2-3

==================================
 |Percona Server| 1.0.2-3 Sapporo
==================================

Released on March 2, 2009

  * Move to |MySQL| 5.1.31

  * Fixed :bug:`329290`: crash in insert/delete load.

  * Fixed :bug:`324734`: fails to build with |MySQL| using ``--with-debug``

  * More safe ``rw-lock``: fix :bug:`326445` - ``XtraDB hangs in 10 threads TPC-C bug``

  * Scalability fix -- replaced ``page_hash`` mutex to ``page_hash``, ``read-write`` lock

  * fixed :bug:`317074`: |InnoDB| compression hangs

  * Fix broken group commit]] in |InnoDB|, add new global variable to :ref:`innodb_io_page`: :variable:`enable_unsafe_group_commit`

  * Scalability fix -- ability to use :ref:`innodb_extra_rseg` 

  * Fixed :bug:`316995`: crash when attempt to select from ``information_schema`` table

  * Fixed :bug:`316449` compilation warning

  * Fixed :bug:`317584`: failed assertion

  * Fixed :bug:`316982`: compilation failed with ``UNIV_DEBUG`` enabled

  * Changed parameter for control read ahead activity in :ref:`innodb_io_page` - now it accepts string values

  * Fixed compiler warnings on ``buf_blockt``

  * Changed parameter name from ``innodb_ibuf_positive_contract`` to :ref:`innodb_io_page`: ``innodb_ibuf_active_contract``.

  * Added parameter to control merging insert buffer to :ref:`innodb_io_page`: :variable:`innodb_ibuf_accel_rate`

  * Added parameter to control neighbor flushing to :ref:`innodb_io_page`: :variable:`innodb_flush_neighbor_pages`

  * Added parameters to restrict insert buffer size to :ref:`innodb_io_page`: :variable:`innodb_ibuf_max_size`, :variable:`innodb_ibuf_active_contract`

  * Stop ``adaptive_checkpoint`` flushing when exceeds ``LOG_POOL_PREFLUSH_RATIO_ASYNC``
