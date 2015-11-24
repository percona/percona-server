.. _misc_info_schema_tables:

=================================
 Misc. INFORMATION_SCHEMA Tables
=================================

Temporary tables
================

 Only the temporary tables that were explicitly created with `CREATE TEMPORARY TABLE` or `ALTER TABLE` are shown, and not the ones created to process complex queries. 

.. table:: INFORMATION_SCHEMA.GLOBAL_TEMPORARY_TABLES

   :column SESSION_ID: |MySQL| connection id
   :column TABLE_SCHEMA: Schema in which the temporary table is created
   :column TABLE_NAME: Name of the temporary table
   :column ENGINE: Engine of the temporary table
   :column NAME: Internal name of the temporary table
   :column TABLE_ROWS: Number of rows of the temporary table
   :column AVG_ROW_LENGTH: Average row length of the temporary table
   :column DATA_LENGTH: Size of the data (Bytes)
   :column INDEX_LENGTH: Size of the indexes (Bytes)
   :column CREATE_TIME: Date and time of creation of the temporary table
   :column UPDATE_TIME: Date and time of the latest update of the temporary table

This table holds information on the temporary tables existing for all connections. You don't need the ``SUPER`` privilege to query this table.

This information is also available by running the following command: 
.. code-block:: mysql

  SHOW GLOBAL TEMPORARY TABLES

.. table:: INFORMATION_SCHEMA.TEMPORARY_TABLES

   :column SESSION_ID: |MySQL| connection id
   :column TABLE_SCHEMA: Schema in which the temporary table is created
   :column TABLE_NAME: Name of the temporary table
   :column ENGINE: Engine of the temporary table
   :column NAME: Internal name of the temporary table
   :column TABLE_ROWS: Number of rows of the temporary table
   :column AVG_ROW_LENGTH: Average row length of the temporary table
   :column DATA_LENGTH: Size of the data (Bytes)
   :column INDEX_LENGTH: Size of the indexes (Bytes)
   :column CREATE_TIME: Date and time of creation of the temporary table
   :column UPDATE_TIME: Date and time of the latest update of the temporary table

This table holds information on the temporary tables existing for the running connection.

This information is also available by running the following command: 
.. code-block:: mysql

  SHOW TEMPORARY TABLES

Status Variables
----------------

.. variable:: Com_show_temporary_tables

  :vartype: Numeric
  :scope: Global/Session

The :variable:`Com_show_temporary_tables` statement counter variable indicates the number of times the statements ``SHOW GLOBAL TEMPORARY TABLES`` and ``SHOW TEMPORARY TABLES`` have been executed.

Buffer Pool Data Structure Tables
=================================

The following tables provide various information about the contents of the |InnoDB| buffer pool.

.. table:: INFORMATION_SCHEMA.INNODB_BUFFER_POOL_PAGES

  :column PAGE_TYPE: Type of the page. Possible values: index, undo_log, inode, ibuf_free_list, allocated, bitmap, sys, trx_sys, fsp_hdr, xdes, blob, zblob, zblob2, unknown
  :column SPACE_ID: Tablespace ID
  :column PAGE_NO:  Page offset within its tablespace
  :column LRU_POSITION: this field is always ``0`` and will be removed in a future Percona Server release
  :column FIX_COUNT: reference count of a page. It is incremented every time the page is accessed by |InnoDB|, and is 0 if and only if the page is not currently being accessed
  :column FLUSH_TYPE: type of the last flush of the page (0:LRU 2:flush_list)

Example: ::

  mysql> select * from information_schema.INNODB_BUFFER_POOL_PAGES LIMIT 20;  
  +-----------+----------+---------+--------------+-----------+------------+
  | page_type | space_id | page_no | lru_position | fix_count | flush_type |
  +-----------+----------+---------+--------------+-----------+------------+
  | allocated |        0 |       7 |            0 |         0 |          2 | 
  | allocated |        0 |       1 |            0 |         0 |          0 | 
  | allocated |        0 |       3 |            0 |         0 |          0 | 
  | inode     |        0 |       2 |            0 |         0 |          2 | 
  | index     |        0 |       4 |            0 |         0 |          2 | 
  | index     |        0 |      11 |            0 |         0 |          0 | 
  | index     |        0 |   12956 |            0 |         0 |          0 | 
  | allocated |        0 |       5 |            0 |         0 |          2 | 
  | allocated |        0 |       6 |            0 |         0 |          2 | 
  | undo_log  |        0 |      51 |            0 |         0 |          2 | 
  | undo_log  |        0 |      52 |            0 |         0 |          2 | 
  | index     |        0 |       8 |            0 |         0 |          0 | 
  | index     |        0 |     288 |            0 |         0 |          0 | 
  | index     |        0 |     290 |            0 |         0 |          2 | 
  | index     |        0 |     304 |            0 |         0 |          0 | 
  | allocated |        0 |       0 |            0 |         0 |          2 | 
  | index     |        0 |      10 |            0 |         0 |          0 | 
  | index     |        0 |   12973 |            0 |         0 |          0 | 
  | index     |        0 |       9 |            0 |         0 |          2 | 
  | index     |        0 |      12 |            0 |         0 |          0 | 
  +-----------+----------+---------+--------------+-----------+------------+
  20 rows in set (0.81 sec)

This table shows the characteristics of the allocated pages in buffer pool and current state of them.

.. table:: INFORMATION_SCHEMA.INNODB_BUFFER_POOL_PAGES_INDEX

  :column index_id: index name
  :column space_id: Tablespace ID
  :column page_no: Page offset within its tablespace
  :column n_recs: number of user records on page
  :column data_size: sum of the sizes of the records in page
  :column hashed: the block is in adaptive hash index (1) or not (0)
  :column access_time: time of the last access to that page
  :column modified: modified since loaded (1) or not (0)
  :column dirty: modified since last flushed (1) or not (0)
  :column old: is old blocks in the LRU list (1) or not (0)
  :column lru_position: page position in the LRU list
  :column fix_count: reference count of a page. It is incremented every time the page is accessed by |InnoDB|, and is 0 if and only if the page is not currently being accessed
  :column flush_type: type of the last flush of the page (0:LRU 2:flush_list)

Example: ::

  +----------+----------+---------+--------+-----------+--------+-------------+----------+-------+-----+--------------+-----------+------------+
  | index_id | space_id | page_no | n_recs | data_size | hashed | access_time | modified | dirty | old | lru_position | fix_count | flush_type |
  +----------+----------+---------+--------+-----------+--------+-------------+----------+-------+-----+--------------+-----------+------------+
  |       39 |        0 |    5787 |    468 |     14976 |      1 |  2636182517 |        1 |     0 |   1 |            0 |         0 |          2 |
  |       40 |        0 |    5647 |   1300 |     15600 |      1 |  2636182517 |        1 |     0 |   1 |            0 |         0 |          2 |
  |       39 |        0 |    5786 |    468 |     14976 |      1 |  2636182516 |        1 |     0 |   1 |            0 |         0 |          2 |
  |       40 |        0 |    6938 |   1300 |     15600 |      1 |  2636193968 |        1 |     0 |   1 |            0 |         0 |          2 |
  |       39 |        0 |    5785 |    468 |     14976 |      1 |  2636182514 |        1 |     0 |   1 |            0 |         0 |          2 |
  |       39 |        0 |    5784 |    468 |     14976 |      1 |  2636182512 |        1 |     0 |   1 |            0 |         0 |          2 |
  |       40 |        0 |    5646 |   1300 |     15600 |      1 |  2636182511 |        1 |     0 |   1 |            0 |         0 |          2 |
  |       39 |        0 |    7203 |    468 |     14976 |      1 |  2636193967 |        1 |     0 |   1 |            0 |         0 |          2 |
  |       39 |        0 |    5783 |    468 |     14976 |      1 |  2636182507 |        1 |     0 |   1 |            0 |         0 |          2 |
  |       39 |        0 |    5782 |    468 |     14976 |      1 |  2636182506 |        1 |     0 |   1 |            0 |         0 |          2 |
  +----------+----------+---------+--------+-----------+--------+-------------+----------+-------+-----+--------------+-----------+------------+

This table shows information about the index pages located in the buffer pool.

.. table:: INFORMATION_SCHEMA.INNODB_BUFFER_POOL_PAGES_BLOB

  :column space_id: tablespace ID
  :column page_no: page offset within its tablespace
  :column compressed:    contains compressed data (1) or not (0)
  :column part_len:  data length in the page
  :column next_page_no:  page number of the next data
  :column lru_position: page position in the LRU list
  :column fix_count: reference count of a page. It is incremented every time the page is accessed by InnoDB, and is 0 if and only if the page is not currently being accessed
  :column flush_type:    type of the last flush of the page (0:LRU 2:flush_list)

Example: ::

  mysql> select * from information_schema.INNODB_BUFFER_POOL_PAGES_BLOB LIMIT 20;
  +----------+---------+------------+----------+--------------+--------------+-----------+------------+
  | space_id | page_no | compressed | part_len | next_page_no | lru_position | fix_count | flush_type |
  +----------+---------+------------+----------+--------------+--------------+-----------+------------+
  |     1748 |     111 |          0 |    10137 |            0 |          263 |         0 |          2 | 
  |     1748 |     307 |          0 |     5210 |            0 |         1084 |         0 |          2 | 
  |     1748 |    1329 |          0 |     6146 |            0 |         4244 |         0 |          2 | 
  |     1748 |    1330 |          0 |    11475 |            0 |         4245 |         0 |          2 | 
  |     1748 |    1345 |          0 |     5550 |            0 |         4247 |         0 |          2 | 
  |     1748 |    1346 |          0 |     7597 |            0 |         4248 |         0 |          2 | 
  |     1748 |    3105 |          0 |     6716 |            0 |         8919 |         0 |          2 | 
  |     1748 |    3213 |          0 |     8170 |            0 |         9390 |         0 |          2 | 
  |     1748 |    6142 |          0 |     5648 |            0 |        19638 |         0 |          2 | 
  |     1748 |    7387 |          0 |    10634 |            0 |        24191 |         0 |          2 | 
  |     1748 |    7426 |          0 |     5355 |            0 |        24194 |         0 |          2 | 
  |     1748 |    7489 |          0 |    16330 |         7489 |        24196 |         0 |          2 | 
  |     1748 |    7490 |          0 |     7126 |            0 |        24197 |         0 |          2 | 
  |     1748 |    7657 |          0 |    13571 |            0 |        24681 |         0 |          2 | 
  |     1748 |    7840 |          0 |    11208 |            0 |        25737 |         0 |          2 | 
  |     1748 |    9599 |          0 |    11882 |            0 |        31989 |         0 |          2 | 
  |     1748 |   11719 |          0 |     7367 |            0 |        40466 |         0 |          2 | 
  |     1748 |   12051 |          0 |    11049 |            0 |        41441 |         0 |          2 | 
  |     1748 |   12052 |          0 |    16330 |        12052 |        41442 |         0 |          2 | 
  |     1748 |   12053 |          0 |     2674 |            0 |        41443 |         0 |          2 | 
  +----------+---------+------------+----------+--------------+--------------+-----------+------------+
  20 rows in set (0.05 sec)

This table shows information from blob pages located in buffer pool.

InnoDB Undo Logs
================

The purpose of this table is to report on the existence and usage of the internal undo log records. These undo records are stored in standard |InnoDB| pages and are used in a few ways but their main purpose is that currently executing but uncommitted user transactions can be rolled back after either a crash, fast shutdown or other recovery purpose. Each record within the table identifies an |InnoDB| undo segment and will refer to other INFORMATION_SCHEMA tables such as INNODB_TRX and INODB_RSEG. This table can be used to help troubleshoot large system tablespaces and identify run-away or long running transactions.

.. table:: INFORMATION_SCHEMA.INNODB_UNDO_LOGS

   :column trx_id: Transaction ID - this is the id of the transaction that has currently allocated the undo segment and will potentially place undo records within it. More information on this transaction can be found by matching the trx_id with that in the INFORMATION_SCHEMA.INNODB_TRX table.
   :column rseg_id: Rollback segment ID associated with this particular undo segment. More info on this rollback segment can be found by matching the rseg_id with that in the INFORMATION_SCHEMA.INNODB_RSEG.
   :column useg_id: Undo segment ID
   :column type: Segment type - identifies what type of operation the segments is allocated for.
   :column state: Segment state 
   :column size: Segment size in pages
   
States of an undo log segment:
 * ACTIVE - contains an undo log of an active transaction
 * CACHED - cached for quick reuse
 * TO_FREE - insert undo segment can be freed
 * TO_PURGE - update undo segment will not be reused; it can be freed in purge when all undo data in it is removed
 * PREPARED - contains an undo log of a prepared transaction
