.. _innodb_adaptive_hash_index_partitions_page:

==========================================
 Multiple Adaptive Hash Search Partitions
==========================================

The |InnoDB| adaptive hash index can have contention issues on multi-core systems when you run a mix of read and write queries that need to scan secondary indexes. This feature splits the adaptive hash index across several partitions to avoid such problems.

The number of adaptive hash partitions specified by the variable :variable:`innodb_adaptive_hash_index_partitions` are created, and hash indexes are assigned to each one based on ``index_id``. This should help to solve contention problems in the adaptive hash search process when they occur.


Version Specific Information
============================
 * :rn:`5.6.13-60.6` - Feature ported from |Percona Server| 5.5

System Variables
----------------

.. variable:: innodb_adaptive_hash_index_partitions

   :version 5.6.13-60.6: Ported from |Percona Server| 5.5
   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: No
   :vartype: Numeric
   :def: 1
   :range: 1-64, (on 32-bit platform 1-32)

Specifies the number of partitions to use in the adaptive hash search process.

When set to one, no extra partitions are created and the normal process is in effect. When greater than one, the specified number of partitions are created across which to perform the adaptive search.

Other reading
-------------

  * `Index lock and adaptive search <http://www.mysqlperformanceblog.com/2010/02/25/index-lock-and-adaptive-search-next-two-biggest-innodb-problems/>`_

  * |MySQL| bug :mysqlbug:`62018`
