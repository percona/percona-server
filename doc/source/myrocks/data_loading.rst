.. _data-loading:

============
Data Loading
============

By default, MyRocks configurations are optimized for short transactions,
and not for data loading. MyRocks has a couple of special session variables
to speed up data loading dramatically.

Sorted bulk loading
===================

If your data is guaranteed to be loaded in primary key order, then this method
is recommended. This method works by dropping any secondary keys first, loading
data into your table in primary key order, and then restoring the secondary
keys via Fast Secondary Index Creation.

Creating Secondary Indexes
--------------------------

When loading data into empty tables, it is highly recommended to drop all
secondary indexes first, then loading data, and adding all secondary indexes
after finishing loading data. MyRocks has a feature called ``Fast Secondary
Index Creation``. Fast Secondary Index Creation is automatically used when
executing ``CREATE INDEX`` or ``ALTER TABLE ... ADD INDEX``. With Fast
Secondary Index Creation, the secondary index entries are directly written
to bottommost RocksDB levels and bypassing compaction. This significantly
reduces total write volume and CPU time for decompressing and compressing
data on higher levels.

.. _myrocks_data_loading:

Loading Data
------------

As described above, loading data is highly recommended for tables with primary
key only (no secondary keys), with all secondary indexes added after loading
data.

When loading data into MyRocks tables, there are two recommended session
variables:

.. code-block:: mysql

  SET session sql_log_bin=0;
  SET session rocksdb_bulk_load=1;

When converting from large MyISAM/InnoDB tables, either by using the ``ALTER``
or ``INSERT INTO SELECT`` statements it's recommended that you
create MyRocks tables as below (in case the table is sufficiently big it will
cause the server to consume all the memory and then be terminated by the OOM
killer):

.. code-block:: mysql

  SET session sql_log_bin=0;
  SET session rocksdb_bulk_load=1;
  ALTER TABLE large_myisam_table ENGINE=RocksDB;
  SET session rocksdb_bulk_load=0;

Using sql_log_bin=0 avoids writing to binary logs.

With :ref:`rocksdb_bulk_load` set to ``1``, MyRocks enters special mode to
write all inserts into bottommost RocksDB levels, and skips writing data into
MemTable and the following compactions. This is very efficient way to load
data.

The :ref:`rocksdb_bulk_load` mode operates with a few conditions:

* None of the data being bulk loaded can overlap with existing data in the
  table. The easiest way to ensure this is to always bulk load into an empty
  table, but the mode will allow loading some data into the table, doing other
  operations, and then returning and bulk loading addition data if there is no
  overlap between what is being loaded and what already exists.

* The data may not be visible until bulk load mode is ended (i.e. the
  :ref:`rocksdb_bulk_load` is set to zero again). The method that is used
  is building up SST files which will later be added as-is to the database.
  Until a particular SST has been added the data will not be visible to the
  rest of the system, thus issuing a ``SELECT`` on the table currently being
  bulk loaded will only show older data and will likely not show the most
  recently added rows. Ending the bulk load mode will cause the most recent SST
  file to be added. When bulk loading multiple tables, starting a new table
  will trigger the code to add the most recent SST file to the system -- as a
  result, it is inadvisable to interleave ``INSERT`` statements to two or more
  tables during bulk load mode.

By default, the :ref:`rocksdb_bulk_load` mode expects all data be inserted
in primary key order (or reversed order). If the data is in the reverse order
(i.e. the data is descending on a normally ordered primary key or is ascending
on a reverse ordered primary key), the rows are cached in chunks to switch the
order to match the expected order.

Inserting one or more rows out of order will result in an error and may result
in some of the data being inserted in the table and some not. To resolve the
problem, one can either fix the data order of the insert, truncate the table,
and restart.

Unsorted bulk loading
=====================

If your data is not ordered in primary key order, then this method is
recommended. With this method, secondary keys do not need to be dropped and
restored. However, writing to the primary key no longer goes directly to SST
files, and are written to temporary files for sorted first, so there is extra
cost to this method.

To allow for loading unsorted data:

.. code-block:: mysql

  SET session sql_log_bin=0;
  SET session rocksdb_bulk_load_allow_unsorted=1;
  SET session rocksdb_bulk_load=1;
  ...
  SET session rocksdb_bulk_load=0;
  SET session rocksdb_bulk_load_allow_unsorted=0;

Note that :ref:`rocksdb_bulk_load_allow_unsorted` can only be changed when
:ref:`rocksdb_bulk_load` is disabled (set to ``0``). In this case, all
input data will go through an intermediate step that writes the rows to
temporary SST files, sorts them rows in the primary key order, and then writes
to final SST files in the correct order.

Other Approaches
================

If :ref:`rocksdb_commit_in_the_middle` is enabled, MyRocks implicitly
commits every :ref:`rocksdb_bulk_load_size` records (default is ``1,000``)
in the middle of your transaction. If your data loading fails in the middle of
the statement (``LOAD DATA`` or bulk ``INSERT``), rows are not entirely rolled
back, but some of rows are stored in the table. To restart data loading, you'll
need to truncate the table and loading data again.

.. warning::

  If you are loading large data without enabling :ref:`rocksdb_bulk_load`
  or :ref:`rocksdb_commit_in_the_middle`, please make sure transaction
  size is small enough. All modifications of the ongoing transactions are kept
  in memory.

Other Reading
=============
* `Data Loading <https://github.com/facebook/mysql-5.6/wiki/Data-Loading>`_ -
  this document has been used as a source for writing this documentation
* `ALTER TABLE ... ENGINE=ROCKSDB uses too much memory
  <https://github.com/facebook/mysql-5.6/issues/692>`_
