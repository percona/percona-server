.. _innodb_lru_dump_restore:

=================================
 Dump/Restore of the Buffer Pool
=================================

|Percona Server| can speed up restarts by saving and restoring the contents of the buffer pool, the largest memory buffer the |MySQL| server typically uses. Servers with large amounts of memory typically need a long time to warm up the buffer pool after a restart, so a server cannot be placed under production load for hours or even days. This special feature of |Percona Server| enables the buffer pool to be restored to its pre-shutdown state in a matter of minutes.

The feature works as follows. The buffer pool is a list of pages, usually 16kb in size, which are identified by an 8-byte number. The list is kept in least-recently-used order, which is why the buffer pool is sometimes referred to as an LRU list. The mechanism is to save the list of 8-byte page numbers just before shutdown, and after restart, to read the pages from disk and insert them back into the LRU at the correct position. The pages are sorted by ID to avoid random I/O, which is slower than sequential I/O on most disks. The LRU list is saved to the file ib_lru_dump in the directory specified by the datadir configuration setting, so you can back it up and restore it with the rest of your data easily.

Note that this feature does not store the contents of the buffer pool (i.e. it does not write 1GB of data to disk if you have a 1GB buffer pool). It stores only the identifiers of the pages in the buffer pool, which is a very small amount of data even for large buffer pools.

This feature can be used both manually and automatically. It is safe to enable automatically, and we have not found any performance regressions in it.

Automatic Operation
===================

To perform dump/restore of the buffer pool automatically, set the :variable:`innodb_lru_dump_restore` configuration variable. A non-zero value for this variable causes the server to create a new thread at startup. This thread's first task is to read and sort the saved file, and then restore the LRU accordingly.

After finishing the restore operation, the thread switches into dump mode, to periodically dump the LRU. The period is specified by the configuration variable's value in seconds. For example, if you set the variable to 60, then the thread saves the LRU list once per minute.


Manual Operation
================

Manual dump/restore is done through the ``INFORMATION_SCHEMA`` using the following two administrative commands (see ``XTRADB_ADMIN_COMMAND``):

  * ``XTRA_LRU_DUMP``: 
    Dumps the contents of the buffer pool (a list of space_id and page_no) to the file ib_lru_dump in the directory specified by the datadir configuration setting.

  * ``XTRA_LRU_RESTORE``:
    Restores pages based on the file ib_lru_dump.

Here is an example of how to manually save and restore the buffer pool. On a running server, examine the number of pages in the buffer pool, as in the following example: ::

  mysql> show status like ``innodb_buffer_pool_pages_data``;
  +-------------------------------+-------+
  | Variable_name                 | Value |
  +-------------------------------+-------+
  | innodb_buffer_pool_pages_data | 6231  |
  +-------------------------------+-------+

Save the contents of the LRU list to a file: ::

  mysql> select * from information_schema.XTRADB_ADMIN_COMMAND /*!XTRA_LRU_DUMP*/;
  +------------------------------+
  | result_message               |
  +------------------------------+
  | XTRA_LRU_DUMP was succeeded. |
  +------------------------------+
  1 row in set (0.02 sec)

This is a fast operation, and the resulting file is very small compared to the buffer pool. The file is in binary format, not text format. Now restart |MySQL|, and examine the number of pages in the buffer pool, for example, ::

  mysql> show status like ``innodb_buffer_pool_pages_data``;
  +-------------------------------+-------+
  | Variable_name                 | Value |
  +-------------------------------+-------+
  | innodb_buffer_pool_pages_data | 22    |
  +-------------------------------+-------+

The following command instructs |XtraDB| to restore the LRU from the file: ::

  mysql> select * from information_schema.XTRADB_ADMIN_COMMAND /*!XTRA_LRU_RESTORE*/;
  +---------------------------------+
  | result_message                  |
  +---------------------------------+
  | XTRA_LRU_RESTORE was succeeded. |
  +---------------------------------+
  1 row in set (0.62 sec)

This command executes quickly, because it doesn't ``use direct_io``. Afterwards, inspect the status of the buffer pool again: ::

  mysql> show status like ``innodb_buffer_pool_pages_data``;
  +-------------------------------+-------+
  | Variable_name                 | Value |
  +-------------------------------+-------+
  | innodb_buffer_pool_pages_data | 6231  |
  +-------------------------------+-------+

Status Information
==================

Status information about the dump and restore is written to the server``s error file: ::

  ....
  091217 11:49:16 InnoDB: administration command ``XTRA_LRU_DUMP`` was detected.
  ....
  091217 11:51:44 InnoDB: administration command ``XTRA_LRU_RESTORE`` was detected.
  091217 11:51:45 InnoDB: reading pages based on the dumped LRU list was done. (requested: 6231, read: 6209)

The requested number of pages is the number of pages that were in the LRU dump file. A page might not be read if it is already in the buffer pool, or for some other miscellaneous reasons, so the number of pages read can be less than the number requested.


Implementation Details
======================

The mechanism used to read pages into the LRU is the normal |InnoDB| calls for reading a page into the buffer pool. This means that it still performs all of the usual checks for data integrity. It also means that if you decrease the size of the buffer pool, |InnoDB| uses the usual page replacement and flushing algorithm to free pages when it becomes full.

The pages are sorted by tablespace, and then by ID within the tablespace.

The dump file is not deleted after loading, so you should delete it if you wish to disable the feature. For example, suppose you dump the LRU, and then some time later you decide to enable automatic dumping and reloading. You set the configuration variable and restart |MySQL|. Upon restart, the server will load the LRU to its state in the previously saved file, which might be very stale and not what you want to happen.


Version Specific Information
============================

  * 5.1.50-12.1:
    Automatic dump/restore implemented.
  * 5.5.10-20.1:
    Renamed variable :variable:`innodb_auto_lru_dump` to :variable:`innodb_buffer_pool_restore_at_startup`.

System Variables
================

.. variable:: innodb_auto_lru_dump

     :version 5.5.10-20.1: Renamed.
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: 0
     :range: 0-UINT_MAX32
     :unit: Seconds

This variable specifies the time in seconds between automatic buffer pool dumps. When set to zero, automatic dumps are disabled and must be done manually. When set to a non-zero value, an automatic restore of the buffer pool is also performed at startup, as described above.

 This variable was renamed to :variable:`innodb_buffer_pool_restore_at_startup`, beginning in release 5.5.10-20.1. It still exists as :variable:`innodb_auto_lru_dump` in versions prior to that.

.. variable:: innodb_buffer_pool_restore_at_startup

     :version 5.5.10-20.1: Added.
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: 0
     :range: 0-UINT_MAX32
     :unit:  Seconds

This variable specifies the time in seconds between automatic buffer pool dumps. When set to zero, automatic dumps are disabled and must be done manually. When set to a non-zero value, an automatic restore of the buffer pool is also performed at startup, as described above.

 This variable was added in release 5.5.10-20.1. Prior to that, it was named :variable:`innodb_auto_lru_dump`, which still exists in earlier versions.


Other reading
=============

  * `Save / restore buffer pool <http://www.|MySQL|performanceblog.com/2010/01/20/|XtraDB|-feature-save-restore-buffer-pool/>`_
