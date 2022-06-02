.. _innodb_kill_idle_trx:

======================
Kill Idle Transactions
======================

This feature limits the age of idle transactions, for all transactional storage
engines. If a transaction is idle for more seconds than the threshold
specified, it will be killed. This prevents users from blocking InnoDB purge
by mistake.

In *Percona Server for MySQL* :ref:`5.7.17-11` this feature has been re-implemented by
setting a connection socket read timeout value instead of periodically scanning
the internal InnoDB transaction list.

Version Specific Information
============================

  * :ref:`5.7.10-1`: Feature ported from *Percona Server for MySQL* 5.6
  * :ref:`5.7.17-11`: Feature re-implemented using socket timeouts

System Variables
================

.. _innodb_kill_idle_transaction:

.. rubric:: ``innodb_kill_idle_transaction``

:ref:`5.7.17-11` - Variable is now an alias of :ref:`kill_idle_transaction`.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Config file
     - ``YES``
   * - Scope
     - ``GLOBAL``
   * - Dynamic
     - ``YES``
   * - Data type
     - ``INTEGER``
   * - Default
     - 0 (disabled)
   * - Units
     - Seconds

To enable this feature, set this variable to the desired seconds wait until the transaction is killed. **NOTE:** This variable has been deprecated and it will be removed in a future major release.

.. _kill_idle_transaction:

.. rubric:: ``kill_idle_transaction``

Implemented in :ref:`5.7.17-11`.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Config file
     - ``YES``
   * - Scope
     - ``GLOBAL``
   * - Dynamic
     - ``YES``
   * - Data type
     - ``INTEGER``
   * - Default
     - 0 (disabled)
   * - Units
     - Seconds

If non-zero, any idle transaction will be killed after being idle for this many seconds.

