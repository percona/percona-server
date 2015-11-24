.. _innodb_stats:

=====================
 |InnoDB| Statistics
=====================

This feature provides new startup options (control method and collection of index statistics estimation) and information schema views to confirm the statistics.

Version Specific Information
============================

  * :rn:`5.5.8-20.0`:
    Renamed three fields in :table:`INNODB_INDEX_STATS` table.


System Variables
================

Four new system variables were introduced by this feature.

.. variable:: innodb_stats_method

   :cli: YES
   :configfile: YES
   :scope: GLOBAL
   :dyn: YES
   :type: STRING
   :default: ``nulls_equal``
   :allowed: ``nulls_equal``, ``nulls_unequal``, ``nulls_ignored``

The values and meanings are almost same to ``myisam_stats_method`` option of native |MySQL| (``nulls_equal``, ``nulls_unequal``, ``nulls_ignored``). But |InnoDB| doesn't have several patterns of statistics currently. Even though this option can be changed dynamically, statistics needs to be re-calculated to change the method for the table.

(reference: `MyISAM Index Statistics Collection <http://dev.mysql.com/doc/refman/5.5/en/myisam-index-statistics.html>`_)

.. variable:: innodb_stats_auto_update

   :cli: Yes
   :configfile: Yes
   :scope: Global
   :dyn: Yes
   :type: BOOLEAN
   :default: 1

|InnoDB| updates the each index statistics automatically (many updates were done, some information_schema is accessed, table monitor, etc.). Setting this option 0 can stop these automatic recalculation of the statistics except for "first open" and "ANALYZE TABLE command".


.. variable:: innodb_stats_update_need_lock

   :cli: Yes
   :configfile: Yes
   :scope: Global
   :dyn: Yes
   :type: BOOLEAN
   :default: 1

If you meet contention of ``&dict_operation_lock``, setting 0 reduces the contention. But 0 disables to update ``Data_free:`` of ``SHOW TABLE STATUS``.


.. variable:: innodb_use_sys_stats_table

   :cli: Yes
   :configfile: Yes
   :scope: Global
   :dyn: No
   :type: BOOLEAN
   :default: 0

If this option is enabled, |XtraDB| uses the ``SYS_STATS`` system table to store statistics of table indexes. Also, when |InnoDB| opens a table for the first time, it loads the statistics from ``SYS_STATS`` instead of sampling index pages. If you use a high ``stats_sample_pages`` value, the first open of a table is expensive. In such a case, this option will help. Intended behavior is to never update statistics unless an explicit ``ANALYZE TABLE`` is issued.

INFORMATION_SCHEMA Tables
=========================

.. table:: INFORMATION_SCHEMA.INNODB_SYS_STATS

   Shows statistics of table indexes.
   
   :column INDEX_ID: Index ID
   :column KEY_COLS: Number of key columns
   :column DIFF_VALS: Number of Different Values
   :column NON_NULL_VALS: Number of Non ``NULL`` Values

.. table:: INFORMATION_SCHEMA.INNODB_SYS_TABLES

   Shows the information about |InnoDB| tables

   :column TABLE_ID: Table ID
   :column SCHEMA: Database (schema) name
   :column NAME: Table name
   :column FLAG: Contains `0` if it is a InnoDB system table or `1` it is a user table
   :column N_COLS: Number of columns in the table
   :column SPACE: Tablespace ID

.. table:: INFORMATION_SCHEMA.INNODB_SYS_TABLESTATS

   Shows the information about the performance statistics of |InnoDB| tables.

   :column TABLE_ID: Table ID
   :column SCHEMA: Database (schema) Name
   :column NAME: Table Name
   :column STATS_INITIALIZED: Contains ``Initialized`` value if the statistics are collected or ``Uninitialized`` if they are not collected.
   :column NUM_ROWS: Estimated number of rows in the table.
   :column CLUST_INDEX_SIZE: Number of pages on disk that store the clustered index. 
   :column OTHER_INDEX_SIZE: Number of pages on disk that store all secondary indexes. 
   :column MODIFIED_COUNTER: Number of rows modified by DML operations.
   :column AUTOINC: 
   :column MYSQL_HANDLES_OPENED:

.. table:: INFORMATION_SCHEMA.INNODB_SYS_INDEXES

   Shows the information about |InnoDB| indexes

   :column INDEX_ID: Index ID
   :column NAME: Index Name
   :column TABLE_ID: Table ID
   :column TYPE: Numeric identifier signifying the index type
   :column N_FIELDS: Number of columns in the index
   :column PAGE_NO: Page offset within its tablespace
   :column SPACE: Tablespace ID

.. table:: INFORMATION_SCHEMA.INNODB_SYS_COLUMNS

   Shows the information about the |InnoDB| table columns

   :column TABLE_ID: Table ID
   :column NAME: Column Name
   :column POS: Position of the column inside the table. 
   :column MTYPE: Numeric identifier for the column type.
   :column PRTYPE: Binary value with bits representing data type, character set code and nullability.
   :column LEN: Column length.

.. table:: INFORMATION_SCHEMA.INNODB_SYS_FIELDS

   Shows the information about the |InnoDB| index key fields.

   :column INDEX_ID: Index ID
   :column NAME: Index Name
   :column POS: Position of the field inside the index.

.. table:: INFORMATION_SCHEMA.INNODB_SYS_FOREIGN
 
   Shows the information about the |InnoDB| foreign keys.

   :column ID: Foreign Key ID
   :column FOR_NAME: Database/Table which contains the Foreign Key 
   :column FOR_REF: Database/Table being referenced by the Foreign Key
   :column N_COLS: Number of columns in the foreign key.
   :column TYPE: Type of foreign key, represented by the bit flags.

.. table:: INFORMATION_SCHEMA.INNODB_SYS_FOREIGN_COLS

   Shows the information about the columns of the |InnoDB| foreign keys.

   :column ID: Foreign Key ID
   :column FOR_COL_NAME: Foreign Key Column Name
   :column FOR_REF: Referenced Column Name
   :column POS: Position of the field inside the index.

.. table:: INFORMATION_SCHEMA.INNODB_TABLE_STATS

   Shows table statistics information of dictionary cached.

   :column table_schema: Database name of the table.
   :column table_name: Table name.
   :column rows: estimated number of all rows.
   :column clust_size: cluster index (table/primary key) size in number of pages.
   :column other_size: Other index (non primary key) size in number of pages.
   :column modified: Internal counter to judge whether statistics recalculation should be done.

If the value of modified column exceeds "rows / 16" or 2000000000, the statistics recalculation is done when ``innodb_stats_auto_update == 1``. We can estimate the oldness of the statistics by this value.

.. table:: INFORMATION_SCHEMA.INNODB_INDEX_STATS

   Shows index statistics information of dictionary cached.

   :column table_schema: Database name of the table.
   :column table_name: Table name.
   :column index_name: Index name.
   :column fields: How many fields the index key has. (it is internal structure of |InnoDB|, it may be larger than the ``CREATE TABLE``).
   :column rows_per_key: Estimate rows per 1 key value. ([1 column value], [2 columns value], [3 columns value], ...).
   :column index_total_pages: Number of index pages.
   :column index_leaf_pages: Number of leaf pages.

In releases before 5.5.8-20.0, these fields had different names:

  * ``rows_per_key`` was ``row_per_keys``

  * ``index_total_pages`` was ``index_size``

  * ``index_leaf_pages`` was ``leaf_pages``

Example
=======

``[innodb_stats_method = nulls_equal (default behavior of InnoDB)]`` ::

  mysql> explain SELECT COUNT(*), 0 FROM orgs2 orgs LEFT JOIN sa_opportunities2 sa_opportunities ON orgs.org_id=sa_opportunities.org_id LEFT JOIN contacts2 contacts ON orgs.org_id=contacts.org_id;
  +----+-------------+------------------+-------+-----------------+-----------------+---------+-------------------+-------+-------------+
  | id | select_type | table            | type  | possible_keys   | key             | key_len | ref               | rows  | Extra       |
  +----+-------------+------------------+-------+-----------------+-----------------+---------+-------------------+-------+-------------+
  |  1 | SIMPLE      | orgs             | index | NULL            | orgs$org_id     | 4       | NULL              |   128 | Using index |
  |  1 | SIMPLE      | sa_opportunities | ref   | sa_opp$org_id   | sa_opp$org_id   | 5       | test2.orgs.org_id |  5751 | Using index |
  |  1 | SIMPLE      | contacts         | ref   | contacts$org_id | contacts$org_id | 5       | test2.orgs.org_id | 23756 | Using index |
  +----+-------------+------------------+-------+-----------------+-----------------+---------+-------------------+-------+-------------+
  3 rows in set (0.00 sec)

``[innodb_stats_method = nulls_unequal or nulls_ignored]`` ::

  mysql> explain SELECT COUNT(*), 0 FROM orgs2 orgs LEFT JOIN sa_opportunities2 sa_opportunities ON orgs.org_id=sa_opportunities.org_id LEFT JOIN contacts2 contacts ON orgs.org_id=contacts.org_id;
  +----+-------------+------------------+-------+-----------------+-----------------+---------+-------------------+------+-------------+
  | id | select_type | table            | type  | possible_keys   | key             | key_len | ref               | rows | Extra       |
  +----+-------------+------------------+-------+-----------------+-----------------+---------+-------------------+------+-------------+
  |  1 | SIMPLE      | orgs             | index | NULL            | orgs$org_id     | 4       | NULL              |  128 | Using index |
  |  1 | SIMPLE      | sa_opportunities | ref   | sa_opp$org_id   | sa_opp$org_id   | 5       | test2.orgs.org_id |    1 | Using index |
  |  1 | SIMPLE      | contacts         | ref   | contacts$org_id | contacts$org_id | 5       | test2.orgs.org_id |    1 | Using index |
  +----+-------------+------------------+-------+-----------------+-----------------+---------+-------------------+------+-------------+
  3 rows in set (0.00 sec)
  <example of information_schema>

  mysql> select * from information_schema.innodb_table_stats;
  +------------------------+-------+------------+------------+----------+
  | table_name             | rows  | clust_size | other_size | modified |
  +------------------------+-------+------------+------------+----------+
  | test/sa_opportunities2 | 11175 |         21 |         11 |        0 |
  | test/orgs2             |   128 |          1 |          0 |        0 |
  | test/contacts2         | 47021 |         97 |         97 |        0 |
  +------------------------+-------+------------+------------+----------+
  3 rows in set (0.00 sec)

  mysql> select * from information_schema.innodb_index_stats;
  +------------------------+-----------------+--------+--------------+------------+------------+
  | table_name             | index_name      | fields | row_per_keys | index_size | leaf_pages |
  +------------------------+-----------------+--------+--------------+------------+------------+
  | test/sa_opportunities2 | GEN_CLUST_INDEX |      1 | 1            |         21 |         20 |
  | test/sa_opportunities2 | sa_opp$org_id   |      2 | 338, 1       |          11|         10 |
  | test/orgs2             | orgs$org_id     |      1 | 1            |          1 |          1 |
  | test/contacts2         | GEN_CLUST_INDEX |      1 | 1            |       97   |         80 |
  | test/contacts2         | contacts$org_id |      2 | 516, 0       |       97   |         37 |
  +------------------------+-----------------+--------+--------------+------------+------------+
  5 rows in set (0.00 sec)

Other reading
=============

  * `InnoDB Table/Index stats <http://www.mysqlperformanceblog.com/2010/03/20/InnoDB-tableindex-stats/>`_

