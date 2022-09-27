.. _innodb_io_page:

===================================
 Improved InnoDB I/O Scalability
===================================

Because *InnoDB* is a complex storage engine it must be configured properly in
order to perform at its best. Some points are not configurable in standard
*InnoDB*. The goal of this feature is to provide a more exhaustive set of
options for *XtraDB*.

Version Specific Information
================================================================================

  * `8.0.12-1`: The feature was ported from *Percona Server for MySQL* 5.7.

System Variables
================================================================================

.. _innodb_flush_method:

.. rubric:: ``innodb_flush_method``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global
   * - Dynamic
     - No
   * - Data type
     - Enumeration
   * - Default
     - NULL
   * - Allowed values
     - ``fsync``, ``O_DSYNC``, ``O_DIRECT``, ``O_DIRECT_NO_FSYNC``, ``littlesync``, ``nosync``

The following values are allowed:

  * ``fdatasync``:
    use ``fsync()`` to flush data, log, and parallel doublewrite files.

  * ``O_SYNC``:
    use ``O_SYNC`` to open and flush the log and parallel doublewrite files; use ``fsync()`` to flush the data files. Do not use ``fsync()`` to flush the parallel doublewrite file.

  * ``O_DIRECT``:
    use O_DIRECT to open the data files and ``fsync()`` system call to flush data, log, and parallel doublewrite files.

  * ``O_DIRECT_NO_FSYNC``:
    use O_DIRECT to open the data files and parallel doublewrite files, but does not use the ``fsync()`` system call to flush the data files, log files, and parallel doublewrite files. Do not use this option for the *XFS* file system.

  * ``ALL_O_DIRECT``: 
    use O_DIRECT to open data files, log files, and parallel doublewrite files
    and use ``fsync()`` to flush the data files but not the log files or 
    parallel doublewrite files. This option is recommended when *InnoDB* log files are big (more than 8GB), otherwise, there may be performance degradation. **Note**: When using this option on *ext4* filesystem variable :ref:`innodb_log_block_size` should be set to 4096 (default log-block-size in *ext4*) in order to avoid the ``unaligned AIO/DIO`` warnings.


Starting from *Percona Server for MySQL* 8.0.20-11, the `innodb_flush_method <https://dev.mysql.com/doc/refman/8.0/en/innodb-parameters.html#sysvar_innodb_flush_method>`__ affects doublewrite buffers exactly the same as in *MySQL* 8.0.20. 
 
Status Variables
================================================================================

The following information has been added to ``SHOW ENGINE INNODB STATUS`` to confirm the checkpointing activity: 

.. code-block:: mysql

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

.. note:: 

        Implemented in *Percona Server for MySQL* 8.0.13-4, ``max checkpoint age`` has been
        removed because the information is identical to ``log capacity``.  
