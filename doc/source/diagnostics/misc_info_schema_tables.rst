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
 
   :version 8.0.12-1: Feature ported from |Percona Server| 5.7
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

This table holds information on the temporary tables that exist for all connections. You don't need the ``SUPER`` privilege to query this table.

.. table:: INFORMATION_SCHEMA.TEMPORARY_TABLES

   :version 8.0.12-1: Feature ported from |Percona Server| 5.7
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
