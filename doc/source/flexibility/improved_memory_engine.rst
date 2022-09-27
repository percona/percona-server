.. _improved_memory_engine:

====================================
 ``Improved MEMORY`` Storage Engine
====================================

As of ``MySQL`` 5.5.15, a *Fixed Row Format* (``FRF``) is still being used in the ``MEMORY`` storage engine. The fixed row format imposes restrictions on the type of columns as it assigns on advance a limited amount of memory per row. This renders a ``VARCHAR`` field in a ``CHAR`` field in practice and makes impossible to have a ``TEXT`` or ``BLOB`` field with that engine implementation.

To overcome this limitation, the *Improved MEMORY Storage Engine* is introduced in this release for supporting **true** ``VARCHAR``, ``VARBINARY``, ``TEXT`` and ``BLOB`` fields in ``MEMORY`` tables.

This implementation is based on the *Dynamic Row Format* (``DFR``) introduced by the `mysql-heap-dynamic-rows <http://code.google.com/p/mysql-heap-dynamic-rows/>`_ patch.

``DFR`` is used to store column values in a variable-length form, thus helping to decrease memory footprint of those columns and making possible ``BLOB`` and ``TEXT`` fields and real ``VARCHAR`` and ``VARBINARY``.

Unlike the fixed implementation, each column value in ``DRF`` only uses as much space as required. This is, for variable-length values, up to 4 bytes is used to store the actual value length, and then only the necessary number of blocks is used to store the value.

Rows in ``DFR`` are represented internally by multiple memory blocks, which means that a single row can consist of multiple blocks organized into one set. Each row occupies at least one block, there can not be multiple rows within a single block. Block size can be configured when creating a table (see below).

This ``DFR`` implementation has two caveats regarding to ordering and indexes.

Caveats
=======

Ordering of Rows
----------------

In the absence of ``ORDER BY``, records may be returned in a different order than the previous ``MEMORY`` implementation.

This is not a bug. Any application relying on a specific order without an ``ORDER BY`` clause may deliver unexpected results. A specific order without ``ORDER BY`` is a side effect of a storage engine and query optimizer implementation which may and will change between minor *MySQL* releases.


Indexing
--------

It is currently impossible to use indexes on ``BLOB`` columns due to some limitations of the *Dynamic Row Format*. Trying to create such an index will fail with the following error: ::

  BLOB column '<name>' can't be used in key specification with the used table type.

Restrictions
============

For performance reasons, a mixed solution is implemented: the fixed format is used at the beginning of the row, while the dynamic one is used for the rest of it.

The size of the fixed-format portion of the record is chosen automatically on ``CREATE TABLE`` and cannot be changed later. This, in particular, means that no indexes can be created later with ``CREATE INDEX`` or ``ALTER TABLE`` when the dynamic row format is used. 

All values for columns used in indexes are stored in fixed format at the first block of the row, then the following columns are handled with ``DRF``.

This sets two restrictions to tables:

  * the order of the fields and therefore,

  * the minimum size of the block used in the table.

Ordering of Columns
-------------------

The columns used in fixed format must be defined before the dynamic ones in the ``CREATE TABLE`` statement. If this requirement is not met, the engine will not be able to add blocks to the set for these fields and they will be treated as fixed.

Minimum Block Size
------------------

The block size has to be big enough to store all fixed-length information in the first block. If not, the ``CREATE TABLE`` or ``ALTER TABLE`` statements will fail (see below).

Limitations
===========

*MyISAM* tables are still used for query optimizer internal temporary tables where the ``MEMORY`` tables could be used now instead: for temporary tables containing large ``VARCHAR``s, ``BLOB``, and ``TEXT`` columns.

Setting Row Format
==================

Taking the restrictions into account, the *Improved MEMORY Storage Engine* will choose ``DRF`` over ``FRF`` at the moment of creating the table according to following criteria:

  * There is an implicit request of the user in the column types **OR**

  * There is an explicit request of the user **AND** the overhead incurred by ``DFR`` is beneficial.

Implicit Request
----------------

The implicit request by the user is taken when there is at least one ``BLOB`` or ``TEXT`` column in the table definition. If there are none of these columns and no relevant option is given, the engine will choose ``FRF``.

For example, this will yield the use of the dynamic format: ::

  mysql> CREATE TABLE t1 (f1 VARCHAR(32), f2 TEXT, PRIMARY KEY (f1)) ENGINE=HEAP;

While this will not: ::

  mysql> CREATE TABLE t1 (f1 VARCHAR(16), f2 VARCHAR(16), PRIMARY KEY (f1)) ENGINE=HEAP;

Explicit Request
----------------

The explicit request is set with one of the following options in the ``CREATE TABLE`` statement:

  * ``KEY_BLOCK_SIZE = <value>``

    * Requests the DFR with the specified block size (in bytes)

Despite its name, the ``KEY_BLOCK_SIZE`` option refers to a block size used to store data rather then indexes. The reason for this is that an existing ``CREATE TABLE`` option is reused to avoid introducing new ones.

*The Improved MEMORY Engine* checks whether the specified block size is large enough to keep all key column values. If it is too small, table creation will abort with an error.

After ``DRF`` is requested explicitly and there are no ``BLOB`` or ``TEXT`` columns in the table definition, the *Improved MEMORY Engine* will check if using the dynamic format provides any space saving benefits as compared to the fixed one:

  * if the fixed row length is less than the dynamic block size (plus the dynamic row overhead - platform dependent) **OR**

  * there isn't any variable-length columns in the table or ``VARCHAR`` fields are declared with length 31 or less,

the engine will revert to the fixed format as it is more space efficient in such case. The row format being used by the engine can be checked using ``SHOW TABLE STATUS``.

Examples
========

On a 32-bit platform: ::

  mysql> CREATE TABLE t1 (f1 VARCHAR(32), f2 VARCHAR(32), f3 VARCHAR(32), f4 VARCHAR(32),
                          PRIMARY KEY (f1)) KEY_BLOCK_SIZE=124 ENGINE=HEAP;
  
  mysql> SHOW TABLE STATUS LIKE 't1';
  Name	Engine	Version	   Rows	Avg_row_length	Data_length	Max_data_length	Index_length	Data_free	Auto_increment	Create_time	Update_time	Check_time	Collation	Checksum	Create_options	Comment
  t1	MEMORY	10	   X	0	X	0	0	NULL	NULL	NULL	NULL	latin1_swedish_ci	NULL	row_format=DYNAMIC KEY_BLOCK_SIZE=124	

On a 64-bit platform: ::

  mysql> CREATE TABLE t1 (f1 VARCHAR(32), f2 VARCHAR(32), f3 VARCHAR(32), f4 VARCHAR(32),
                          PRIMARY KEY (f1)) KEY_BLOCK_SIZE=124 ENGINE=HEAP;
  
  mysql> SHOW TABLE STATUS LIKE 't1';
  Name	Engine	Version	   Rows	Avg_row_length	Data_length	Max_data_length	Index_length	Data_free	Auto_increment	Create_time	Update_time	Check_time	Collation	Checksum	Create_options	Comment	
  t1	MEMORY	10	   X	0	X	0	0	NULL	NULL	NULL	NULL	latin1_swedish_ci	NULL	KEY_BLOCK_SIZE=124	

Implementation Details
======================

*MySQL* *MEMORY* tables keep data in arrays of fixed-size chunks. These chunks are organized into two groups of ``HP_BLOCK`` structures:

  * ``group1`` contains indexes, with one ``HP_BLOCK`` per key (part of ``HP_KEYDEF``),

  * ``group2`` contains record data, with a single ``HP_BLOCK`` for all records.

While columns used in indexes are usually small, other columns in the table may need to accommodate larger data. Typically, larger data is placed into ``VARCHAR`` or ``BLOB`` columns.

*The Improved MEMORY Engine* implements the concept of dataspace, ``HP_DATASPACE``, which incorporates the ``HP_BLOCK`` structures for the record data, adding more information for managing variable-sized records.

Variable-size records are stored in multiple “chunks”, which means that a single record of data (a database “row”) can consist of multiple chunks organized into one “set”, contained in ``HP_BLOCK`` structures.

In variable-size format, one record is represented as one or many chunks depending on the actual data, while in fixed-size mode, one record is always represented as one chunk. The index structures would always point to the first chunk in the chunkset.

Variable-size records are necessary only in the presence of variable-size columns. The *Improved Memory Engine* will be looking for ``BLOB`` or ``VARCHAR`` columns with a declared length of 32 or more. If no such columns are found, the table will be switched to the fixed-size format. You should always put such columns at the end of the table definition in order to use the variable-size format.

Whenever data is being inserted or updated in the table, the *Improved Memory Engine* will calculate how many chunks are necessary.

For ``INSERT`` operations, the engine only allocates new chunksets in the recordspace. For ``UPDATE`` operations it will modify the length of the existing chunkset if necessary, unlinking unnecessary chunks at the end, or allocating and adding more if a larger length is needed.

When writing data to chunks or copying data back to a record, fixed-size columns are copied in their full format, while ``VARCHAR`` and ``BLOB`` columns are copied based on their actual length, skipping any ``NULL`` values.

When allocating a new chunkset of N chunks, the engine will try to allocate chunks one-by-one, linking them as they become allocated. For allocating a single chunk, it will attempt to reuse a deleted (freed) chunk. If no free chunks are available, it will try to allocate a new area inside a ``HP_BLOCK``.

When freeing chunks, the engine will place them at the front of a free list in the dataspace, each one containing a reference to the previously freed chunk.

The allocation and contents of the actual chunks varies between fixed and variable-size modes:

  * Format of a fixed-size chunk:

    * ``uchar[]``

      * With ``sizeof=chunk_dataspace_length``, but at least ``sizeof(uchar*)`` bytes. It keeps actual data or pointer to the next deleted chunk, where ``chunk_dataspace_length`` equals to full record length

    * ``uchar`` 

      * Status field (1 means “in use”, 0 means “deleted”)

  * Format of a variable-size chunk:

      * ``uchar[]``

        * With ``sizeof=chunk_dataspace_length``, but at least ``sizeof(uchar*)`` bytes. It keeps actual data or pointer to the next deleted chunk, where ``chunk_dataspace_length`` is set according to table's ``key_block_size``

    * ``uchar*`` 

      * Pointer to the next chunk in this chunkset, or NULL for the last chunk

    * ``uchar``

      * Status field (1 means “first”, 0 means “deleted”, 2 means “linked”)

Total chunk length is always aligned to the next ``sizeof(uchar*)``.

See Also
========

  * `Dynamic row format for MEMORY tables <http://www.mysqlperformanceblog.com/2011/09/06/dynamic-row-format-for-memory-tables/>`_
