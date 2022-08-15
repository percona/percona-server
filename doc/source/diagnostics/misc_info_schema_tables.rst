.. _misc_info_schema_tables:

=================================
 Misc. INFORMATION_SCHEMA Tables
=================================

This page lists the ``INFORMATION_SCHEMA`` tables added to standard MySQL by *Percona Server for MySQL* that don't exist elsewhere in the documentation.

.. _temp_tables:

Temporary tables
================

.. note::

 This feature implementation is considered ALPHA quality.

Only the temporary tables that were explicitly created with `CREATE TEMPORARY TABLE` or `ALTER TABLE` are shown, and not the ones created to process complex queries.

.. _GLOBAL_TEMPORARY_TABLES:

.. rubric:: ``INFORMATION_SCHEMA.GLOBAL_TEMPORARY_TABLES``

:ref:`5.7.10-1` - Feature ported from *Percona Server for MySQL* 5.6

.. list-table::
      :header-rows: 1

      * - Column Name
        - Description
      * - 'SESSION_ID'
        - 'MySQL connection id'
      * - 'TABLE_SCHEMA'
        - 'Schema in which the temporary table is created'
      * - 'TABLE_NAME'
        - 'Name of the temporary table'
      * - 'ENGINE'
        - 'Engine of the temporary table'
      * - 'NAME'
        - 'Internal name of the temporary table'
      * - 'TABLE_ROWS'
        - 'Number of rows of the temporary table'
      * - 'AVG_ROW_LENGTH'
        - 'Average row length of the temporary table'
      * - 'DATA_LENGTH'
        - 'Size of the data (Bytes)'
      * - 'INDEX_LENGTH'
        - 'Size of the indexes (Bytes)'
      * - 'CREATE_TIME'
        - 'Date and time of creation of the temporary table'
      * - 'UPDATE_TIME'
        - 'Date and time of the latest update of the temporary table'

This table holds information on the temporary tables existing for all connections. You don't need the ``SUPER`` privilege to query this table.

.. _TEMPORARY_TABLES:

.. rubric:: ``INFORMATION_SCHEMA.TEMPORARY_TABLES``

:ref:`5.7.10-1` - Feature ported from *Percona Server for MySQL* 5.6

.. list-table::
      :header-rows: 1

      * - Column Name
        - Description
      * - 'SESSION_ID'
        - 'MySQL connection id'
      * - 'TABLE_SCHEMA'
        - 'Schema in which the temporary table is created'
      * - 'TABLE_NAME'
        - 'Name of the temporary table'
      * - 'ENGINE'
        - 'Engine of the temporary table'
      * - 'NAME'
        - 'Internal name of the temporary table'
      * - 'TABLE_ROWS'
        - 'Number of rows of the temporary table'
      * - 'AVG_ROW_LENGTH'
        - 'Average row length of the temporary table'
      * - 'DATA_LENGTH'
        - 'Size of the data (Bytes)'
      * - 'INDEX_LENGTH'
        - 'Size of the indexes (Bytes)'
      * - 'CREATE_TIME'
        - 'Date and time of creation of the temporary table'
      * - 'UPDATE_TIME'
        - 'Date and time of the latest update of the temporary table'

This table holds information on the temporary tables existing for the running connection.

Multiple Rollback Segments
==========================

*Percona Server for MySQL*, in addition to the upstream multiple rollback segment implementation, provides the additional Information Schema table: ``INFORMATION_SCHEMA.XTRADB_RSEG``.

``INFORMATION_SCHEMA`` Tables
=============================

This feature provides the following table:

.. _XTRADB_RSEG:

.. rubric:: ``INFORMATION_SCHEMA.XTRADB_RSEG``

.. list-table::
      :header-rows: 1

      * - Column Name
        - Description
      * - 'rseg_id'
        - 'rollback segment id'
      * - 'space_id'
        - 'space where the segment placed'
      * - 'physical_page_size'
        - 'physical page size'
      * - 'logical_page_size'
        - 'logical page size'
      * - 'is_compressed'
        - 'is the page compressed'
      * - 'page_no'
        - 'page number of the segment header'
      * - 'max_size'
        - 'max size in pages'
      * - 'curr_size'
        - 'current size in pages'

This table shows information about all the rollback segments (the default segment and the extra segments).

Here is an example of output with ``innodb_rollback_segments = 8``:

.. code-block:: mysql

  mysql> SELECT * FROM INFORMATION_SCHEMA.XTRADB_RSEG;
  +---------+----------+--------------------+-------------------+---------------+---------+------------+-----------+
  | rseg_id | space_id | physical_page_size | logical_page_size | is_compressed | page_no | max_size   | curr_size |
  +---------+----------+--------------------+-------------------+---------------+---------+------------+-----------+
  |       0 |        0 |              16384 |             16384 |             0 |       6 | 4294967294 |         2 |
  |       1 |       24 |              16384 |             16384 |             0 |       3 | 4294967294 |         1 |
  |       2 |       24 |              16384 |             16384 |             0 |       4 | 4294967294 |         1 |
  |       3 |       24 |              16384 |             16384 |             0 |       5 | 4294967294 |         1 |
  |       4 |       24 |              16384 |             16384 |             0 |       6 | 4294967294 |         1 |
  |       5 |       24 |              16384 |             16384 |             0 |       7 | 4294967294 |         1 |
  |       6 |       24 |              16384 |             16384 |             0 |       8 | 4294967294 |         1 |
  |       7 |       24 |              16384 |             16384 |             0 |       9 | 4294967294 |         1 |
  |       8 |       24 |              16384 |             16384 |             0 |      10 | 4294967294 |         1 |
  +---------+----------+--------------------+-------------------+---------------+---------+------------+-----------+
  9 rows in set (0.00 sec)

