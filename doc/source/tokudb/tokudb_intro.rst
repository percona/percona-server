.. _tokudb_intro:

================================================================================
TokuDB Introduction
================================================================================

:Availability: |TokuDB| is deprecated in the 8.0 series and will be supported
               through the 8.0 series until further notice. This storage engine
               will not be included in the next major release of |Percona
               Server|. We recommend MyRocks as a long-term migration path.

.. Percona TokuBackup is very limited in 8.0, and the fixes to bring it up to speed have been deferred

|TokuDB| is a highly scalable, zero-maintenance downtime MySQL storage engine
that delivers indexing-based query acceleration, improved replication
performance, unparalleled compression, and live schema modification. The
|TokuDB| storage engine is a scalable, ACID and MVCC compliant storage engine
that provides indexing-based query improvements, offers online schema
modifications, and reduces slave lag for both hard disk drives and flash
memory. This storage engine is specifically designed for high performance on
write-intensive workloads which is achieved with Fractal Tree indexing.

|Percona Server| is compatible with the separately available |TokuDB| storage
engine package. The |TokuDB| engine must be separately downloaded and then
enabled as a plug-in component. This package can be installed alongside with
standard |Percona Server| releases and does not require any specially adapted
version of |Percona Server|.

.. warning::

   Only the `Percona supplied
   <http://www.percona.com/downloads/Percona-Server-8.0/LATEST/>`_ |TokuDB|
   engine should be used with |Percona Server|. A |TokuDB| engine downloaded
   from other sources is not compatible. |TokuDB| file formats are not the same
   across |MySQL| variants. Migrating from one variant to any other variant
   requires a logical data dump and reload.

Additional features unique to |TokuDB| include:

- Up to 25x Data Compression
- Fast Inserts
- Eliminates Slave Lag with :ref:`Read Free Replication <tokudb_read_free_replication>`
- Hot Schema Changes
- Hot Index Creation - |TokuDB| tables support insertions, deletions and queries
  with no down time while indexes are being added to that table
- Hot column addition, deletion, expansion, and rename - |TokuDB| tables support
  insertions, deletions and queries without down-time when an alter table adds,
  deletes, expands, or renames columns
- On-line Backup 

.. note::

   The |TokuDB| storage engine does not support the |sql.no-wait| and
   |sql.skip-locked| modifiers introduced in the |InnoDB| storage
   engine with |MySQL| 8.0.
  
For more information on installing and using |TokuDB| click on the following
links:

.. toctree::
   :maxdepth: 1

   tokudb_installation
   using_tokudb
   tokudb_quickstart
   tokudb_variables
   toku_backup
   tokudb_troubleshooting
   tokudb_faq
   removing_tokudb

Getting the Most from TokuDB
----------------------------

Compression
   |TokuDB| compresses all data on disk, including indexes. Compression
   lowers cost by reducing the amount of storage required and frees up disk
   space for additional indexes to achieve improved query performance. Depending
   on the compressibility of the data, we have seen compression ratios up to 25x
   for high compression. Compression can also lead to improved performance since
   less data needs to be read from and written to disk.

Fast Insertions and Deletions
   TokuDB's Fractal Tree technology enables fast
   indexed insertions and deletions. Fractal Trees match B-trees in their
   indexing sweet spot (sequential data) and are up to two orders of magnitude
   faster for random data with high cardinality.

Eliminates Slave Lag
   |TokuDB| replication slaves can be configured to process
   the replication stream with virtually no read IO. Uniqueness checking is
   performed on the |TokuDB| master and can be skipped on all |TokuDB|
   slaves. Also, row based replication ensures that all before and after row
   images are captured in the binary logs, so the |TokuDB| slaves can harness
   the power of Fractal Tree indexes and bypass traditional read-modify-write
   behavior. This "Read Free Replication" ensures that replication slaves do not
   fall behind the master and can be used for read scaling, backups, and
   disaster recovery, without sharding, expensive hardware, or limits on what
   can be replicated.

Hot Index Creation
   |TokuDB| allows the addition of indexes to an existing table
   while inserts and queries are being performed on that table. This means that
   |MySQL| can be run continuously with no blocking of queries or insertions
   while indexes are added and eliminates the down-time that index changes would
   otherwise require.

Hot Column Addition, Deletion, Expansion and Rename
   |TokuDB| allows the addition
   of new columns to an existing table, the deletion of existing columns from an
   existing table, the expansion of ``char``, ``varchar``, ``varbinary``, and
   ``integer`` type columns in an existing table, and the renaming of an
   existing column while inserts and queries are being performed on that table.

Online (Hot) Backup
   The |TokuDB| can create backups of online database servers without downtime.

Fast Indexing
   In practice, slow indexing often leads users to choose a smaller
   number of sub-optimal indexes in order to keep up with incoming data
   rates. These sub-optimal indexes result in disproportionately slower queries,
   since the difference in speed between a query with an index and the same
   query when no index is available can be many orders of magnitude. Thus, fast
   indexing means fast queries.

Clustering Keys and Other Indexing Improvements
   |TokuDB| tables are clustered on
   the primary key. |TokuDB| also supports clustering secondary keys, providing
   better performance on a broader range of queries. A clustering key includes
   (or clusters) all of the columns in a table along with the key. As a result,
   one can efficiently retrieve any column when doing a range query on a
   clustering key. Also, with |TokuDB|, an auto-increment column can be used in
   any index and in any position within an index. Lastly, |TokuDB| indexes can
   include up to 32 columns.

Less Aging/Fragmentation
   |TokuDB| can run much longer, likely indefinitely,
   without the need to perform the customary practice of dump/reload or
   ``OPTIMIZE TABLE`` to restore database performance. The key is the
   fundamental difference with which the Fractal Tree stores data on
   disk. Since, by default, the Fractal Tree will store data in 4MB chunks
   (pre-compression), as compared to InnoDB's 16KB, |TokuDB| has the ability to
   avoid "database disorder" up to 250x better than InnoDB.

Bulk Loader
   |TokuDB| uses a parallel loader to create tables and offline
   indexes. This parallel loader will use multiple cores for fast offline table
   and index creation.

Full-Featured Database
   |TokuDB| supports fully ACID-compliant transactions, MVCC
   (Multi-Version Concurrency Control), serialized isolation levels, row-level
   locking, and XA. |TokuDB| scales with high number of client connections, even
   for large tables.

Lock Diagnostics
   |TokuDB| provides users with the tools to diagnose locking and
   deadlock issues. For more information, see :ref:`Lock Visualization in TokuDB
   <tokudb_lock_visualization>`.

Progress Tracking
   Running ``SHOW PROCESSLIST`` when adding indexes provides
   status on how many rows have been processed. Running ``SHOW PROCESSLIST``
   also shows progress on queries, as well as insertions, deletions and
   updates. This information is helpful for estimating how long operations will
   take to complete.

Fast Recovery
   |TokuDB| supports very fast recovery, typically less than a minute.
