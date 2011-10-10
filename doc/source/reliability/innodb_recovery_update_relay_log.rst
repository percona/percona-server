.. _innodb_recovery_update_relay_log_page:

=============================
 Crash-Resistant Replication
=============================

This feature makes replication much more reliable after a crash by making the replica's position relative to the master transactional.

|MySQL| replication normally stores its position in a file that is neither durable nor consistent. Thus, if the replica crashes, it can re-execute committed transactions. This usually causes replication to fail, potentially forcing the replica``s data to be re-initialized from the master or from a recent backup.

The improvement in |Percona Server| makes |InnoDB| store the replication position transactionally, and overwrite the usual master.info file upon recovery, so replication restarts from the correct position and does not try to re-execute committed transactions. This change greatly improves the durability of |MySQL| replication. It can be set to activate automatically, so replication “just works” and no intervention is necessary after a crash.


Restrictions
============

When :variable:`innodb_overwrite_relay_log_info` is enabled, you should only update |InnoDB| / |XtraDB| tables, not |MyISAM| tables or other storage engines.
You should not use relay or binary log filenames longer than 480 characters (normal: up to 512). If longer, the replication position information is not recorded in |InnoDB|.

Example Server Error Log Output
===============================

Upon crash recovery, the error log on a replica will show information similar to the following: ::

  InnoDB: Starting crash recovery.
  ....
  InnoDB: Apply batch completed
  InnoDB: In a MySQL replication slave the last master binlog file
  InnoDB: position 0 468, file name gauntlet3-bin.000015
  InnoDB: and relay log file
  InnoDB: position 0 617, file name ./gauntlet3-relay-bin.000111

If this feature is enabled, the output will look like the following, with additional lines prefixed with a ``+`` symbol: ::

  ....
  + InnoDB: Warning: innodb_overwrite_relay_log_info is enabled. Updates of other storage engines may have problem of consistency.
  + InnoDB: relay-log.info is detected.
  + InnoDB: relay log: position 429, file name ./gauntlet3-relay-bin.000111
  + InnoDB: master log: position 280, file name gauntlet3-bin.000015
  ....
    InnoDB: Starting crash recovery.
  ....
    InnoDB: Apply batch completed
  + InnoDB: In a MySQL replication slave the last master binlog file
  + InnoDB: position 0 468, file name gauntlet3-bin.000015
  + InnoDB: and relay log file
  + InnoDB: position 0 617, file name ./gauntlet3-relay-bin.000111
    090205 17:41:31 InnoDB Plugin 1.0.2-3 started; log sequence number 57933
  + InnoDB: relay-log.info have been overwritten.
  ....
    090205 17:41:31 [Note] Slave SQL thread initialized, starting replication in log ``gauntlet3-bin.000015`` at position 468, relay log ``./gauntlet3-relay-bin.000111`` position: 617

In this case, the master log position was overwritten to 468 from 280, so replication will start at position 468 and not repeat the transaction beginning at 280.

Version Specific Information
============================

  * 5.5.10-20.1:
    Renamed variable :variable:`innodb_overwrite_relay_log_info` to :variable:`innodb_recovery_update_relay_log`.

System Variables
================

One new system variable was introduced by this feature.

.. variable:: innodb_overwrite_relay_log_info

     :version 5.5.10-20.1: Renamed.
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: BOOLEAN
     :default: FALSE
     :range: TRUE/FALSE

If set to true, |InnoDB| overwrites ``relay-log.info`` at crash recovery when the information is different from the record in |InnoDB|.

 This variable was renamed to :variable:`innodb_recovery_update_relay_log`, beginning in release 5.5.10-20.1. It still exists as :variable:`innodb_overwrite_relay_log_info` in versions prior to that.

.. variable:: innodb_recovery_update_relay_log

     :version 5.5.10-20.1: Introduced.
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: BOOLEAN
     :default: FALSE
     :range: TRUE/FALSE

If set to true, |InnoDB| overwrites :file:`relay-log.info` at crash recovery when the information is different from the record in |InnoDB|.

 This variable was added in release 5.5.10-20.1. Prior to that, it was named :variable:`innodb_overwrite_relay_log_info`, which still exists in earlier versions.


Other Reading
=============

  * Another solution for |MySQL| 5.0 is `Google's transactional replication feature <http://code.google.com/p/google-mysql-tools/wiki/TransactionalReplication>`_, but it had some problems and bugs.

  * `Related bug (fixed and re-implemented in this feature) <http://bugs.|MySQL|.com/bug.php?id=34058>`_

  * `A blog post explaining how this feature makes replication more reliable <http://www.|MySQL|performanceblog.com/2009/03/04/making-replication-a-bit-more-reliable/>`_
