.. _innodb_io_55_page:

===================================
 Improved |InnoDB| I/O Scalability
===================================

.. default-domain:: psdom

|InnoDB| is a complex storage engine. It must be configured properly in order to perform at its best. Some points are not configurable in standard |InnoDB|, however. The goal of this feature is to provide a more exhaustive set of options for |XtraDB|. Note that some of these parameters are already available in the |InnoDB| plugin.

These new variables are divided into several categories:

  * Configuration of the capacity of the I/O subsystem (number of read and write threads, number of available I/O operations per second)

  * Additional options to control the flushing and checkpointing activities

  * Configuration of the insert buffer (maximum size, activity)

  * Various other options

Version Specific Information
============================

  * 5.5.19-24.0

    * Added option value ``cont`` for variable :variable:`innodb_flush_neighbor_pages`.

  * 5.5.8-20.0

    * Added variable :variable:`innodb_adaptive_flushing_method`.

    * Added variable :variable:`innodb_ibuf_active_merge`.

    * Added variable :variable:`innodb_ibuf_merge_rate`.

    * Added variable :variable:`innodb_use_global_flush_log_at_trx_commit`.
  
  * 5.5.20-beta
   
    * The 'reflex' value was removed from :variable:`innodb_adaptive_flushing_method` in 5.5.20-beta as a fix for bug :bug:`689450`.

System Variables
================

.. psdom:variable:: innodb_adaptive_flushing

   :cli: NOT AVAILABLE
   :vartype: BOOLEAN
   :default: TRUE


This is an existing |InnoDB| variable used to attempt flushing dirty pages in a way that avoids I/O bursts at checkpoints. In |XtraDB|, the default value of the variable is changed from that in |InnoDB|.

.. variable:: innodb_adaptive_flushing_method

   :version 5.5.8-20.0: Introduced
   :cli: YES
   :configfile: YES
   :scope: GLOBAL
   :dyn: YES
   :type: STRING
   :default: ``estimate``
   :allowed: ``native``, ``estimate``, ``keep_average`` (or 0/1/2, respectively, for compatibility)

This variable controls the way adaptive checkpointing is performed. |InnoDB| constantly flushes dirty blocks from the buffer pool. Normally, the checkpoint is done passively at the current oldest page modification (this is called “fuzzy checkpointing”). When the checkpoint age nears the maximum checkpoint age (determined by the total length of all transaction log files), |InnoDB| tries to keep the checkpoint age away from the maximum by flushing many dirty blocks. But, if there are many updates per second and many blocks have almost the same modification age, the huge number of flushes can cause stalls.

Adaptive checkpointing forces a constant flushing activity at a rate of approximately [modified age / maximum checkpoint age]. This can avoid or soften the impact of stalls casued by aggressive flushing.

The following values are allowed:

  * ``native`` [0]:
    This setting causes checkpointing to operate exactly as it does in native |InnoDB|.

  * ``estimate`` [1]: 
    If the oldest modified age exceeds 1/4 of the maximum age capacity, |InnoDB| starts flushing blocks every second. The number of blocks flushed is determined by [number of modified blocks], [LSN progress speed] and [average age of all modified blocks]. So, this behavior is independent of the ``innodb_io_capacity`` variable for the 1-second loop, but the variable still has an effect for the 10-second loop.

  * ``keep_average`` [2]:
    This method attempts to keep the I/O rate constant by using a much shorter loop cycle (0.1 second) than that of the other methods (1.0 second). It is designed for use with SSD cards.

  * ``reflex``:
    This behavior is similar to innodb_max_dirty_pages_pct flushing. The difference is that this method starts flushing blocks constantly and contiguously based on the oldest modified age. If the age exceeds 1/2 of the maximum age capacity, |InnoDB| starts weak contiguous flushing. If the age exceeds 3/4, |InnoDB| starts strong flushing. The strength can be adjusted by the |MySQL| variable :variable:`innodb_io_capacity`. In other words, we must tune ``innodb_io_capacity`` for the ``reflex`` method to work the best. This method was removed in :rn:`5.5.20-beta` as a fix for bug :bug:`689450`.


.. variable:: innodb_checkpoint_age_target

   :cli: Yes
   :conf: Yes
   :scope: GLOBAL
   :dyn: Yes
   :vartype: Numeric
   :default: 0
   :range: 0+

This variable controls the maximum value of the checkpoint age if its value is different from 0. If the value is equal to 0, it has no effect.

It is not needed to shrink ``innodb_log_file_size`` to tune recovery time.


.. variable:: innodb_flush_method

   :cli: Yes
   :conf: Yes
   :scope: Global
   :Dyn: No
   :vartype: Enumeration
   :default: ``fdatasync``
   :allowed: ``fdatasync``, ``O_DSYNC``, ``O_DIRECT``, ``ALL_O_DIRECT``

This is an existing |MySQL| 5.5 system variable. It determines the method |InnoDB| uses to flush its data and log files. (See ``innodb_flush_method`` in the |MySQL| 5.5 Reference Manual).

The following values are allowed:

  * ``fdatasync``: 
    use fsync() to flush both the data and log files.

  * ``O_SYNC``: 
    use O_SYNC to open and flush the log files; use fsync() to flush the data files.

  * ``O_DIRECT``: 
    use O_DIRECT to open the data files and fsync() system call to flush both the data and log files.

  * ``ALL_O_DIRECT``: 
    use O_DIRECT to open both data and log files, and use fsync() to flush the data files but it is skipped for all log files writes. This option is recommended when |InnoDB| log files are big (more than 8GB), otherwise there might be even a performance degradation. **Note**: When using this option on *ext4* filesystem variable :variable:`innodb_log_block_size` should be set to 4096 (default log-block-size in *ext4*) in order to avoid the ``unaligned AIO/DIO`` warnings.

.. variable:: innodb_flush_neighbor_pages

   :version 5.5.19-24.0: Introduced option value ``cont``
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :vartype: Enumeration
   :default: ``area``
   :range: ``none``, ``area``, ``cont``

This variable specifies whether, when the dirty pages are flushed to
the data file, the neighbor pages in the data file are also flushed at
the same time or not. The following values (and their numeric
counterparts ``0``, ``1`` and ``2`` for older patch compatibility) are
available:

  * ``none``: 
    disables the feature.

  * ``area`` (default): 
    enables flushing of non-contiguous neighbor pages. For each page
    that is about to be flushed, look into its vicinity for other
    dirty pages and flush them too. This value implements the standard
    |InnoDB| behavior. If you use a storage which has no “head seek
    delay” (e.g. SSD or enough memory for write buffering), ``none``
    or ``cont`` may show better performance. 

  * ``cont``:
    enable flushing of contiguous neighbor pages. For each page that
    is about to be flushed, look if there is a contiguous block of
    dirty pages surrounding it. If such block is found it is flushed
    in a sequential I/O operation as opposed to several random I/Os if
    ``area`` is used.

.. variable:: innodb_read_ahead

   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :vartype: String
   :default: ``linear``
   :allowed: ``none``, ``random`` (*), ``linear``, ``both``

This variable controls the read-ahead algorithm of |InnoDB|. The following values are available:

  * ``none``: 
    disables read-ahead

  * ``random``: 
    if enough pages within the same extent are in the buffer pool, |InnoDB| will automatically fetch the remaining pages (an extent consists of 64 consecutive pages)

  * ``linear`` (default): 
    if enough pages within the same extent are accessed sequentially, |InnoDB| will automatically fetch the remaining pages

  * ``both``: 
    enable both ``random`` and ``linear`` algorithms.

You can also control the threshold from which |InnoDB| will perform a read ahead request with the innodb_read_ahead_threshold variable

(*) ``random`` is removed from |InnoDB| Plugin 1.0.5, |XtraDB| ignores it after 1.0.5.

.. variable:: innodb_use_global_flush_log_at_trx_commit

   :version 5.5.8-20.0: Introduced
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

.. variable:: innodb_log_file_size

   :version 5.5.8-20.0: Introduced
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: No
   :type: Numeric
   :default: 5242880
   :range: 1048576 .. 4294967295

In upstream |MySQL| the limit for the combined size of log files must be less than 4GB. But in Percona Server it is:
  * on 32-bit systems: individual log file limit is 4 GB and total log file size limit is 4 GB, i.e. the same as in the upstream server.
  * on 64-bit systems: both individual log files and total log file size are practically unlimited (the limit is 2^63 - 1 bytes which is 8+ million TB).

.. variable:: innodb_purge_threads

   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: No
   :type: Numeric
   :default: 1

This variable is the same as the one in the upstream version. The only difference is the default value, in |Percona Server| it is ``1`` while in the upstream version is ``0``.

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
