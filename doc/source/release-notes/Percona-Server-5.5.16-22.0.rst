.. rn:: 5.5.16-22.0

==============================
 |Percona Server| 5.5.16-22.0
==============================

Percona is glad to announce the release of |Percona Server| 5.5.16-22.0 on October 14, 2011 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.16-22.0/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.5.16 <http://dev.mysql.com/doc/refman/5.5/en/news-5-5-16.html>`_, including all the bug fixes in it, |Percona Server| 5.5.16-22.0 is now the current stable release in the 5.5 series. All of |Percona| 's software is open-source and free, all the details of the release can be found in the `5.5.16-22.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.16-22.0>`_.


InnoDB Fake Changes
-------------------

When restarting a slave server in a replication environment, the process can be speed up by having prefetch threads to warm the server: replay statements and then rollback at commit.

That makes prefetch simple but has high overhead from locking rows only to undo changes at rollback.

Using this approach, support for Fake Changes have been implemented in order to remove the overhead and make it faster.

By reading the rows for "INSERT", "UPDATE" and "DELETE" statements but not updating them (Fake Changes), the rollback is very fast as in most cases there is nothing to do.

Kill Idle Transactions
----------------------

**NOTE:** Percona classes this feature as Beta and possibly not yet suited for production environments.

This feature limits the age of idle |XtraDB| transactions. If a transaction is idle for more seconds than the threshold specified, it will be killed. This prevents users from blocking purge by mistake.

Block Startup until LRU dump is loaded
--------------------------------------

Added a new boolean option, :variable:`innodb_blocking_buffer_pool_restore`, which is ``OFF`` by default. When set to ``ON``, restoring from the LRU dump file is synchronous, i.e. |XtraDB| waits until it is complete before reporting successful startup to the server. Bug Fixed: :bug:`785489` (*Alexey Kopytov*).

Behavior changes
----------------

The :ref:`Fast Index Creation Feature <innodb_fast_index_creation>` has been disabled by default to align the behavior with upstream. The boolean variable :variable:`innodb_expand_fast_index_creation` has been introduced for enabling or disabling this feature. Bug Fixed: :bug:`858945` (*Alexey Kopytov*).

Bug Fixes
=========

  * |XtraDB| requires a full table rebuild for foreign key changes. This unnecessarily delays their creation in a mysqldump output, so ``--innodb-optimize-keys`` should ignore foreign key constrains. Bug Fixed: :bug:`859078` (*Alexey Kopytov*).

  * After adding an index using the :ref:`Fast Index Creation Feature <innodb_fast_index_creation>`, statistics for that index provided by |XtraDB| were left in a bogus state until an explicit ``ANALYZE TABLE`` is executed. Bug Fixed: :bug:`857590` (*Alexey Kopytov*).

  * :variable:`QUERY_RESPONSE_TIME` did not respect :variable:`QUERY_RESPONSE_TIME_STATS`. Bug Fixed: :bug:`855312` (*Oleg Tsarev*).

  * The :command:`mysqldump` option ``--innodb-optimize-keys`` did not work correctly with tables where the first ``UNIQUE`` key on non-nullable columns was picked as the clustered index by |XtraDB| in the absence of a ``PRIMARY KEY``. Bug Fixed: :bug:`851674` (*Alexey Kopytov*).

  * The :ref:`Slow Query Log <slow_extended_55>` did not log the error number correctly. #830199 (Oleg Tsarev).

  * Variable :variable:`log-slow-admin-statements` was not listed with ``SHOW VARIABLES``. Bug Fixed: :bug:`830199` (*Oleg Tsarev*).

  * Fixed assertion failure in |XtraDB|. Bug Fixed: :bug:`814404` (*Yasufumi Kinoshita*).

  * Since ``AUTO_INCREMENT`` columns must be defined as keys, omitting key specifications and then adding them back in ``ALTER TABLE`` doesn't work for them. :command:`mysqldump --innodb-optimize-keys` has been fixed to take this into account. Bug Fixed: :bug:`812179` (*Alexey Kopytov*).

Other Changes
=============

Improvements and fixes on general distribution:
-----------------------------------------------

  *  :bug:`845019`, :bug:`702376`, :bug:`795747` (*Alexey Kopytov*, *Ignacio Nin*, *Yasufumi Kinoshita*).

Improvements and fixes on the |Percona Server| Test Suite:
----------------------------------------------------------

  * :bug:`760085`, :bug:`803140`, :bug:`803137`, :bug:`803120`, :bug:`803110`, :bug:`803100`, :bug:`803093`, :bug:`803088`, :bug:`803076`, :bug:`803071`, :bug:`794780`, :bug:`803072` (*Oleg Tsarev*, *Alexey Kopytov*, *Valentine Gostev*).
