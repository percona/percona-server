.. _misc_info_schema_tables:

=================================
 Misc. INFORMATION_SCHEMA Tables
=================================

This page lists the ``INFORMATION_SCHEMA`` tables added to standard *MySQL* by *Percona Server for MySQL* that don't exist elsewhere in the documentation.

.. _temp_tables:

Temporary tables
================

.. note::

 This feature implementation is considered ALPHA quality.

Only the temporary tables that were explicitly created with `CREATE TEMPORARY TABLE` or `ALTER TABLE` are shown, and not the ones created to process complex queries.

.. _GLOBAL_TEMPORARY_TABLES:

.. rubric:: ``INFORMATION_SCHEMA.GLOBAL_TEMPORARY_TABLES``

.. list-table::
      :header-rows: 1

      * - Column Name
        - Description
      * - 'SESSION_ID'
        - '*MySQL* connection id'
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

The feature was ported from *Percona Server for MySQL* 5.7 in :ref:`8.0.12-1`.

This table holds information on the temporary tables that exist for all connections. You don't need the ``SUPER`` privilege to query this table.

.. _TEMPORARY_TABLES:

.. rubric:: ``INFORMATION_SCHEMA.TEMPORARY_TABLES``

.. list-table::
      :header-rows: 1

      * - Column Name
        - Description
      * - 'SESSION_ID'
        - '*MySQL* connection id'
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

The feature was ported from *Percona Server for MySQL* 5.7 in :ref:`8.0.12-1`.

This table holds information on the temporary tables existing for the running connection.
