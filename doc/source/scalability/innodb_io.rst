.. _innodb_io_page:

===================================
 Improved |InnoDB| I/O Scalability
===================================

Because |InnoDB| is a complex storage engine it must be configured properly in order to perform at its best. Some points are not configurable in standard |InnoDB|. The goal of this feature is to provide a more exhaustive set of options for |XtraDB|, like ability to change the log block size. 

Version Specific Information
============================

  * :rn:`5.6.11-60.3`

    * Feature ported from |Percona Server| 5.5

System Variables
================


.. variable:: innodb_use_global_flush_log_at_trx_commit

   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :type: Boolean
   :default: True
   :range: True/False

This variable is used to control the ability of the user to set the value of the global |MySQL| variable ``innodb_flush_log_at_trx_commit``.

If ``innodb_use_global_flush_log_at_trx_commit=0`` (False), the client can set the global |MySQL| variable, using: ::

  SET innodb_use_global_flush_log_at_trx_commit=N

If ``innodb_use_global_flush_log_at_trx_commit=1`` (True), the user session will use the current value of ``innodb_flush_log_at_trx_commit``, and the user cannot reset the value of the global variable using a ``SET`` command.

.. variable:: innodb_log_block_size

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: Numeric
     :default: 512
     :units: Bytes

This variable changes the size of transaction log records. The default size of 512 bytes is good in most situations. However, setting it to 4096 may be a good optimization with SSD cards. While settings other than 512 and 4096 are possible, as a practical matter these are really the only two that it makes sense to use. Clean restart and removal of the old logs is needed for the variable :variable:`innodb_log_block_size` to be changed.

Status Variables
----------------

The following information has been added to ``SHOW ENGINE INNODB STATUS`` to confirm the checkpointing activity: ::

  The max checkpoint age
  The current checkpoint age target
  The current age of the oldest page modification which has not been flushed to disk yet.
  The current age of the last checkpoint
  ...
  ---
  LOG
  ---
  Log sequence number 0 1059494372
  Log flushed up to   0 1059494372
  Last checkpoint at  0 1055251010
  Max checkpoint age  162361775
  Checkpoint age target 104630090
  Modified age        4092465
  Checkpoint age      4243362
  0 pending log writes, 0 pending chkp writes
  ...
