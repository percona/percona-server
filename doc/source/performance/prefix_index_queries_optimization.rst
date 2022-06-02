.. _prefix_index_queries_optimization:

=================================
Prefix Index Queries Optimization
=================================

*Percona Server for MySQL* 5.6 has ported Prefix Index Queries Optimization feature from
Facebook patch for MySQL.

Prior to this InnoDB would always fetch the clustered index for all prefix
columns in an index, even when the value of a particular record was smaller
than the prefix length. This implementation optimizes that case to use the
record from the secondary index and avoid the extra lookup.

Status Variables
================

.. _Innodb_secondary_index_triggered_cluster_reads:

.. rubric:: ``Innodb_secondary_index_triggered_cluster_reads``

Implemented in :ref:`5.7.18-14`.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Data type
     - Numeric
   * - Scope
     - Global

This variable shows the number of times secondary index lookup triggered
cluster lookup.

.. _Innodb_secondary_index_triggered_cluster_reads_avoided:

.. rubric:: ``Innodb_secondary_index_triggered_cluster_reads_avoided``

Implemented in :ref:`5.7.18-14`.

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Data type
     - Numeric
   * - Scope
     - Global

This variable shows the number of times prefix optimization avoided
triggering cluster lookup.
