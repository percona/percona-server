.. _innodb_stats:

=====================
 |InnoDB| Statistics
=====================

This feature provides new startup options (control method and collection of index statistics estimation) and information schema views to confirm the statistics.

This implements the fix for `MySQL Bug #30423 <http://bugs.|MySQL|.com/bug.php?id=30423>`_.

Version Specific Information
============================

  * 5.5.8-20.0:
    Renamed three fields in table ``INNODB_INDEX_STATS``.


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

The values and meanings are almost same to ``myisam_stats_method`` option of native |MySQL| (``nulls_equal``, ``nulls_unequal``, ``nulls_ignored``). But |InnoDB| doesn't have several patterns of statistics currently. So, though this option able to be changed dynamically, we need re-calculate statistics to change the method for the table.

(reference: `MyISAM Index Statistics Collection <http://dev.mysql.com/doc/refman/5.1/en/myisam-index-statistics.html>`_)

**Note:** Beginning in release 5.1.56-12.7, a variable with the same and functionality was implemented in the upstream |InnoDB|.

.. variable:: innodb_stats_auto_update

   :type: BOOLEAN
   :default: 1

|InnoDB| updates the each index statistics automatically (many updates were done, some information_schema is accessed, table monitor, etc.). Setting this option 0 can stop these automatic recalculation of the statistics except for “first open” and “ANALYZE TABLE command”.


.. variable:: innodb_stats_update_need_lock

   :type: BOOLEAN
   :default: 1

If you meet contention of ``&dict_operation_lock``, setting 0 reduces the contention. But 0 disables to update ``Data_free:`` of ``SHOW TABLE STATUS``.


.. variable:: innodb_use_sys_stats_table

   :type: BOOLEAN
   :default: 0


If this option is enabled, |XtraDB| uses the ``SYS_STATS`` system table to store statistics of table indexes. Also, when |InnoDB| opens a table for the first time, it loads the statistics from ``SYS_STATS`` instead of sampling index pages. If you use a high ``stats_sample_pages`` value, the first open of a table is expensive. In such a case, this option will help. Note: This option may cause less frequent updating of statistics. So, you should intentionally use the ``ANALYZE TABLE`` command more often.

(This variable was introduced in release 5.1.50-11.4.)


INFORMATION_SCHEMA Tables
=========================

Two new tables were introduced by this feature.

.. table:: INFORMATION_SCHEMA.INNODB_TABLE_STATS

   Shows table statistics information of dictionary cached.

   :column table_schema: Database name of the table.
   :column table_name: Table name.
   :column rows: estimated number of all rows.
   :column clust_size: cluster index (table/primary key) size in number of pages.
   :column other_size: Other index (non primary key) size in number of pages.
   :column modified: Internal counter to judge whether statistics recalculation should be done.

If the value of modified column exceeds “rows / 16” or 2000000000, the statistics recalculation is done when ``innodb_stats_auto_update == 1``. We can estimate the oldness of the statistics by this value.

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

This example uses the same data to Bug #30423 of |MySQL|.

``[innodb_stats_method = nulls_equal (default behavior of |InnoDB|)]`` ::

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
  | test/sa_opportunities2 | sa_opp$org_id   |      2 | 338, 1       |         11|         10 |
  | test/orgs2             | orgs$org_id     |      1 | 1            |          1 |          1 |
  | test/contacts2         | GEN_CLUST_INDEX |      1 | 1            |       97   |         80 |
  | test/contacts2         | contacts$org_id |      2 | 516, 0       |         97   |         37 |
  +------------------------+-----------------+--------+--------------+------------+------------+
  5 rows in set (0.00 sec)

Other reading
=============

  * `InnoDB Table/Index stats <http://www.|MySQL|performanceblog.com/2010/03/20/|InnoDB|-tableindex-stats/>`_

