.. _innodb_kill_idle_trx:

========================
 Kill Idle Transactions
========================

This feature limits the age of idle |XtraDB| transactions. If a transaction is idle for more seconds than the threshold specified, it will be killed. This prevents users from blocking purge by mistake.

Version Specific Information
============================

  * :rn:`5.7.10-1`:
        Feature ported from |Percona Server| 5.6

System Variables
================

.. variable:: innodb_kill_idle_transaction
   
   :scope: ``GLOBAL``
   :config: ``YES``
   :dyn: ``YES``
   :vartype: ``INTEGER``
   :default: 0 (disabled)
   :unit: Seconds

   To enable this feature, set this variable to the desired seconds wait until the transaction is killed.   

