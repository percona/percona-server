.. _innodb_fake_changes_page:

==========================
 Support for Fake Changes
==========================

.. note::

 This feature implementation is considered ALPHA quality.

Restarting a slave server in a replication environment or setting up new slave server can cause a replication reads slower. This is happening because replication in |MySQL| is single-threaded and because it needs to read the data before it can execute the queries. The process can be sped up by having prefetch threads to warm the server: replay statements and then rollback at commit.

That makes prefetch simple but has high overhead from locking rows only to undo changes at rollback.

Using this approach, support for *Fake Changes* have been implemented in order to remove the overhead and make it faster.

By reading the rows for ``INSERT``, ``UPDATE`` and ``DELETE`` statements but not updating them (*Fake Changes*), the rollback is very fast as in most cases there is nothing to do.

Caveats
=======

``DML`` operations **are supported**
------------------------------------

Currently only ``DML`` operations **are supported**, i.e. ``UPDATE``, ``INSERT``, ``REPLACE`` and ``DELETE`` (set deleted flag).

``DDL`` operations **are not supported**
----------------------------------------

``DDL`` operations **are not supported**, i.e. ``ALTER TABLE`` and ``TRUNCATE TABLE``. Fake Changes should be disabled temporally if ``DDL`` statements are going to be executed. Otherwise, data may be lost.

Explicit ``COMMIT`` will lead to an error
-----------------------------------------

From the viewpoint of transactional RDBMS, ``COMMIT`` should not be "fake" anytime. ``ROLLBACK`` must be used to terminate the fake transaction.

System Variables
================

.. variable:: innodb_fake_changes
   
   :version 5.6.11-60.3: Introduced
   :scope: ``GLOBAL``
   :type: ``BOOLEAN``
   :dyn: ``YES``
   :default: ``FALSE``

   This variable enables the *Fake Changes* feature.

.. variable:: innodb_locking_fake_changes

   :version 5.6.11-60.3: Introduced
   :scope: ``GLOBAL``
   :type: ``BOOLEAN``
   :dyn: ``YES``
   :default: ``TRUE``
 
   When this variable is set to ``FALSE``, it makes fake transactions not to take any row locks. This feature was implemented because, although fake change transactions downgrade the requested exclusive (X) row locks to shared (S) locks, these S locks prevent X locks from being taken and block the real changes. However, this option is not safe to set to FALSE by default, because the fake changes implementation is not ready for lock-less operation for all workloads. Namely, if a real transaction will remove a row that a fake transaction is doing a secondary index maintenance for, the latter will fail. This option is considered experimental and might be removed in the future if lockless operation mode fixes are implemented.

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
