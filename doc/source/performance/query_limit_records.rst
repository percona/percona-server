.. _query-limit-estimates:

=========================================================
Limiting the Estimation of Records in a Query
=========================================================

:Availability:  The feature is **technical preview** quality.

This page describes an alternative when running queries against a large number
of table partitions. When a query runs, InnoDB estimates the records in each
partition. This process can result in more pages read and more disk I/O, if the
buffer pool must fetch the pages from disk. This process increases the query
time if there are a large number of partitions.

The addition of two variables makes it possible to override `records_in_range
<https://dev.mysql.com/doc/internals/en/records-in-range.html>`__ which
effectively bypasses the process.

.. warning::

    The use of these variables may result in improper index selection by the
    optimizer.

.. _innodb_records_in_range:

.. rubric:: ``innodb_records_in_range``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--innodb-records-in-range``
   * - Scope
     - Global
   * - Dynamic
     - Yes
   * - Data type
     - Numeric
   * - Default
     - 0

:Availability:  The feature is **technical preview** quality.

The variable provides a method to limit the number of records estimated for a
query.

.. code-block:: MySQL

    mysql> SET @@GLOBAL.innodb_records_in_range=100;
    100

.. _innodb_force_index_records_in_range:

.. rubric:: ``innodb_force_index_records_in_range``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - ``--innodb-force-index-records-in-range``
   * - Scope
     - Global
   * - Dynamic
     - Yes
   * - Data type
     - Numeric
   * - Default
     - 0

:Availability:  The feature is **technical preview** quality.

This variable provides a method to override the `records_in_range` result when a
FORCE INDEX is used in a query.

.. code-block:: MySQL

    mysql> SET @@GLOBAL.innodb_force_index_records_in_range=100;
    100

.. _favor_range_scan:

Using the favor_range_scan optimizer switch
--------------------------------------------

:Availability:  The feature is **technical preview** quality.

In specific scenarios, the optimizer chooses to scan a table instead of using a range scan. The conditions are the following:

* Table with an extremely large number of rows

* Compound primary keys made of two or more columns

* WHERE clause contains multiple range conditions

The `optimizer_switch <https://dev.mysql.com/doc/refman/8.0/en/switchable-optimizations.html>`__ controls the optimizer behavior. The `favor_range_scan` switch arbitrarily lowers the cost of a range scan by a factor of 10.

The available values are:

* ON

* OFF (Default)

* DEFAULT

::

    mysql> SET optimizer_switch='favor_range_scan=on';
