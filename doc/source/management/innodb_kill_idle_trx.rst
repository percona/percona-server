.. _innodb_kill_idle_trx:

========================
 Kill Idle Transactions
========================

This feature limits the age of idle |XtraDB| transactions. If a transaction is idle for more seconds than the threshold specified, it will be killed. This prevents users from blocking purge by mistake.

System Variables
================

.. variable:: innodb_kill_idle_transaction
   
   :version 5.5.16-22.0: Introduced
   :scope: ``GLOBAL``
   :config: ``YES``
   :dyn: ``YES``
   :vartype: ``INTEGER``
   :default: 0 (disabled)
   :unit: Seconds

   To enable this feature, set this variable to the desired seconds wait until the transaction is killed.   

