.. _kill_idle_trx:

======================
Kill Idle Transactions
======================

This feature limits the age of idle transactions, for all transactional storage
engines. If a transaction is idle for more seconds than the threshold
specified, it will be killed. This prevents users from blocking |InnoDB| purge
by mistake.

In |Percona Server| :rn:`5.7.17-11` this feature was re-implemented by
setting a connection socket read timeout value instead of periodically scanning
the internal |InnoDB| transaction list.

Version Specific Information
============================

  * :rn:`8.0.12-1`:
        Feature ported from |Percona Server| 5.7
  * :rn:`5.7.17-11`:
        Feature re-implemented using socket timeouts

System Variables
================

.. variable:: kill_idle_transaction

   :version 5.7.17-11: Variable implemented
   :scope: ``GLOBAL``
   :config: ``YES``
   :dyn: ``YES``
   :vartype: ``INTEGER``
   :default: 0 (disabled)
   :unit: Seconds

   If non-zero, any idle transaction will be killed after being idle for this
   many seconds.

