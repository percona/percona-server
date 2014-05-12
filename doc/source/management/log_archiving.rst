.. _log_archiving:

==========================
 Log Archiving for XtraDB
==========================

.. note::

 This feature implementation is considered BETA quality.

|XtraDB| and |InnoDB| write to the redo log files in a cyclic manner, so that the oldest log data is overwritten with the newest one. This feature makes copies of the old log files before they are overwritten, thus saving all the redo log for a write workload.

When log archiving is enabled, it duplicates all redo log writes in a separate set of files in addition to normal redo log writing, creating new files as necessary.

Archived log file name format is ``ib_log_archive_<startlsn>``. The start LSN marks the log sequence number when the archive was started. An example of the archived log files should look like this: :: 

 ib_log_archive_00000000010145937920
 ib_log_archive_00000000010196267520

The oldest archived logs can be removed automatically by setting up the :variable:`innodb_log_arch_expire_sec` variable.

This feature can be used to create incremental backups with |Percona XtraBackup| as described in this :ref:`guide <xb21:xb_incremental_ps_56>`.

User statements for handling the XtraDB log archiving
======================================================

New statements have been introduced in |Percona Server| for handling the |XtraDB| log archiving. Both of these statements require ``SUPER`` privilege.

 * ``PURGE ARCHIVED LOGS BEFORE <datetime>`` - this will delete archived logs modified before date-time. Archive which is currently in progress will not be deleted.

 * ``PURGE ARCHIVED LOGS TO <log_filename>`` - this will delete all archived logs up to the 'log_filename' (including 'log_filename' too). Archive which is currently in progress will not be deleted.

Limitations
===========

When log archiving is enabled both redo and archived logs need to be written to disk, which can have some IO performance impact. It can also lead to more aggressive flushing because less space is available in the redo log.

Version Specific Information
============================

  * :rn:`5.6.11-60.3`:
    Feature implemented 

System Variables
================

.. variable:: innodb_log_archive

     :version 5.6.11-60.3: Introduced.
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Boolean
     :values: ON/OFF
     :default: OFF

This variable is used to enable or disable log archiving.

.. variable:: innodb_log_arch_dir

     :version 5.6.11-60.3: Introduced.
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :type: Text
     :default: ./

This variable is used to specify the log archiving directory.

.. variable:: innodb_log_arch_expire_sec

     :version 5.6.11-60.3: Introduced.
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :type: Numeric
     :default: 0 

Number of seconds since last modification after which archived log should be deleted.
