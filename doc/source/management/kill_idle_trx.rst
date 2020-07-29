.. _kill_idle_trx:

======================
Kill Idle Transactions
======================

This feature limits the age of idle transactions, for all transactional storage
engines. If a transaction is idle for more seconds than the threshold
specified, it will be killed. This prevents users from blocking |InnoDB| purge
by mistake.

Version Specific Information
============================

  * :rn:`8.0.12-1`:
        Feature ported from |Percona Server| 5.7

System Variables
================

.. variable:: kill_idle_transaction

   :scope: ``GLOBAL``
   :config: ``YES``
   :dyn: ``YES``
   :vartype: ``INTEGER``
   :default: 0 (disabled)
   :unit: Seconds

   If non-zero, any idle transaction will be killed after being idle for this
   many seconds.

