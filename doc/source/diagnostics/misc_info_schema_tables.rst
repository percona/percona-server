.. _misc_info_schema_tables:

=================================
 Misc. INFORMATION_SCHEMA Tables
=================================

This page lists the ``INFORMATION_SCHEMA`` tables added to standard |MySQL| by |Percona Server| that don't exist elsewhere in the documentation.

.. _temp_tables:

Temporary tables
================

.. note::

 This feature implementation is considered ALPHA quality.

Only the temporary tables that were explicitly created with `CREATE TEMPORARY TABLE` or `ALTER TABLE` are shown, and not the ones created to process complex queries.

.. table:: INFORMATION_SCHEMA.GLOBAL_TEMPORARY_TABLES
 
   :version 5.6.5-60.0: Feature introduced
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

   :version 5.6.5-60.0: Feature introduced
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

Multiple Rollback Segments
==========================

|Percona Server|, in addition to the upstream multiple rollback segment implementation, provides the additional Information Schema table: INFORMATION_SCHEMA.XTRADB_RSEG.

``INFORMATION_SCHEMA`` Tables
=============================

This feature provides the following table:

.. table:: INFORMATION_SCHEMA.XTRADB_RSEG

   :column rseg_id: rollback segment id
   :column space_id: space where the segment placed
   :column zip_size: compressed page size in bytes if compressed otherwise 0
   :column page_no: page number of the segment header
   :column max_size: max size in pages
   :column curr_size: current size in pages

This table shows information about all the rollback segments (the default segment and the extra segments).

Here is an example of output with ``innodb_extra_rsegments = 8`` ::

  mysql> select * from information_schema.innodb_rseg;
  +---------+----------+----------+---------+------------+-----------+
  | rseg_id | space_id | zip_size | page_no | max_size   | curr_size |
  +---------+----------+----------+---------+------------+-----------+
  |       0 |        0 |        0 |       6 | 4294967294 |         1 |
  |       1 |        0 |        0 |      13 | 4294967294 |         2 |
  |       2 |        0 |        0 |      14 | 4294967294 |         1 |
  |       3 |        0 |        0 |      15 | 4294967294 |         1 |
  |       4 |        0 |        0 |      16 | 4294967294 |         1 |
  |       5 |        0 |        0 |      17 | 4294967294 |         1 |
  |       6 |        0 |        0 |      18 | 4294967294 |         1 |
  |       7 |        0 |        0 |      19 | 4294967294 |         1 |
  |       8 |        0 |        0 |      20 | 4294967294 |         1 |
  +---------+----------+----------+---------+------------+-----------+
  9 rows in set (0.00 sec)

