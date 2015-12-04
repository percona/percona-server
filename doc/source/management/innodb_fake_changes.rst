.. _innodb_fake_changes_page:

==========================
 Support for Fake Changes
==========================

.. note:: 

  This feature implementation is considered *BETA* quality.

Unless multi-threaded slaves are used, replication is single threaded in nature, and it's important from the standpoint of performance to make sure that the queries executed by the replication thread or the events applied should be executed as fast as possible. A single event taking too long to apply is going to cause entire replication to stall, slowing down the rate at which replication catches up. This is especially painful when the slave server is restarted because with cold buffer pool individual events take far too long to complete. The slave is also generally I/O bound because of the difference of workload on master and the slave, and the biggest problem with single replication thread is that it has to read data to execute queries and most of the time is spent reading data then actually updating it.

Concept of Replication Prefetching
==================================

The process can be sped up by having prefetch threads to warm the server: replay statements and then rollback at commit. Prefetching works on a simple principle that if the data needed by the slave to apply events is already read then the application of events will be very fast as the data would already be cached. Replication is made up of two independent processes, an I/O thread that receives events from the master and writes to the relay log, and a SQL thread that reads the relay logs and applies those events. If the events in the relay log can be read in advance before the SQL thread reads them then the data that is needed by the SQL thread to apply the event would already be in the buffer pool and hence random disk I/O would be avoided, which would drastically improve the performance of SQL thread.

Prefetching with InnoDB Fake Changes
====================================

The way prefetching can be implemented without :ref:`innodb_fake_changes_page`, in most of the cases is by replaying the statements from the relay log but then manually converting all ``COMMITs`` to ``ROLLBACKs`` . This has the caveat of introducing the extra locking that is caused by the locks that are taken by the statements which are being replayed. The locks taken by statements executed by the process which is doing the prefetching can also cause lock contention with events that the SQL thread is trying to apply. Another issue with doing rollback is that, when a transaction changes data, old versions of the data are written to the undo log buffer. During the rollback phase |InnoDB| then has to read old versions of the data corresponding to what it was before the change from the undo log buffer and move it back to the |InnoDB| data page. In case of large transactions, or a transaction that updates a lot of data, the rollback can be costly and can generate significant amount of I/O.

Keeping in view the need of prefetching and the current caveats the :variable:`innodb_fake_changes` variable was implemented. The :variable:`innodb_fake_changes` variable enables an option for the server-side which allows for prefetching to work in a more performant manner. What enabling this option really does is that |InnoDB| reads the data needed by the ``DML`` queries but does not actually update the records, and hence no undo log record is generated, as nothing has changed, which means that rollback is instantaneous, and |InnoDB| doesn't have to do any additional work on rollback. However, the problem of locking contention is not completely solved, when the records are read, SHARED locks are taken on the records, so this can still cause contention with data changes that SQL thread needs to make. |Percona Server| does have a variable :variable:`innodb_locking_fake_changes` to make fake changes implementation completely lock-less. Because the fake changes implementation is not ready for lock-less operation for all workloads this variable is not safe to use and that is why it is disabled by default.

The :variable:`innodb_fake_changes` option, by enabling rollbacks on ``COMMITs``, enables prefetching tools to use it. It's by no way a tool that does prefetching of data. It merely provides a feature that is needed by prefetching tools to work in a performant manner. There is no prefetching that is transparently done by the slave when :variable:`innodb_fake_changes` is enabled, i.e., there is no change in slave behavior, there is no separate thread that is started to prefetch events. A separate utility is needed that runs with the session :variable:`innodb_fake_changes` variable enabled and that reads events from the relay log.

Caveats
=======

.. warning:: 

  This feature is only safe to use with an InnoDB-only server, because it is implemented in |InnoDB| only. Using it with any other storage engine, such as |MyISAM| or |TokuDB|,  will cause data inconsistencies because ``COMMITs`` will not be rolled back on those storage engines.

``DML`` operations **are supported**
------------------------------------

Currently only ``DML`` operations **are supported**, i.e. ``UPDATE``, ``INSERT``, ``REPLACE`` and ``DELETE`` (set deleted flag).

``DDL`` operations **are not supported**
----------------------------------------

``DDL`` operations **are not supported**, i.e. ``ALTER TABLE`` and ``TRUNCATE TABLE``. Running the ``DDL`` operations with :variable:`innodb_fake_changes` enabled would return an error and the subsequent ``DML`` operations may fail (from missing column etc.). 

Explicit ``COMMIT`` will lead to an error
-----------------------------------------

There are two types of transactions, implicit and explicit. Implicit transactions are ones that are created automatically by |InnoDB| to wrap around statements that are executed with autocommit enabled. For example, an ``UPDATE`` query that is not enclosed by ``START TRANSACTION`` and ``COMMIT``, when autocommit is enabled will be automatically treated as a single statement transaction. When |MySQL| writes events to the binary log, the events corresponding to the implicit transactions are automatically wrapped by ``BEGIN`` and ``COMMIT``.

When a session has the :variable:`innodb_fake_changes` option enabled, all the ``COMMITs`` will lead to an error, and nothing will be committed, this is in line with the implementation of :variable:`innodb_fake_changes` option, which guarantees that data is not left in an inconsistent state. Hence the option :variable:`innodb_fake_changes` would not be needed to be enabled at the ``GLOBAL`` level, rather the option :variable:`innodb_fake_changes` will only be enabled at the ``SESSION`` level by the utility that you would use to read and replay the relay logs. Enabling :variable:`innodb_fake_changes` only for the session that is used by the utility will ensure that the utility can safely execute DML queries without the actual data getting modified.

How to use InnoDB Fake Changes
==============================

A separate tool would be needed to read the relay log and replay the queries, the only purpose of :variable:`innodb_fake_changes` is to prevent actual data modifications. There are two different tools developed by Facebook that rely on :variable:`innodb_fake_changes` and can be used for the purpose of slave prefetching:

* One tool is built using python and is named `prefetch <http://bazaar.launchpad.net/~mysqlatfacebook/mysqlatfacebook/tools/files/head:/prefetch/>`_ . 
* Second tool is built in C and is named `faker <http://bazaar.launchpad.net/~mysqlatfacebook/mysqlatfacebook/tools/files/head:/faker/>`_. 

Both the tools rely on the |Percona Server| :variable:`innodb_fake_changes` option. 

Any other utility that can read the relay logs and replay them using multiple threads, would achieve what the above two tools achieve. Making sure that data is not modified by the tool would be done by enabling :variable:`innodb_fake_changes` option, but only on the ``SESSION`` level.

System Variables
================

.. variable:: innodb_fake_changes
   
   :version 5.6.11-60.3: Introduced
   :scope: Global, Session
   :type: Boolean
   :dyn: Yes
   :default: OFF

   This variable enables the :ref:`innodb_fake_changes_page` feature.

.. variable:: innodb_locking_fake_changes

   :version 5.6.11-60.3: Introduced
   :scope: Global, Session
   :type: Boolean
   :dyn: Yes
   :default: ON
 
   When this variable is set to ``OFF``, fake transactions will not take any row locks. This feature was implemented because, although fake change transactions downgrade the requested exclusive (X) row locks to shared (S) locks, these S locks prevent X locks from being taken and block the real changes. However, this option is not safe to set to ``OFF`` by default, because the fake changes implementation is not ready for lock-less operation for all workloads. Namely, if a real transaction will remove a row that a fake transaction is doing a secondary index maintenance for, the latter will fail. This option is considered experimental and might be removed in the future if lockless operation mode fixes are implemented.

Implementation Details
======================

  * The fake session is used as a prefetch of the replication, it  should not affect to later replication SQL execution.

  * The effective unit is each transaction. The behavior is decided at the start of the each one and never changed during the transaction

  * ``INSERT`` operations doesn't use the ``INSERT BUFFER``, it always causes the reading of the page actually for the option. ``DELETE`` also doesn't use the ``INSERT BUFFER``.

  * It never acquires ``X_LOCK`` from tables or records, only ``S_LOCK``.

  * The auto increment values behaves as usual.

  * It reserves free pages as usual.

  * Existed only ``root ~ leaf`` pages, which are accessed in the ``DML`` operation.

  * It will not prefetch allocate/free, split/merge, ``INODE``, ``XDES`` or other management pages. The same is for extern pages, i.e. large ``BLOB`` s).

  * Foreign key constraints are checked (for causing IO), but passed always.

Related Reading
===============

  * `on MySQL replication prefetching <http://dom.as/2011/12/03/replication-prefetching/>`_

  * `replication prefetching revisited <http://dom.as/2012/09/04/faker/>`_
