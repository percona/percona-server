.. rn:: 5.1.59-13.0

==============================
 |Percona Server| 5.1.59-13.0
==============================

Percona is glad to announce the release of |Percona Server| 5.1.59-13.0 on October 13, 2011 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.59-13.0/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.1.59 <http://dev.mysql.com/doc/refman/5.1/en/news-5-1-59.html>`_, including all the bug fixes in it, |Percona Server| 5.1.59-13.0 is now the current stable release in the 5.1 series. All of |Percona| 's software is open-source and free, all the details of the release can be found in the `5.1.59-13.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.1.59-13.0>`_.


New Features
============

InnoDB Fake Changes
-------------------

When restarting a slave server in a replication environment, the process can be speed up by having prefetch threads to warm the server: replay statements and then rollback at commit.

That makes prefetch simple but has high overhead from locking rows only to undo changes at rollback.

Using this approach, support for Fake Changes have been implemented in order to remove the overhead and make it faster.

By reading the rows for “INSERT“, “UPDATE“ and “DELETE“ statements but not updating them (Fake Changes), the rollback is very fast as in most cases there is nothing to do.

Kill Idle Transactions
----------------------

**NOTE:** Percona classes this feature as Beta and possibly not yet suited for production environments.

This feature limits the age of idle |XtraDB| transactions. If a transaction is idle for more seconds than the threshold specified, it will be killed. This prevents users from blocking purge by mistake.

Block Startup until LRU dump is loaded
--------------------------------------

Added a new boolean option, :variable:`innodb-blocking-lru-restore`, which is ``OFF`` by default. When set to ``ON``, restoring from the LRU dump file is synchronous, i.e. |XtraDB| waits until it is complete before reporting successful startup to the server. Bug Fixed: :bug:`785489` (*Alexey Kopytov*).

Behavior changes
----------------

The :ref:`Fast Index Creation Feature <innodb_fast_index_creation>` has been disabled by default to align the behavior with upstream. The boolean variable :variable:`innodb_expand_fast_index_creation` has been introduced for enabling or disabling this feature. Bug Fixed: :bug:`858945` (*Alexey Kopytov*).

Bug Fixes
=========

  * |XtraDB| requires a full table rebuild for foreign key changes. This unnecessarily delays their creation in a mysqldump output, so ``--innodb-optimize-keys`` should ignore foreign key constrains. Bug Fixed: :bug:`859078` (*Alexey Kopytov*).

  * After adding an index using the :ref:`Fast Index Creation Feature <innodb_fast_index_creation>`, statistics for that index provided by |XtraDB| were left in a bogus state until an explicit ``ANALYZE TABLE`` is executed. Bug Fixed: :bug:`857590` (*Alexey Kopytov*).

  * :variable:`QUERY_RESPONSE_TIME` did not respect :variable:`QUERY_RESPONSE_TIME_STATS`. Bug Fixed: :bug:`855312` (*Oleg Tsarev*).

  * The :command:`mysqldump` option ``--innodb-optimize-keys`` did not work correctly with tables where the first ``UNIQUE`` key on non-nullable columns was picked as the clustered index by |XtraDB| in the absence of a ``PRIMARY KEY``. Bug Fixed: :bug:`851674` (*Alexey Kopytov*).

  * Backported fix for `MySQL bug #53761 <http://bugs.mysql.com/bug.php?id=53761>`_ (Wrong estimate for ``RANGE`` query with compound indexes). Bug Fixed: :bug:`832528` (*Alexey Kopytov*).


  * Fixed assertion failure in |XtraDB|. Bug Fixed: :bug:`814404` (*Yasufumi Kinoshita*).

  * Since ``AUTO_INCREMENT`` columns must be defined as keys, omitting key specifications and then adding them back in ``ALTER TABLE`` doesn't work for them. :command:`mysqldump --innodb-optimize-keys` has been fixed to take this into account. Bug Fixed: :bug:`812179` (*Alexey Kopytov*).

Other Changes
=============

Improvements and fixes on general distribution:
-----------------------------------------------

  * :bug:`858467`, :bug:`845019`, (*Alexey Kopytov*).

Improvements and fixes on the |Percona Server| Test Suite:
----------------------------------------------------------

  *  :bug:`862378`, :bug:`862252`, :bug:`860416`, :bug:`838725`, :bug:`760085`, :bug:`870156`, :bug:`794790` (*Oleg Tsarev*, *Alexey Kopytov*, *Valentine Gostev*).
