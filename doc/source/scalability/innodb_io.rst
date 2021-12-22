.. _innodb_io_page:

===================================
 Improved InnoDB I/O Scalability
===================================

Because InnoDB is a complex storage engine it must be configured properly in order to perform at its best. Some points are not configurable in standard InnoDB. The goal of this feature is to provide a more exhaustive set of options for XtraDB.

Version Specific Information
============================

  * :rn:`5.7.10-1`

    * Feature ported from |Percona Server| 5.6

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

This variable enables or disables the effect of the per-session value of
the `innodb_flush_log_at_trx_commit` variable.

If the global variable  `innodb_use_global_flush_log_at_trx_commit` is
set to ``1``, the session uses the current
global value of `innodb_flush_log_at_trx_commit`. This is the
upstream-compatible mode. If the user attempts to change the
`innodb_flush_log_at_trx_commit` value for a
session, the session value is ignored.

If the global variable `innodb_use_global_flush_log_at_trx_commit` is set to
``0``, a user can modify the
``innodb_flush_log_at_trx_commit`` per-session using the following command:

.. code-block:: text

    SET SESSION innodb_flush_log_at_trx_commit=0

This modification only affects the transactions in that session. Other sessions,
if they have not been individually modified, continue to use the
global `innodb_use_flush_log_at_trx_commit` value.

.. code-block:: mysql

  SET innodb_use_global_flush_log_at_trx_commit=1

.. variable:: innodb_flush_method

   :Version Info: - :rn:`5.7.10-3` - Ported from |Percona Server| 5.6
   :cli: Yes
   :conf: Yes
   :scope: Global
   :Dyn: No
   :vartype: Enumeration
   :default: ``fdatasync``
   :allowed: ``fdatasync``, ``O_DSYNC``, ``O_DIRECT``, ``O_DIRECT_NO_FSYNC``, ``ALL_O_DIRECT``

This is an existing MySQL 5.7 system variable that has a new allowed value ``ALL_O_DIRECT``. It determines the method InnoDB uses to flush its data and log files. (See :variable:`innodb_flush_method` in the MySQL 5.7 `Reference Manual <https://dev.mysql.com/doc/refman/5.7/en/innodb-parameters.html#sysvar_innodb_flush_method>`_).

The following values are allowed:

  * ``fdatasync``:
    use ``fsync()`` to flush data, log, and parallel doublewrite files.

  * ``O_SYNC``:
    use ``O_SYNC`` to open and flush the log and parallel doublewrite files; use ``fsync()`` to flush the data files. Do not use ``fsync()`` to flush the parallel doublewrite file.

  * ``O_DIRECT``:
    use O_DIRECT to open the data files and ``fsync()`` system call to flush data, log, and parallel doublewrite files.

  * ``O_DIRECT_NO_FSYNC``:
    use O_DIRECT to open the data files and parallel doublewrite files, but does not use the ``fsync()`` system call to flush the data files, log files, and parallel doublewrite files. This option isn't suitable for *XFS* file system.

  * ``ALL_O_DIRECT``: 
    use O_DIRECT to open data files, log files, and parallel doublewrite files
    and use ``fsync()`` to flush the data files but not the log files or 
    parallel doublewrite files. This option is recommended when |InnoDB| log files are big (more than 8GB), 
    otherwise, there may be performance degradation. **Note**: When using this option on *ext4* filesystem 
    variable :variable:`innodb_log_block_size` 
    should be set to 4096 (default log-block-size in *ext4*) in order to avoid the ``unaligned AIO/DIO`` warnings.

Status Variables
----------------

The following information has been added to ``SHOW ENGINE INNODB STATUS`` to confirm the checkpoint activity:

.. code-block:: bash

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

