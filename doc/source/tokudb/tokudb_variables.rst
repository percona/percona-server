.. _tokudb_variables:

================
TokuDB Variables
================

Like all storage engines, |TokuDB| has variables to tune performance and
control behavior. Fractal Tree algorithms are designed for near optimal
performance and TokuDB's default settings should work well in most situations,
eliminating the need for complex and time consuming tuning in most cases.

.. contents::
  :local:

TokuDB Server Variables
-----------------------

.. tabularcolumns:: |p{8.5cm}|p{1.5cm}|p{1.5cm}|p{2cm}|p{1.5cm}|

.. list-table::
   :header-rows: 1

   * - Name
     - Cmd-Line
     - Option File
     - Var Scope
     - Dynamic
   * - :variable:`tokudb_alter_print_error`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_analyze_delete_fraction`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_analyze_in_background`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_analyze_mode`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_analyze_throttle`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_analyze_time`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_auto_analyze`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_backup_allowed_prefix`
     - No
     - Yes
     - Global
     - No
   * - :variable:`tokudb_backup_dir`
     - No
     - Yes
     - Session
     - No
   * - :variable:`tokudb_backup_exclude`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_backup_last_error`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_backup_last_error_string`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_backup_plugin_version`
     - No
     - No
     - Global
     - No
   * - :variable:`tokudb_backup_throttle`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_backup_version`
     - No
     - No
     - Global
     - No
   * - :variable:`tokudb_block_size`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_bulk_fetch`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_cachetable_pool_threads`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`tokudb_cardinality_scale_percent`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`tokudb_check_jemalloc`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`tokudb_checkpoint_lock`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`tokudb_checkpoint_on_flush_logs`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`tokudb_checkpoint_pool_threads`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`tokudb_checkpointing_period`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`tokudb_cleaner_iterations`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`tokudb_cleaner_period`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`tokudb_client_pool_threads`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`tokudb_commit_sync`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_compress_buffers_before_eviction`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`tokudb_create_index_online`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_data_dir`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`tokudb_debug`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`tokudb_dir_per_db`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`tokudb_directio`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`tokudb_disable_hot_alter`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_disable_prefetching`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_disable_slow_alter`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_empty_scan`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_enable_fast_update`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_enable_fast_upsert`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_enable_partial_eviction`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`tokudb_fanout`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_fs_reserve_percent`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`tokudb_fsync_log_period`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`tokudb_hide_default_row_format`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_killed_time`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_last_lock_timeout`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_load_save_space`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_loader_memory_size`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_lock_timeout`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_lock_timeout_debug`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_log_dir`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`tokudb_max_lock_memory`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`tokudb_optimize_index_fraction`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_optimize_index_name`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_optimize_throttle`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_pk_insert_mode`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_prelock_empty`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_read_block_size`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_read_buf_size`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_read_status_frequency`
     - Yes
     - Yes
     - Global
     - Yes
   * - :variable:`tokudb_row_format`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_rpl_check_readonly`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_rpl_lookup_rows`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_rpl_lookup_rows_delay`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_rpl_unique_checks`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_rpl_unique_checks_delay`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_strip_frm_data`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`tokudb_support_xa`
     - Yes
     - Yes
     - Session, Global
     - Yes
   * - :variable:`tokudb_tmp_dir`
     - Yes
     - Yes
     - Global
     - No
   * - :variable:`tokudb_version`
     - No
     - No
     - Global
     - No
   * - :variable:`tokudb_write_status_frequency`
     - Yes
     - Yes
     - Global
     - Yes

.. variable:: tokudb_alter_print_error

  :cli: Yes
  :conf: Yes
  :scope: Global/Session
  :dyn: Yes
  :vartype: Boolean
  :default: OFF

When set to ``ON`` errors will be printed to the client during the ``ALTER
TABLE`` operations on |TokuDB| tables.

.. variable:: tokudb_analyze_delete_fraction

  :cli: Yes
  :conf: Yes
  :scope: Global/Session
  :dyn: Yes
  :vartype: Numeric
  :default: ``1.000000``
  :range: ``0.0`` - ``1.000000``

This variables controls whether or not deleted rows in the fractal tree are
reported to the client and to the |MySQL| error log during an ``ANALYZE TABLE``
operation on a |TokuDB| table. When set to ``1``, nothing is reported. When set
to ``0.1`` and at least 10% of the rows scanned by ``ANALYZE`` were deleted
rows that are not yet garbage collected, a report is returned to the client and
the |MySQL| error log.

.. variable:: tokudb_backup_allowed_prefix

  :cli: No
  :conf: Yes
  :scope: Global
  :dyn: No
  :vartype: String
  :default: NULL

This system-level variable restricts the location of the destination directory
where the backups can be located. Attempts to backup to a location outside of
the directory this variable points to or its children will result in an error.

The default is NULL, backups have no restricted locations. This read only
variable can be set in the :file:`my.cnf` configuration file and displayed with
the ``SHOW VARIABLES`` command when :ref:`toku_backup` plugin is loaded.

.. code-block:: mysql

  mysql> SHOW VARIABLES LIKE 'tokudb_backup_allowed_prefix';
  +------------------------------+-----------+
  | Variable_name                | Value     |
  +------------------------------+-----------+
  | tokudb_backup_allowed_prefix | /dumpdir  |
  +------------------------------+-----------+

.. variable:: tokudb_backup_dir

  :cli: No
  :conf: No
  :scope: Session
  :dyn: Yes
  :vartype: String
  :default: NULL

When enabled, this session level variable serves two purposes, to point to the
destination directory where the backups will be dumped and to kick off the
backup as soon as it is set. For more information see :ref:`toku_backup`.

.. variable:: tokudb_backup_exclude

  :cli: No
  :conf: No
  :scope: Session
  :dyn: Yes
  :vartype: String
  :default: ``(mysqld_safe\.pid)+``

Use this variable to set a regular expression that defines source files
excluded from backup. For example, to exclude all :file:`lost+found`
directories, use the following command:

.. code-block:: mysql

  mysql> set tokudb_backup_exclude='/lost\\+found($|/)';

For more information see :ref:`toku_backup`.

.. variable:: tokudb_backup_last_error

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Numeric
  :default: 0

This session variable will contain the error number from the last backup.
``0`` indicates success. For more information see :ref:`toku_backup`.

.. variable:: tokudb_backup_last_error_string

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: String
  :default: NULL

This session variable will contain the error string from the last backup. For
more information see :ref:`toku_backup`.

.. variable:: tokudb_backup_plugin_version

  :cli: No
  :conf: No
  :scope: Global
  :dyn: No
  :vartype: String

This read-only server variable documents the version of the |TokuBackup|
plugin. For more information see :ref:`toku_backup`.

.. variable:: tokudb_backup_throttle

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Numeric
  :default: 18446744073709551615

This variable specifies the maximum number of bytes per second the copier of a
hot backup process will consume. Lowering its value will cause the hot backup
operation to take more time but consume less I/O on the server. The default
value is ``18446744073709551615`` which means no throttling. For more
information see :ref:`toku_backup`.

.. variable:: tokudb_backup_version

  :cli: No
  :conf: No
  :scope: Global
  :dyn: No
  :vartype: String

This read-only server variable documents the version of the hot backup library.
For more information see :ref:`toku_backup`.

.. variable:: tokudb_block_size

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Numeric
  :default: 512 MB
  :range: 4096 - 4294967295

This variable controls the maximum size of node in memory before messages
must be flushed or node must be split.

Changing the value of :variable:`tokudb_block_size` only affects subsequently
created tables and indexes. The value of this variable cannot be changed for an
existing table/index without a dump and reload.

.. variable:: tokudb_bulk_fetch

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Boolean
  :default: ON

This variable determines if our bulk fetch algorithm is used for ``SELECT``
statements. ``SELECT`` statements include pure ``SELECT ...`` statements, as
well as ``INSERT INTO table-name ... SELECT ...``, ``CREATE TABLE table-name
... SELECT ...``, ``REPLACE INTO table-name ... SELECT ...``, ``INSERT IGNORE
INTO table-name ... SELECT ...``, and ``INSERT INTO table-name ... SELECT ...
ON DUPLICATE KEY UPDATE``.

.. variable:: tokudb_cache_size

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: No
  :vartype: Numeric

This variable configures the size in bytes of the |TokuDB| cache table. The
default cache table size is 1/2 of physical memory. Percona highly recommends
using the default setting if using buffered I/O, if using direct I/O then
consider setting this parameter to 80% of available memory.

Consider decreasing :variable:`tokudb_cache_size` if excessive swapping is
causing performance problems. Swapping may occur when running multiple |MySQL|
server instances or if other running applications use large amounts of physical
memory.

.. variable:: tokudb_cachetable_pool_threads

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: No
  :vartype: Numeric
  :range: 0 - 1024
  :default: 0

This variable defines the number of threads for the cachetable worker thread
pool. This pool is used to perform node prefetches, and to serialize, compress,
and write nodes during cachetable eviction. The default value of 0 calculates
the pool size to be num_cpu_threads * 2.

.. variable:: tokudb_check_jemalloc

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: No
  :vartype: Boolean
  :default: OFF

This variable enables/disables startup checking if jemalloc is linked and
correct version and that transparent huge pages are disabled. Used for
testing only.

.. variable:: tokudb_checkpoint_lock

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Boolean
  :default: OFF

Disables checkpointing when true. Session variable but acts like a global, any
session disabling checkpointing disables it globally. If a session sets this
lock and disconnects or terminates for any reason, the lock will not be
released. Special purpose only, do **not** use this in your application.

.. variable:: tokudb_checkpoint_on_flush_logs

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: Yes
  :vartype: Boolean
  :default: OFF

When enabled forces a checkpoint if we get a flush logs command from the
server.

.. variable:: tokudb_checkpoint_pool_threads

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: No
  :vartype: Numeric
  :range: 0 - 1024
  :default: 0

This defines the number of threads for the checkpoint worker thread pool. This
pool is used to serialize, compress and write nodes cloned during checkpoint.
Default of ``0`` uses old algorithm to set pool size to ``num_cpu_threads/4``.

.. variable:: tokudb_checkpointing_period

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: Yes
  :vartype: Numeric
  :range: 0 - 4294967295
  :default: 60

This variable specifies the time in seconds between the beginning of one
checkpoint and the beginning of the next. The default time between |TokuDB|
checkpoints is 60 seconds. We recommend leaving this variable unchanged.

.. variable:: tokudb_cleaner_iterations

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: Yes
  :vartype: Numeric
  :range: 0 - 18446744073709551615
  :default: 5

This variable specifies how many internal nodes get processed in each
:variable:`tokudb_cleaner_period` period. The default value is ``5``. Setting
this variable to ``0`` turns off cleaner threads.

.. variable:: tokudb_cleaner_period

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: Yes
  :vartype: Numeric
  :range: 0 - 18446744073709551615
  :default: 1

This variable specifies how often in seconds the cleaner thread runs. The
default value is ``1``. Setting this variable to ``0`` turns off cleaner
threads.

.. variable:: tokudb_client_pool_threads

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: No
  :vartype: Numeric
  :range: 0 - 1024
  :default: 0

This variable defines the number of threads for the client operations thread
pool. This pool is used to perform node maintenance on over/undersized nodes
such as message flushing down the tree, node splits, and node merges. Default
of ``0`` uses old algorithm to set pool size to ``1 * num_cpu_threads``.

.. variable:: tokudb_commit_sync

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Boolean
  :default: ON

Session variable :variable:`tokudb_commit_sync` controls whether or not the
transaction log is flushed when a transaction commits. The default behavior is
that the transaction log is flushed by the commit. Flushing the transaction log
requires a disk write and may adversely affect the performance of your
application.

To disable synchronous flushing of the transaction log, disable the
:variable:`tokudb_commit_sync` session variable as follows:

.. code-block:: mysql

  SET tokudb_commit_sync=OFF;

Disabling this variable may make the system run faster. However, transactions
committed since the last checkpoint are not guaranteed to survive a crash.

.. warning::

  By disabling this variable and/or setting the
  :variable:`tokudb_fsync_log_period` to non-zero value you have effectively
  downgraded the durability of the storage engine. If you were to have a crash
  in this same window, you would lose data. The same issue would also appear if
  you were using some kind of volume snapshot for backups.

.. variable:: tokudb_compress_buffers_before_eviction

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: No
  :vartype: Boolean
  :default: ON

When this variable is enabled it allows the evictor to compress unused
internal node partitions in order to reduce memory requirements as a first step
of partial eviction before fully evicting the partition and eventually the
entire node.

.. variable:: tokudb_create_index_online

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Boolean
  :default: ON

This variable controls whether indexes created with the ``CREATE INDEX``
command are hot (if enabled), or offline (if disabled). Hot index creation
means that the table is available for inserts and queries while the index is
being created. Offline index creation means that the table is not available for
inserts and queries while the index is being created.

.. note:: Hot index creation is slower than offline index creation.

.. variable:: tokudb_data_dir

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: No
  :vartype: String
  :default: ``NULL``

This variable configures the directory name where the |TokuDB| tables are
stored. The default value is ``NULL`` which uses the location of the |MySQL|
data directory. For more information check :ref:`tokudb_files_and_file_types`
and :ref:`tokudb_file_management`.

.. variable:: tokudb_debug

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: Yes
  :vartype: Numeric
  :range: 0 - 18446744073709551615
  :default: 0

This variable enables mysqld debug printing to ``STDERR`` for |TokuDB|.
Produces tremendous amounts of output that is nearly useless to anyone but a
|TokuDB| developer, not recommended for any production use at all. It is a mask
value ``ULONG``::

  #define TOKUDB_DEBUG_INIT                   (1<<0)
  #define TOKUDB_DEBUG_OPEN                   (1<<1)
  #define TOKUDB_DEBUG_ENTER                  (1<<2)
  #define TOKUDB_DEBUG_RETURN                 (1<<3)
  #define TOKUDB_DEBUG_ERROR                  (1<<4)
  #define TOKUDB_DEBUG_TXN                    (1<<5)
  #define TOKUDB_DEBUG_AUTO_INCREMENT         (1<<6)
  #define TOKUDB_DEBUG_INDEX_KEY              (1<<7)
  #define TOKUDB_DEBUG_LOCK                   (1<<8)
  #define TOKUDB_DEBUG_CHECK_KEY              (1<<9)
  #define TOKUDB_DEBUG_HIDE_DDL_LOCK_ERRORS   (1<<10)
  #define TOKUDB_DEBUG_ALTER_TABLE            (1<<11)
  #define TOKUDB_DEBUG_UPSERT                 (1<<12)
  #define TOKUDB_DEBUG_CHECK                  (1<<13)
  #define TOKUDB_DEBUG_ANALYZE                (1<<14)
  #define TOKUDB_DEBUG_XA                     (1<<15)
  #define TOKUDB_DEBUG_SHARE                  (1<<16)

.. variable:: tokudb_dir_per_db

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: Yes
  :vartype: Boolean
  :default: ON

When this variable is set to ``ON`` all new tables and indices will be placed
within their corresponding database directory within the
:variable:`tokudb_data_dir` or system :term:`datadir`. Existing table files
will not be automatically relocated to their corresponding database directory.
If you rename a table, while this variable is enabled, the mapping in the
|Percona FT| directory file will be updated and the files will be renamed on
disk to reflect the new table name. For more information check
:ref:`tokudb_files_and_file_types` and :ref:`tokudb_file_management`.

.. variable:: tokudb_directio

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: No
  :vartype: Boolean
  :default: OFF

When enabled, |TokuDB| employs Direct I/O rather than Buffered I/O for writes.
When using Direct I/O, consider increasing :variable:`tokudb_cache_size` from
its default of 1/2 physical memory.

.. variable:: tokudb_disable_hot_alter

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Boolean
  :default: OFF

This variable is used specifically for testing or to disable hot alter in case
there are bugs. Not for use in production.

.. variable:: tokudb_disable_prefetching

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Boolean
  :default: OFF

|TokuDB| attempts to aggressively prefetch additional blocks of rows, which is
helpful for most range queries but may create unnecessary I/O for range queries
with ``LIMIT`` clauses. Prefetching is ``ON`` by default, with a value of
``0``, it can be disabled by setting this variable to ``1``.

.. variable:: tokudb_disable_slow_alter

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Boolean
  :default: OFF

This variable is used specifically for testing or to disable hot alter in case
there are bugs. Not for use in production. It controls whether slow alter
tables are allowed. For example, the following command is slow because
``HCADER`` does not allow a mixture of column additions, deletions, or
expansions:

.. code-block:: mysql

  ALTER TABLE table
  ADD COLUMN column_a INT,
  DROP COLUMN column_b;

By default, :variable:`tokudb_disable_slow_alter` is disabled, and the engine
reports back to |MySQL| that this is unsupported resulting in the following
output:

.. code-block:: text

  ERROR 1112 (42000): Table 'test_slow' uses an extension that doesn't exist in this MySQL version

.. variable:: tokudb_empty_scan

  :cli: Yes
  :conf: Yes
  :scope: Global/Session
  :dyn: Yes
  :vartype: ENUM
  :default: ``rl``
  :range: ``disabled``, ``rl`` - right to left, ``lr`` - left to right

Defines direction to be used to perform table scan to check for empty tables
for bulk loader.

.. variable:: tokudb_enable_fast_update

  :cli: Yes
  :conf: Yes
  :scope: Global/Session
  :dyn: Yes
  :vartype: Boolean
  :default: OFF

Toggles the fast updates feature ON/OFF for the ``UPDATE`` statement. Fast
update involves queries optimization to avoid random reads during their
execution.

.. variable:: tokudb_enable_fast_upsert

  :cli: Yes
  :conf: Yes
  :scope: Global/Session
  :dyn: Yes
  :vartype: Boolean
  :default: OFF

Toggles the fast updates feature ON/OFF for the ``INSERT`` statement. Fast
update involves queries optimization to avoid random reads during their
execution.

.. variable:: tokudb_enable_partial_eviction

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: No
  :vartype: Boolean
  :default: OFF

This variable is used to control if partial eviction of nodes is enabled or
disabled.

.. variable:: tokudb_fanout

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Numeric
  :range: 2-16384
  :default: 16

This variable controls the Fractal Tree fanout. The fanout defines the number
of pivot keys or child nodes for each internal tree node.
Changing the value of :variable:`tokudb_fanout` only affects subsequently
created tables and indexes. The value of this variable cannot be changed for an
existing table/index without a dump and reload.

.. variable:: tokudb_fs_reserve_percent

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: No
  :vartype: Numeric
  :range: 0-100
  :default: 5

This variable controls the percentage of the file system that must be available
for inserts to be allowed. By default, this is set to ``5``. We recommend that
this reserve be at least half the size of your physical memory. See :ref:`Full
Disks <tokudb_full_disks>` for more information.

.. variable:: tokudb_fsync_log_period

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: Yes
  :vartype: Numeric
  :range: 0-4294967295
  :default: 0

This variable controls the frequency, in milliseconds, for ``fsync()``
operations. If set to ``0`` then the ``fsync()`` behavior is only controlled by
the :variable:`tokudb_commit_sync`, which can be ``ON`` or ``OFF``.

.. variable:: tokudb_hide_default_row_format

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Boolean
  :default: ON

This variable is used to hide the ``ROW_FORMAT`` in ``SHOW CREATE TABLE``. If
``zlib`` compression is used, row format will show as ``DEFAULT``.

.. variable:: tokudb_killed_time

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Numeric
  :range: 0-18446744073709551615
  :default: 4000

This variable is used to specify frequency in milliseconds for lock wait to
check to see if the lock was killed.

.. variable:: tokudb_last_lock_timeout

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: String
  :default: NULL

This variable contains a JSON document that describes the last lock conflict
seen by the current |MySQL| client. It gets set when a blocked lock request
times out or a lock deadlock is detected.

The :variable:`tokudb_lock_timeout_debug` session variable must have bit ``0``
set for this behavior, otherwise this session variable will be empty.

.. variable:: tokudb_load_save_space

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Boolean
  :default: ON

This session variable changes the behavior of the bulk loader. When it is
disabled the bulk loader stores intermediate data using uncompressed files
(which consumes additional CPU), whereas ``ON`` compresses the intermediate
files.

.. note::

  The location of the temporary disk space used by the bulk loader may be
  specified with the :variable:`tokudb_tmp_dir` server variable.

If a ``LOAD DATA INFILE`` statement fails with the error message ``ERROR 1030
(HY000): Got error 1`` from storage engine, then there may not be enough disk
space for the optimized loader, so disable :variable:`tokudb_prelock_empty` and
try again. More information is available in :ref:`Known Issues
<tokudb_known_issues>`.

.. variable:: tokudb_loader_memory_size

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Numeric
  :range: 0-18446744073709551615
  :default: 100000000

This variable limits the amount of memory (in bytes) that the |TokuDB| bulk
loader will use for each loader instance. Increasing this value may provide
a performance benefit when loading extremely large tables with several
secondary indexes.

.. note::

  Memory allocated to a loader is taken from the |TokuDB| cache, defined in
  :variable:`tokudb_cache_size`, and may impact the running workload's
  performance as existing cached data must be ejected for the loader to begin.


.. variable:: tokudb_lock_timeout

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Numeric
  :range: 0-18446744073709551615
  :default: 4000

This variable controls the amount of time that a transaction will wait for a
lock held by another transaction to be released. If the conflicting transaction
does not release the lock within the lock timeout, the transaction that was
waiting for the lock will get a lock timeout error. The units are milliseconds.
A value of ``0`` disables lock waits. The default value is 4000 (four seconds).

If your application gets a ``lock wait timeout`` error (-30994), then you may
find that increasing the :variable:`tokudb_lock_timeout` may help. If your
application gets a ``deadlock found`` error (-30995), then you need to abort
the current transaction and retry it.

.. variable:: tokudb_lock_timeout_debug

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Numeric
  :range: 0-3
  :default: 1


The following values are available:

  * ``0``: No lock timeouts or lock deadlocks are reported.

  * ``1``: A JSON document that describes the lock conflict is stored in the
    :variable:`tokudb_last_lock_timeout` session variable

  * ``2``: A JSON document that describes the lock conflict is printed to the
    MySQL error log.

      In addition to the JSON document describing the lock conflict, the
      following lines are printed to the MySQL error log:

      * A line containing the blocked thread id and blocked SQL
      * A line containing the blocking thread id and the blocking SQL.

  * ``3``: A JSON document that describes the lock conflict is stored in the
    :variable:`tokudb_last_lock_timeout` session variable and is printed to the
    MySQL error log.

      In addition to the JSON document describing the lock conflict, the
      following lines are printed to the MySQL error log:

      * A line containing the blocked thread id and blocked SQL
      * A line containing the blocking thread id and the blocking SQL.

.. variable:: tokudb_log_dir

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: No
  :vartype: String
  :default: NULL

This variable specifies the directory where the |TokuDB| log files are stored.
The default value is ``NULL`` which uses the location of the |MySQL| data
directory. Configuring a separate log directory is somewhat involved. Please
contact Percona support for more details. For more information check
:ref:`tokudb_files_and_file_types` and :ref:`tokudb_file_management`.

.. warning::

  After changing |TokuDB| log directory path, the old |TokuDB| recovery log
  file should be moved to new directory prior to start of |MySQL| server and
  log file's owner must be the ``mysql`` user. Otherwise server will fail to
  initialize the |TokuDB| store engine restart.

.. variable:: tokudb_max_lock_memory

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: No
  :vartype: Numeric
  :range: 0-18446744073709551615
  :default: 65560320

This variable specifies the maximum amount of memory for the PerconaFT lock
table.

.. variable:: tokudb_optimize_index_fraction

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Numeric
  :range: 0.000000 - 1.000000
  :default: 1.000000

For patterns where the left side of the tree has many deletions (a common
pattern with increasing id or date values), it may be useful to delete a
percentage of the tree. In this case, it's possible to optimize a subset of a
fractal tree starting at the left side. The
:variable:`tokudb_optimize_index_fraction` session variable controls the size
of the sub tree. Valid values are in the range [0.0,1.0] with default 1.0
(optimize the whole tree).

.. variable:: tokudb_optimize_index_name

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: String
  :default: NULL

This variable can be used to optimize a single index in a table, it can be set
to select the index by name.

.. variable:: tokudb_optimize_throttle

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Numeric
  :range: 0-18446744073709551615
  :default: 0

By default, table optimization will run with all available resources. To limit
the amount of resources, it is possible to limit the speed of table
optimization. This determines an upper bound on how many fractal tree leaf
nodes per second are optimized. The default ``0`` imposes no limit.

.. variable:: tokudb_pk_insert_mode

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Numeric
  :range: 0-3
  :default: 1

.. note::

  The :variable:`tokudb_pk_insert_mode` session variable was removed
  and the behavior is now that of the former
  :variable:`tokudb_pk_insert_mode` set to ``1``. The optimization
  will be used where safe and not used where not safe.

.. variable:: tokudb_prelock_empty

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Boolean
  :default: ON

By default |TokuDB| preemptively grabs an entire table lock on empty tables. If
one transaction is doing the loading, such as when the user is doing a table
load into an empty table, this default provides a considerable speedup.

However, if multiple transactions try to do concurrent operations on an empty
table, all but one transaction will be locked out. Disabling
:variable:`tokudb_prelock_empty` optimizes for this multi-transaction case by
turning off preemptive pre-locking.

.. note::

  If this variable is set to ``OFF``, fast bulk loading is turned off as well.

.. variable:: tokudb_read_block_size

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Numeric
  :range: 4096 - 4294967295
  :default: 16384 (16KB)

Fractal tree leaves are subdivided into read blocks, in order to speed up point
queries. This variable controls the target uncompressed size of the read
blocks. The units are bytes and the default is 64 KB. A smaller value
favors read performance for point and small range scans over large range scans
and higher compression. The minimum value of this variable is 4096 (4KB).

Changing the value of :variable:`tokudb_read_block_size` only affects
subsequently created tables. The value of this variable cannot be changed for
an existing table without a dump and reload.

.. variable:: tokudb_read_buf_size

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Numeric
  :range: 0 - 1048576
  :default: 131072 (128KB)

This variable controls the size of the buffer used to store values that are
bulk fetched as part of a large range query. Its unit is bytes and its default
value is 131,072 (128 KB).

A value of ``0`` turns off bulk fetching. Each client keeps a thread of this
size, so it should be lowered if situations where there are a large number of
clients simultaneously querying a table.

.. variable:: tokudb_read_status_frequency

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: Yes
  :vartype: Numeric
  :range: 0 - 4294967295
  :default: 10000

This variable controls in how many reads the progress is measured to display
``SHOW PROCESSLIST``. Reads are defined as ``SELECT`` queries.

For slow queries, it can be helpful to set this variable and
:variable:`tokudb_write_status_frequency` to ``1``, and then run ``SHOW
PROCESSLIST`` several times to understand what progress is being made.

.. variable:: tokudb_row_format

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: ENUM
  :range: ``TOKUDB_DEFAULT``, ``TOKUDB_FAST``, ``TOKUDB_SMALL``, ``TOKUDB_ZLIB``, ``TOKUDB_QUICKLZ``, ``TOKUDB_LZMA``, ``TOKUDB_SNAPPY``, ``TOKUDB_UNCOMPRESSED``
  :default: ``TOKUDB_QUICKLZ``

This controls the default compression algorithm used to compress data. For more information
on compression algorithms see :ref:`Compression Details <tokudb_compression>`.

.. variable:: tokudb_rpl_check_readonly

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Boolean
  :default: ON

The |TokuDB| replication code will run row events from the binary log with
:ref:`tokudb_read_free_replication` when the slave is in read-only mode. This
variable is used to disable the slave read only check in the |TokuDB|
replication code.

This allows Read-Free-Replication to run when the slave is NOT read-only. By
default, :variable:`tokudb_rpl_check_readonly` is enabled (check that slave is
read-only). Do **NOT** change this value unless you completely understand the
implications!

.. variable:: tokudb_rpl_lookup_rows

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Boolean
  :default: ON

When disabled, |TokuDB| replication slaves skip row lookups for ``delete row``
log events and ``update row`` log events, which eliminates all associated read
I/O for these operations.

.. warning::

  |TokuDB| :ref:`tokudb_read_free_replication` will not propagate ``UPDATE``
  and ``DELETE`` events reliably if |TokuDB| table is missing the primary key
  which will eventually lead to data inconsistency on the slave.

.. note::

  Optimization is only enabled when :variable:`read_only` is set to ``1`` and
  :variable:`binlog_format` is ``ROW``.

.. variable:: tokudb_rpl_lookup_rows_delay

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Numeric
  :range: 0 - 18446744073709551615
  :default: 0

This variable allows for simulation of long disk reads by sleeping for the
given number of microseconds prior to the row lookup query, it should only be
set to a non-zero value for testing.

.. variable:: tokudb_rpl_unique_checks

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Boolean
  :default: ON

When disabled, |TokuDB| replication slaves skip uniqueness checks on inserts
and updates, which eliminates all associated read I/O for these operations.

.. note::

  Optimization is only enabled when :variable:`read_only` is set to ``1`` and
  :variable:`binlog_format` is ``ROW``.

.. variable:: tokudb_rpl_unique_checks_delay

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Numeric
  :range: 0 - 18446744073709551615
  :default: 0

This variable allows for simulation of long disk reads by sleeping for the
given number of microseconds prior to the row lookup query, it should only be
set to a non-zero value for testing.

.. variable:: tokudb_strip_frm_data

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: No
  :vartype: Boolean
  :default: OFF

When this variable is set to ``ON`` during the startup server will check all
the status files and remove the embedded :file:`.frm` metadata. This variable
can be used to assist in |TokuDB| data recovery. **WARNING:** Use this variable
only if you know what you're doing otherwise it could lead to data loss.

.. variable:: tokudb_support_xa

  :cli: Yes
  :conf: Yes
  :scope: Session, Global
  :dyn: Yes
  :vartype: Boolean
  :default: ON

This variable defines whether or not the prepare phase of an XA transaction
performs an ``fsync()``.

.. variable:: tokudb_tmp_dir

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: No
  :vartype: String

This variable specifies the directory where the |TokuDB| bulk loader stores
temporary files. The bulk loader can create large temporary files while it is
loading a table, so putting these temporary files on a disk separate from the
data directory can be useful.

:variable:`tokudb_load_save_space` determines whether the data is compressed or
not. The error message ``ERROR 1030 (HY000): Got error 1 from storage engine``
could indicate that the disk has run out of space.

For example, it can make sense to use a high-performance disk for the data
directory and a very inexpensive disk for the temporary directory. The default
location for temporary files is the |MySQL| data directory.

For more information check :ref:`tokudb_files_and_file_types` and
:ref:`tokudb_file_management`.

.. variable:: tokudb_version

  :cli: No
  :conf: No
  :scope: Global
  :dyn: No
  :vartype: String

This read-only variable documents the version of the |TokuDB| storage engine.

.. variable:: tokudb_write_status_frequency

  :cli: Yes
  :conf: Yes
  :scope: Global
  :dyn: Yes
  :vartype: Numeric
  :range: 0 - 4294967295
  :default: 1000

This variable controls in how many writes the progress is measured to display
``SHOW PROCESSLIST``. Writes are defined as ``INSERT``, ``UPDATE`` and
``DELETE`` queries.

For slow queries, it can be helpful to set this variable and
:variable:`tokudb_read_status_frequency` to 1, and then run ``SHOW
PROCESSLIST`` several times to understand what progress is being made.
