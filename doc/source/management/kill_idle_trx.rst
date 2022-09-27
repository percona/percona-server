.. _kill_idle_trx:

======================
Kill Idle Transactions
======================

This feature limits the age of idle transactions, for all transactional storage
engines. If a transaction is idle for more seconds than the threshold
specified, it will be killed. This prevents users from blocking *InnoDB* purge
by mistake.

Version Specific Information
============================

  * `8.0.12-1`: The feature was ported from *Percona Server for MySQL* 5.7.

System Variables
================

.. _kill_idle_transaction:

.. rubric:: ``kill_idle_transaction``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Config file
     - Yes
   * - Scope
     - Global
   * - Dynamic
     - Yes
   * - Data type
     - Integer
   * - Default
     - 0 (disabled)
   * - Units
     - Seconds
   
If non-zero, any idle transaction will be killed after being idle for this many seconds.

