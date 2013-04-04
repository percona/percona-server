.. _binary_group_commit:

=========================
 Binary Log Group Commit
=========================

In cases when strict durability and recoverability is required and the storage that provides fast syncs is unavailable, setting up the variables :variable:`innodb_flush_log_at_trx_commit` =1 and :variable:`sync_binlog` =1 can result in big performance drop. 

.. note:: Variable :variable:`innodb_flush_log_at_trx_commit` makes sure that every transaction is written to the disk and that it can survive the server crash. In case the binary log is used for replication :variable:`sync_binlog` makes sure that every transaction written to the binary log matches the one executed in the storage engine. More information about these variables can be found in the |MySQL| `documentation <http://dev.mysql.com/doc/refman/5.5/en/innodb-parameters.html#sysvar_innodb_flush_log_at_trx_commit>`_.

Performance drop happening when these variables are enabled is caused by additional *fsync()* system calls on both binary and |XtraDB| ``REDO`` log when committing a transaction, that are needed to store the additional information on the disk. ``Binary Log Group Commit`` feature can use a single *fsync()* call to force data to the storage for multiple concurrently committing transactions, which provides throughput improvements in a write-concurrent workload.

Because there are no negative effects of this feature, it has been enabled by default and can't be disabled. Effects of this feature can be measured by the :variable:`binlog_commits` and :variable:`binlog_group_commits` status variables. The bigger the difference between these two variables the bigger is the performance gained with this feature.

Version Specific Information
============================

 * :rn:`5.5.18-23.0`
    Ported |MariaDB| Group commit for the binary log patch

Status Variables 
=================

.. variable:: binlog_commits

     :cli: Yes
     :scope: Session
     :vartype: Numeric

This variable shows the total number of transactions committed to the binary log.

.. variable:: binlog_group_commits

     :cli: Yes
     :scope: Session
     :vartype: Numeric

This variable shows the total number of group commits done to the binary log.

Other Reading
=============

 * `Testing the Group Commit Fix <http://www.mysqlperformanceblog.com/2011/07/13/testing-the-group-commit-fix/>`_

 * `Fixing MySQL group commit <http://kristiannielsen.livejournal.com/12254.html>`_
