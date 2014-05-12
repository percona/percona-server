.. _innodb_io_page:

===================================
 Improved |InnoDB| I/O Scalability
===================================

Because |InnoDB| is a complex storage engine it must be configured properly in order to perform at its best. Some points are not configurable in standard |InnoDB|. The goal of this feature is to provide a more exhaustive set of options for |XtraDB|, like ability to change the log block size. 

Version Specific Information
============================

  * :rn:`5.6.11-60.3`

    * Feature ported from |Percona Server| 5.5
   
  * :rn:`5.6.14-62.0` 
    
    * New :variable:`innodb_log_checksum_algorithm` variable introduced. 

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

This variable changes the size of transaction log records. The default size of 512 bytes is good in most situations. However, setting it to 4096 may be a good optimization with SSD cards. While settings other than 512 and 4096 are possible, as a practical matter these are really the only two that it makes sense to use. Clean restart and removal of the old logs is needed for the variable :variable:`innodb_log_block_size` to be changed. **Note:** This feature implementation is considered BETA quality.

.. variable:: innodb_flush_method

   :Version Info: - :rn:`5.6.13-61.0` - Ported from |Percona Server| 5.5
   :cli: Yes
   :conf: Yes
   :scope: Global
   :Dyn: No
   :vartype: Enumeration
   :default: ``fdatasync``
   :allowed: ``fdatasync``, ``O_DSYNC``, ``O_DIRECT``, ``O_DIRECT_NO_FSYNC``, ``ALL_O_DIRECT``

This is an existing |MySQL| 5.6 system variable that has a new allowed value ``ALL_O_DIRECT``. It determines the method |InnoDB| uses to flush its data and log files. (See ``innodb_flush_method`` in the |MySQL| 5.6 `Reference Manual <https://dev.mysql.com/doc/refman/5.6/en/innodb-parameters.html#sysvar_innodb_flush_method>`_).

The following values are allowed:

  * ``fdatasync``: 
    use ``fsync()`` to flush both the data and log files.

  * ``O_SYNC``: 
    use O_SYNC to open and flush the log files; use ``fsync()`` to flush the data files.

  * ``O_DIRECT``: 
    use O_DIRECT to open the data files and fsync() system call to flush both the data and log files.

  * ``O_DIRECT_NO_FSYNC``:
    use O_DIRECT to open the data files but don't use ``fsync()`` system call to flush both the data and log files. This option isn't suitable for *XFS* file system.

  * ``ALL_O_DIRECT``: 
    use O_DIRECT to open both data and log files, and use ``fsync()`` to flush the data files but not the log files. This option is recommended when |InnoDB| log files are big (more than 8GB), otherwise there might be even a performance degradation. **Note**: When using this option on *ext4* filesystem variable :variable:`innodb_log_block_size` should be set to 4096 (default log-block-size in *ext4*) in order to avoid the ``unaligned AIO/DIO`` warnings.


.. variable:: innodb_log_checksum_algorithm

   :Version Info: - :rn:`5.6.14-62.0` - Variable introduced
   :cli: Yes
   :conf: Yes
   :scope: Global
   :Dyn: No
   :vartype: Enumeration
   :default: ``innodb``
   :allowed: ``none``, ``innodb``, ``crc32``, ``strict_none``, ``strict_innodb``, ``strict_crc32``

This variable is used to specify how log checksums are generated and verified. Behavior of :variable:`innodb_log_checksum_algorithm` depending on its value is mostly identical to :variable:`innodb_checksum_algorithm`, except that the former applies to log rather than page checksums. **NOTE**: this feature is currently considered experimental.

The following values are allowed:

  * ``none``:
    means that a constant value will be written to log blocks instead of calculated checksum values and no checksum validation will be performed on InnoDB/XtraBackup recovery, or changed page tracking (if enabled).

  * ``innodb``:
    (the default) means the default |InnoDB| behavior -- a custom and inefficient algorithm is used to calculate log checksums, but logs created with this option are compatible with upstream |MySQL| and earlier |Percona Server| or |Percona XtraBackup| versions that do not support other log checksum algorithms.

  * ``crc32``:
    will use CRC32 for log block checksums. Checksums will also benefit from hardware acceleration provided by recent Intel CPUs.

  * ``strict_*``:
    Normally, |XtraDB| or |Percona XtraBackup| will tolerate checksums created with other algorithms than is currently specified with the :variable:`innodb_log_checksum_algorithm` option. That is, if checksums don't match when reading the redo log on recovery, the block is considered corrupted only if no algorithm produces the value matching the checksum stored in the log block header. This can be disabled by prepending the value with the ``strict_`` suffix, e.g. ``strict_none``, ``strict_crc32`` or ``strict_innodb`` will only accept checksums created using the corresponding algorithms, but not the other ones.

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

Other Reading
=============

 * For Fusion-IO devices-specific tuning, see :ref:`atomic_fio` documentation.
