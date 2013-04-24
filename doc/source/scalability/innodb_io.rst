.. _innodb_io_page:

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

  * :rn:`5.1.53-12.4`
  
    * Added variable :variable:`innodb_log_block_size`, method ``keep_average`` to :variable:`innodb_adaptive_checkpoint`.

  * :rn:`5.1.54-12.5`
  
    * Added value ``ALL_O_DIRECT`` to :variable:`innodb_flush_method`.


System Variables
================

.. variable:: innodb_adaptive_checkpoint

   :version 5.1.54-12.5: Added 
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :vartype: String
   :default: ``none`` (1.0.5), ``estimate`` (1.0.6)
   :values: ``none``, ``reflex``, ``estimate``, ``keep_average`` or 0/1/2/3 (for compatibility)

This variable controls the way adaptive checkpointing is performed. |InnoDB| constantly flushes dirty blocks from the buffer pool. Normally, the checkpoint is done passively at the current oldest page modification (this is called “fuzzy checkpointing”). When the checkpoint age nears the maximum checkpoint age (determined by the total length of all transaction log files), |InnoDB| tries to keep the checkpoint age away from the maximum by flushing many dirty blocks. But, if there are many updates per second and many blocks have almost the same modification age, the huge number of flushes can cause stalls.

Adaptive checkpointing forces a constant flushing activity at a rate of approximately ``[modified age / maximum checkpoint age]``. This can avoid or soften the impact of stalls casued by aggressive flushing.

The following values are allowed:

  * ``reflex``: 
    This behavior is similar to :variable:`innodb_max_dirty_pages_pct` flushing. The difference is that this method starts flushing blocks constantly and contiguously based on the oldest modified age. If the age exceeds 1/2 of the maximum age capacity, |InnoDB| starts weak contiguous flushing. If the age exceeds 3/4, |InnoDB| starts strong flushing. The strength can be adjusted by the |MySQL| variable innodb_io_capacity. In other words, we must tune :variable:`innodb_io_capacity` for the ``reflex`` method to work the best.

  * ``estimate``: 
    If the oldest modified age exceeds 1/2 of the maximum age capacity, |InnoDB| starts flushing blocks every second. The number of blocks flushed is determined by ``[number of modified blocks]``, ``[LSN progress speed]`` and ``[average age of all modified blocks]``. So, this behavior is independent of the innodb_io_capacity variable.

  * ``keep_average``: 
    This method attempts to keep the I/O rate constant by using a much shorter loop cycle (0.1 second) than that of the other methods (1.0 second). It is designed for use with SSD cards.


In some cases :variable:`innodb_adaptive_checkpoint` needs larger transaction log files (:variable:`innodb_adaptive_checkpoint` makes the limit of modified age lower). So, doubling the length of the transaction log files may be safe.


.. variable:: innodb_adaptive_flushing

     :cli: No
     :vartype: BOOL
     :default: TRUE
     :range: TRUE/FALSE

This is an existing |InnoDB| variable used to attempt flushing dirty pages in a way that avoids I/O bursts at checkpoints. In |XtraDB|, the default value of the variable is changed from that in |InnoDB|.

.. variable:: innodb_checkpoint_age_target

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: 0
     :range: 0+

This variable controls the maximum value of the checkpoint age if its value is different from 0. If the value is equal to 0, it has no effect.

It is not needed to shrink :variable:`innodb_log_file_size` to tune recovery time.

.. variable:: innodb_enable_unsafe_group_commit

     :version:  This variable is not needed after |XtraDB| 1.0.5.
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: 0
     :range: 0 - 1

This variable allows you to change the default behavior of |InnoDB| concerning the synchronization between the transaction logs and the binary logs at commit time. The following values are available:

  * 0 (default): 
    |InnoDB| keeps transactions in the same order between the transaction logs and the binary logs. This is the safer value but also the slower.

  * 1: 
    Transactions can be group-committed but the order between transactions will not be guaranteed to be kept anymore. Thus there is a slight risk of desynchronization between transaction logs and binary logs. However for servers that perform write-intensive workloads (and have RAID without BBU), you may expect a significant improvement in performance.


.. variable:: innodb_flush_log_at_trx_commit_session

     :version 5.1.49-rel11.3: Added
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: 3
     :range: 0-3

This variable implements a session-level version of the existing global variable :variable:`innodb_flush_log_at_trx_commit`. It allows a session to override the global setting when a different commit mode is required by the session.

The following values are available:

  * 0 / 1 / 2: 
    These values have the same meaning as for the global :variable:`innodb_flush_log_at_trx_commit`

  * 3 (default): 
    The session will ignore :variable:`innodb_flush_log_at_trx_commit_session` and stick to the global variable


.. variable:: innodb_flush_method

     :version 5.1.54-12.5: ``ALL_O_DIRECT`` option added
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: Enumeration
     :default: fdatasync
     :values: ``fdatasync``, ``O_DSYNC``, ``O_DIRECT``, ``ALL_O_DIRECT``

This is an existing |MySQL| 5.1 system variable. It determines the method |InnoDB| uses to flush its data and log files. (See innodb_flush_method in the |MySQL| 5.1 Reference Manual).

The following values are allowed:

  * ``fdatasync``: 
    Use ``fsync()`` to flush both the data and log files.

  * ``O_SYNC``: 
    Use ``O_SYNC`` to open and flush the log files; use ``fsync()`` to flush the data files.

  * ``O_DIRECT``: 
    Use ``O_DIRECT`` (or ``directio()`` on Solaris) to open the data files; use ``fsync()`` to flush both the data and log files.

  * ``ALL_O_DIRECT``: use ``O_DIRECT`` open and flush both the data and the log files. This option is recommended when |InnoDB| log files are big (more than 8GB), otherwise there might be even a performance degradation. **Note**: When using this option on *ext4* filesystem variable :variable:`innodb_log_block_size` should be set to 4096 (default log-block-size in *ext4*) in order to avoid the ``unaligned AIO/DIO`` warnings.


.. variable:: innodb_flush_neighbor_pages

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: 1
     :range: 0-1

This variable specifies whether, when the dirty pages are flushed to the data file, the neighbor pages in the data file are also flushed at the same time or not. The following values are available:

  * 0: 
    Disables the feature

  * 1 (default): 
    Enables the feature


If you use a storage which has no “head seek delay” (e.g. SSD or enough memory for write buffering), 0 may show better performance.

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

   :version 1.0.6-10: Introduced
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

.. variable:: innodb_read_ahead

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: String
     :default: ``linear``
     :values: ``none``, ``random`` (*), ``linear``, ``both``

This variable controls the read-ahead algorithm of |InnoDB|. The following values are available:

  * ``none``: 
    Disables read-ahead

  * ``random``:
    If enough pages within the same extent are in the buffer pool, |InnoDB| will automatically fetch the remaining pages (an extent consists of 64 consecutive pages)

  * ``linear`` (default): 
    If enough pages within the same extent are accessed sequentially, |InnoDB| will automatically fetch the remaining pages.

  * ``both``: 
    Enable both ``random`` and ``linear`` algorithms.


You can also control the threshold from which |InnoDB| will perform a read ahead request with the innodb_read_ahead_threshold variable

``random`` is removed from |InnoDB| Plugin 1.0.5, |XtraDB| ignores it after 1.0.5.


Status Variables
================

The following information has been added to ``SHOW INNODB STATUS`` to confirm the checkpointing activity:

  * The max checkpoint age

  * The current checkpoint age target

  * The current age of the oldest page modification which has not been flushed to disk yet.

  * The current age of the last checkpoint


::

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

