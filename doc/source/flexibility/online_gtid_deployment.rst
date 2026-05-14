.. _online_gtid_deployment:

========================
 Online GTID deployment
========================

|Percona Server| now supports Online GTID deployment. This enables GTID to be deployed on existing replication setups without making the master read-only and stopping all the slaves. This feature was ported from the *Facebook* branch. Before this feature was implemented deploying the GTID replication on already existing replication setups required making a master :variable:`read_only`, shutting down all |MySQL| instances in the replica set simultaneously at the same position, enabling the :variable:`gtid_mode` variable in :file:`my.cnf`, and then starting all of the instances. 

With :variable:`gtid_deployment_step` enabled, a host cannot generate GTID values on its own, but if GTID logged events are received through replication stream from master, they will be logged.

Performing the online GTID deployment
=====================================

The online GTID deployment procedure can be done with the following steps:

1) replicas: :variable:`gtid_mode` = ``ON`` and :variable:`gtid_deployment_step` = ``ON``

   master: :variable:`gtid_mode` = ``OFF`` and :variable:`gtid_deployment_step` = ``OFF``

On each replica, one at a time, restart the |MySQL| server to enable :variable:`gtid_mode` and :variable:`gtid_deployment_step`. Afterward, we are in a state where every replica has :variable:`gtid_mode` set to ``ON`` and :variable:`gtid_deployment_step` set to ``ON``, but the master still has :variable:`gtid_mode` set to ``OFF`` and :variable:`gtid_deployment_step` set to ``OFF``. **NOTE:** in order to successfully restart the slaves :variable:`enforce-gtid-consistency` needs to be enabled as well.

2) new master: :variable:`gtid_mode` = ``ON`` and :variable:`gtid_deployment_step` = ``OFF``

   rest of the replicas: :variable:`gtid_mode` = ``ON`` and :variable:`gtid_deployment_step` = ``ON``

   old master: :variable:`gtid_mode` = ``OFF`` and :variable:`gtid_deployment_step` = ``OFF``

Perform a master promotion as normal, i.e. set the :variable:`gtid_mode` to ``ON`` and :variable:`gtid_deployment_step` to ``OFF``, re-pointing the replicas and original master to the new master. The original master's replication will intentionally break when started, since it still has the variable :variable:`gtid_mode` set to ``OFF`` and :variable:`gtid_deployment_step` to ``OFF``.

3) new master: :variable:`gtid_mode` = ``ON`` and :variable:`gtid_deployment_step` = ``OFF``
   
   rest of the replicas: :variable:`gtid_mode` = ``ON``

   old master: :variable:`gtid_mode` = ``ON``

Restart the original master to enable :variable:`gtid_mode`. It will now be able to replicate from the new master, and the entire replica set now has :variable:`gtid_mode` set to ``ON``. You can now set the :variable:`gtid_deployment_step` to ``OFF``. 

Version Specific Information
============================

  * :rn:`5.6.22-72.0`:
    Feature ported from the *Facebook* `branch <https://github.com/facebook/mysql-5.6>`_

System Variables
================

.. variable:: gtid_deployment_step

     :version 5.6.22-72.0: Introduced.
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :default: OFF

When this variable is enabled, a host cannot generate GTIDs on its own, but if GTID logged events are received through replication stream from the master, they will be logged.

The conditions for dynamic switching the :variable:`gtid_deployment_step` off are the same as for `read_only <http://dev.mysql.com/doc/refman/5.6/en/server-system-variables.html#sysvar_read_only>`_ variable:

 1) If you attempt to enable :variable:`gtid_deployment_step` while you have any explicit locks (acquired with ``LOCK TABLES``) or have a pending transaction, an error occurs.

 2) If you attempt to enable :variable:`gtid_deployment_step` while other clients hold explicit table locks or have pending transactions, the attempt blocks until the locks are released and the transactions end. While the attempt to enable :variable:`gtid_deployment_step` is pending, requests by other clients for table locks or to begin transactions also block until :variable:`gtid_deployment_step` has been set.

Related Reading
===============

 * `Lessons from Deploying MySQL GTID at Scale <https://www.facebook.com/notes/mysql-at-facebook/lessons-from-deploying-mysql-gtid-at-scale/10152252699590933>`_ 
 * |MySQL| `GTID documentation <https://dev.mysql.com/doc/refman/5.6/en/replication-gtids.html>`_

