
.. _query-limit-estimates:

=========================================================
Limiting the Estimation of Records in a Query
=========================================================

:Availability:  This feature is **technical preview** quality.

This page describes an alternative when running queries against a large number of table partitions. When a query runs, InnoDB estimates the records in each partition. This process can result in more pages read and more disk I/O, if the buffer pool must fetch the pages from disk. This process increases the query time if there are a large number of partitions.

The addition of two variables make it possible to override `records_in_range <https://dev.mysql.com/doc/internals/en/records-in-range.html>`__ which effectively bypasses the process.

.. warning::

    The use of these variables may result in improper index selection by the optimizer.

.. variable:: innodb_records_in_range

    :cli: ``--innodb-records-in-range``
    :dyn: Yes
    :scope: Global
    :vartype: Numeric
    :default: 0

The variable provides a method to limit the number of records estimated for a query.

.. code-block:: MySQL

    mysql> SET @@GLOBAL.innodb_records_in_range=100;
    100


.. variable:: innodb_force_index_records_in_range

    :cli: ``--innodb-force-index-records-in-range``
    :dyn: Yes
    :scope: Global
    :vartype: Numeric
    :default: 0

This variable provides a method to override the `records_in_range` result when a FORCE INDEX is used in a query.

.. code-block:: MySQL

    mysql> SET @@GLOBAL.innodb_force_index_records_in_range=100;
    100

:variable: favor_range_scan

    :cli: ``--favor-range-scan``
    :dyn: Yes
    :scope: Session
    :vartype: String
    :default: OFF

In specific scenarios, the optimizer chooses to scan a table instead of using a range scan. The conditions are the following:

* Table with an extremely large number of rows

* Compound primary keys made of two or more columns

* WHERE clause contains multiple range conditions

The `favor_range_scan` arbitrarily lowers the cost of a range scan by a factor of 10. 

The available values are:

* ON

* OFF (Default)

* DEFAULT

::

    mysql> SET favor_range_scan = OFF;


