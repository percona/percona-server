===============================
Misc. INFORMATION_SCHEMA Tables
===============================

Temporary tables
================

 Only temporary tables that were explicitly created with `CREATE TEMPORARY TABLE` are shown, and not the ones created during query execution. The temporary tables that are created for `ALTER TABLE` execution are not listed in `INFORMATION_SCHEMA.TEMPORARY_TABLES` or `GLOBAL_TEMPORARY_TABLES` tables.

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

Buffer Pool Data Structure Tables
=================================

The following tables provide various information about the contents of the |InnoDB| buffer pool.

.. table:: INFORMATION_SCHEMA.INNODB_BUFFER_POOL_PAGES

  :column PAGE_TYPE: Type of the page. Possible values: index, undo_log, inode, ibuf_free_list, allocated, bitmap, sys, trx_sys, fsp_hdr, xdes, blob, zblob, zblob2, unknown
  :column SPACE_ID: tablespace ID
  :column PAGE_NO: page offset within its tablespace
  :column LRU_POSITION: page position in the LRU list
  :column FIX_COUNT: reference count of a page. It is incremented every time the page is accessed by InnoDB, it is 0 if and only if the page is not currently being accessed.
  :column FLUSH_TYPE: type of the last flush of the page (0:LRU 2:flush_list)

Example: ::

  mysql> select * from information_schema.INNODB_BUFFER_POOL_PAGES LIMIT 20;  
  +-----------+----------+---------+--------------+-----------+------------+
  | page_type | space_id | page_no | lru_position | fix_count | flush_type |
  +-----------+----------+---------+--------------+-----------+------------+
  | allocated |        0 |       7 |            3 |         0 |          2 | 
  | allocated |        0 |       1 |            4 |         0 |          0 | 
  | allocated |        0 |       3 |            5 |         0 |          0 | 
  | inode     |        0 |       2 |            6 |         0 |          2 | 
  | index     |        0 |       4 |            7 |         0 |          2 | 
  | index     |        0 |      11 |            8 |         0 |          0 | 
  | index     |        0 |   12956 |            9 |         0 |          0 | 
  | allocated |        0 |       5 |           10 |         0 |          2 | 
  | allocated |        0 |       6 |           11 |         0 |          2 | 
  | undo_log  |        0 |      51 |           12 |         0 |          2 | 
  | undo_log  |        0 |      52 |           13 |         0 |          2 | 
  | index     |        0 |       8 |           14 |         0 |          0 | 
  | index     |        0 |     288 |           15 |         0 |          0 | 
  | index     |        0 |     290 |           16 |         0 |          2 | 
  | index     |        0 |     304 |           17 |         0 |          0 | 
  | allocated |        0 |       0 |           18 |         0 |          2 | 
  | index     |        0 |      10 |           19 |         0 |          0 | 
  | index     |        0 |   12973 |           20 |         0 |          0 | 
  | index     |        0 |       9 |           21 |         0 |          2 | 
  | index     |        0 |      12 |           22 |         0 |          0 | 
  +-----------+----------+---------+--------------+-----------+------------+
  20 rows in set (0.81 sec)

This table shows the characteristics of the allocated pages in buffer pool and current state of them.

.. table:: INFORMATION_SCHEMA.INNODB_BUFFER_POOL_PAGES_INDEX

  :column index_id: index name
  :column space_id: tablespace ID
  :column page_no: page offset within its tablespace
  :column n_recs: number of user records on page
  :column data_size: sum of the sizes of the records in page
  :column hashed: the block is in adaptive hash index (1) or not (0)
  :column access_time: time of the last access to this page.
  :column modified: modified since loaded (1) or not (0)
  :column dirty: modified since last flushed (1) or not (0)
  :column old: is old blocks in the LRU list (1) or not (0)
  :column lru_position: page position in the LRU list
  :column fix_count: reference count of a page. It is incremented every time the page is accessed by InnoDB, it is 0 if and only if the page is not currently being accessed.
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

  :column space_id:	tablespace id
  :column page_no: page offset within its tablespace
  :column compressed: contains compressed data (1) or not (0)
  :column part_len: data length in the page
  :column next_page_no: page number of the next data
  :column lru_position: page position in the LRU list
  :column fix_count: reference count of a page. It is incremented every time the page is accessed by InnoDB, it is 0 if and only if the page is not currently being accessed.
  :column flush_type: type of the last flush of the page (0:LRU 2:flush_list)

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

