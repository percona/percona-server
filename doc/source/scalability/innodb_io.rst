.. _innodb_io_page:

===================================
 Improved |InnoDB| I/O Scalability
===================================

Because |InnoDB| is a complex storage engine it must be configured properly in
order to perform at its best. Some points are not configurable in standard
|InnoDB|. The goal of this feature is to provide a more exhaustive set of
options for |XtraDB|.

Version Specific Information
================================================================================

  * :rn:`8.0.12-1` - the feature was ported from |Percona Server| 5.7.

System Variables
================================================================================

.. variable:: innodb_flush_method

   :cli: Yes
   :conf: Yes
   :scope: Global
   :Dyn: No
   :vartype: Enumeration
   :default: ``fdatasync``
   :allowed: ``fdatasync``, ``O_DSYNC``, ``O_DIRECT``, ``O_DIRECT_NO_FSYNC``

See :variable:`innodb_flush_method` in the |MySQL| 8.0 `Reference Manual
<https://dev.mysql.com/doc/refman/8.0/en/innodb-parameters.html#sysvar_innodb_flush_method>`_).

This variable affects the parallel doublewrite buffer as follows

.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - Value
     - Usage
   * - fdatasync
     - Use ``fsync()`` to flush parallel doublewrite files.
   * - O_SYNC
     - Use ``O_SYNC`` to open and flush parallel doublewrite files; Do not use
       the ``fsync()`` system call to flush the parallel doublewrite file.
   * - O_DIRECT
     - Use O_DIRECT to open the data files and the ``fsync()`` system call to flush
       parallel doublewrite files.
   * - O_DIRECT_NO_FSYNC
     - Use ``O_DIRECT`` to open the data files but don't use ``fsync()`` system
       call to flush doublewrite files.

   
Status Variables
================================================================================

The following information has been added to ``SHOW ENGINE INNODB STATUS`` to confirm the checkpointing activity: 

.. code-block:: guess 

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


