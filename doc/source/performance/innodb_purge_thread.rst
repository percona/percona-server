.. _innodb_purge_thread:

========================
 Dedicated Purge Thread
========================

With |InnoDB|, data modified by a transaction is written to an undo space in the main tablespace, so that the system can provide read consistency. When a transaction is finished, the corresponding area in the undo space is freed. But if there are so many transactions that the purge thread cannot free space quickly enough, the main tablespace will grow dramatically. This will make performance decrease severely and will possibly consume all the available space on disk. This feature lets you use a dedicated purge thread so that the purge activity will be much quicker. And even if the overall performance will decrease when the purge thread is enabled, performance will be more stable which is often highly desirable.

Purge of the undo space is periodically done by the |InnoDB| main thread, along with other maintenance tasks. In most cases for an OLTP application, the transactions are small and short-running so the undo space can fit in memory in the buffer pool. The purge is then quick and efficient.

But there are several reasons that can make the undo space grow very large and go to disk:

  * long-running transactions

  * transactions with lots of changes

  * too many updates for the purge process to keep up

  * In all cases performance will drop dramatically. In standard |InnoDB| it is difficult to find an efficient solution for this problem.

You can now have one or several threads dedicated to the purge. This feature provides several benefits:

  * more control over the purge process

  * more stable performance (no more performance drops)

  * the |InnoDB| main thread does not need to take care of the purge anymore

But be aware that this feature comes at a cost: it reduces the overall performance because purging adds a non-negligible overhead. However we think it is better to have slightly worse but stable performance over time than to have better peak performance but unpredictable sharp drops.


System Variables
================

The following system variable was introduced by this feature:

.. variable:: innodb_use_purge_thread

   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: No
   :type: ULONG
   :default: 0(~1.0.5), 1(1.0.6~)
   :range: 0 - 32 (``UNIV_MAX_PARALLELISM``)

Using a value greater than 1 is experimental!

``UNIV_MAX_PARALLELISM`` is the maximum number of parallel threads in a parallelized operation

Other Information
=================

With ``SHOW INNODB STATUS`` you can monitor the number of unpurged transactions: look at History list length in the ``TRANSACTIONS`` section. This counter increases when a transaction commits and decreases when the purge process removes old versions of rows.

If this counter keeps increasing, it shows that the purge process cannot keep up, so you should use this option to create a dedicated purge thread.


Other Reading
=============

  * `Tuning for heavy writing workloads <http://www.mysqlperformanceblog.com/2009/10/14/tuning-for-heavy-writing-workloads/>`_

  * `Reasons for run-away main |InnoDB| tablespace <http://www.mysqlperformanceblog.com/2009/10/14/tuning-for-heavy-writing-workloads/>`_

  * `Purge thread spiral of death <http://www.mysqlperformanceblog.com/2009/10/14/tuning-for-heavy-writing-workloads/>`_
