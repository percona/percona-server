.. _response_time_distribution:

============================
 Response Time Distribution
============================

The slow query log provides exact information about queries that take a long time to execute. However, sometimes there are a large number of queries that each take a very short amount of time to execute. This feature provides a tool for analyzing that information by counting and displaying the number of queries according to the length of time they took to execute. The user can define time intervals that divide the range 0 to positive infinity into smaller intervals and then collect the number of commands whose execution times fall into each of those intervals.

Note that in a replication environment, the server will not take into account *any* queries executed by the slave SQL threads (whether they are slow or not) for the time distribution. 

Each interval is described as: ::

(range_base ^ n; range_base ^ (n+1)]

The range_base is some positive number (see Limitations). The interval is defined as the difference between two nearby powers of the range base.

For example, if the range base=10, we have the following intervals: ::

  (0; 10 ^ -6], (10 ^ -6; 10 ^ -5], (10 ^ -5; 10 ^ -4], ..., (10 ^ -1; 10 ^1], (10^1; 10^2]...(10^7; positive infinity]

or ::

  (0; 0.000001], (0.000001; 0.000010], (0.000010; 0.000100], ..., (0.100000; 1.0]; (1.0; 10.0]...(1000000; positive infinity]

For each interval, a count is made of the queries with execution times that fell into that interval.

You can select the range of the intervals by changing the range base. For example, for base range=2 we have the following intervals: ::

  (0; 2 ^ -19], (2 ^ -19; 2 ^ -18], (2 ^ -18; 2 ^ -17], ..., (2 ^ -1; 2 ^1], (2 ^ 1; 2 ^ 2]...(2 ^ 25; positive infinity]

or ::

  (0; 0.000001], (0.000001, 0.000003], ..., (0.25; 0.5], (0.5; 2], (2; 4]...(8388608; positive infinity]

Small numbers look strange (i.e., don't look like powers of 2), because we lose precision on division when the ranges are calculated at runtime. In the resulting table, you look at the high boundary of the range.

For example, you may see: ::

  +----------------+-------+------------+
  |      time      | count |    total   |
  +----------------+-------+------------|
  |       0.000001 |     0 |   0.000000 |
  |       0.000010 |    17 |   0.000094 |
  |       0.000100 |  4301 |   0.236555 |
  |       0.001000 |  1499 |   0.824450 |
  |       0.010000 | 14851 |  81.680502 |
  |       0.100000 |  8066 | 443.635693 |
  |       1.000000 |     0 |   0.000000 |
  |      10.000000 |     0 |   0.000000 |
  |     100.000000 |     1 |  55.937094 |
  |    1000.000000 |     0 |   0.000000 |
  |   10000.000000 |     0 |   0.000000 |
  |  100000.000000 |     0 |   0.000000 |
  | 1000000.000000 |     0 |   0.000000 |
  | TOO LONG QUERY |     0 |   0.000000 |
  +----------------+-------+------------+

This means there were: ::

  * 17 queries with 0.000001 < query execution time < = 0.000010 seconds; total execution time of the 17 queries = 0.000094 seconds

  * 4301 queries with 0.000010 < query execution time < = 0.000100 seconds; total execution time of the 4301 queries = 0.236555 seconds

  * 1499 queries with 0.000100 < query execution time < = 0.001000 seconds; total execution time of the 1499 queries = 0.824450 seconds

  * 14851 queries with 0.001000 < query execution time < = 0.010000 seconds; total execution time of the 14851 queries = 81.680502 seconds

  * 8066 queries with 0.010000 < query execution time < = 0.100000 seconds; total execution time of the 8066 queries = 443.635693 seconds

  * 1 query with 10.000000 < query execution time < = 100.0000 seconds; total execution time of the 1 query = 55.937094 seconds

.. _rtd_rw_split:

Logging the queries in separate ``READ`` and ``WRITE`` tables
=============================================================

|Percona Server| is now able to log the queries response times into separate ``READ`` and ``WRITE`` ``INFORMATION_SCHEMA`` tables. The two new tables are named :table:`QUERY_RESPONSE_TIME_READ` and :table:`QUERY_RESPONSE_TIME_WRITE` respectively. The decision on whether a query is a ``read`` or a ``write`` is based on the type of the command. Thus, for example, an ``UPDATE ... WHERE <condition>`` is always logged as a ``write`` query even if ``<condition>`` is always false and thus no actual writes happen during its execution.

Following SQL commands will be considered as ``WRITE`` queries and will be logged into the :table:`QUERY_RESPONSE_TIME_WRITE` table: ``CREATE_TABLE``, ``CREATE_INDEX``, ``ALTER_TABLE``, ``TRUNCATE``, ``DROP_TABLE``, ``LOAD``, ``CREATE_DB``, ``DROP_DB``, ``ALTER_DB``, ``RENAME_TABLE``, ``DROP_INDEX``, ``CREATE_VIEW``, ``DROP_VIEW``, ``CREATE_TRIGGER``, ``DROP_TRIGGER``, ``CREATE_EVENT``, ``ALTER_EVENT``, ``DROP_EVENT``, ``UPDATE``, ``UPDATE_MULTI``, ``INSERT``, ``INSERT_SELECT``, ``DELETE``, ``DELETE_MULTI``, ``REPLACE``, ``REPLACE_SELECT``, ``CREATE_USER``, ``RENAME_USER``, ``DROP_USER``, ``ALTER_USER``, ``GRANT``, ``REVOKE``, ``REVOKE_ALL``, ``OPTIMIZE``, ``CREATE_FUNCTION``, ``CREATE_PROCEDURE``, ``CREATE_SPFUNCTION``, ``DROP_PROCEDURE``, ``DROP_FUNCTION``, ``ALTER_PROCEDURE``, ``ALTER_FUNCTION``, ``INSTALL_PLUGIN``, and ``UNINSTALL_PLUGIN``. Commands not listed here are considered as ``READ`` queries and will be logged into the :table:`QUERY_RESPONSE_TIME_READ` table.

Installing the plugins
======================

In order to enable this feature you'll need to install the necessary plugins:

.. code-block:: mysql

   mysql> INSTALL PLUGIN QUERY_RESPONSE_TIME_AUDIT SONAME 'query_response_time.so';

This plugin is used for gathering statistics.

.. code-block:: mysql

   mysql> INSTALL PLUGIN QUERY_RESPONSE_TIME SONAME 'query_response_time.so';

This plugin provides the interface (:table:`QUERY_RESPONSE_TIME`) to output gathered statistics.

.. code-block:: mysql

   mysql> INSTALL PLUGIN QUERY_RESPONSE_TIME_READ SONAME 'query_response_time.so';

This plugin provides the interface (:table:`QUERY_RESPONSE_TIME_READ`) to output gathered statistics.

.. code-block:: mysql

   mysql> INSTALL PLUGIN QUERY_RESPONSE_TIME_WRITE SONAME 'query_response_time.so';

This plugin provides the interface (:table:`QUERY_RESPONSE_TIME_WRITE`) to output gathered statistics. 

You can check if plugins are installed correctly by running:

.. code-block:: mysql

   mysql> SHOW PLUGINS;

   ...
   | QUERY_RESPONSE_TIME         | ACTIVE   | INFORMATION SCHEMA | query_response_time.so | GPL     |
   | QUERY_RESPONSE_TIME_AUDIT   | ACTIVE   | AUDIT              | query_response_time.so | GPL     |
   | QUERY_RESPONSE_TIME_READ    | ACTIVE   | INFORMATION SCHEMA | query_response_time.so | GPL     |
   | QUERY_RESPONSE_TIME_WRITE   | ACTIVE   | INFORMATION SCHEMA | query_response_time.so | GPL     |
   +-----------------------------+----------+--------------------+------------------------+---------+

Usage
=====

To start collecting query time metrics, :variable:`query_response_time_stats` should be enabled:

.. code-block:: mysql

  SET GLOBAL query_response_time_stats = on;

And to make it persistent, add the same to :file:`my.cnf`:

.. code-block:: none

  [mysqld]
  query_response_time_stats = on


SELECT
------

You can get the distribution using the query:

.. code-block:: mysql

  mysql> SELECT * from INFORMATION_SCHEMA.QUERY_RESPONSE_TIME
  time	                 count	 total
  0.000001	         0	 0.000000
  0.000010	         0	 0.000000
  0.000100	         1	 0.000072
  0.001000	         0	 0.000000
  0.010000	         0	 0.000000
  0.100000	         0	 0.000000
  1.000000	         0	 0.000000
  10.000000	         8	 47.268416
  100.000000	         0	 0.000000
  1000.000000	         0	 0.000000
  10000.000000	         0	 0.000000
  100000.000000	         0	 0.000000
  1000000.000000	 0	 0.000000
  TOO LONG QUERY	 0	 0.000000

You can write a complex query like: 

.. code-block:: mysql

  SELECT c.count, c.time,
  (SELECT SUM(a.count) FROM INFORMATION_SCHEMA.QUERY_RESPONSE_TIME as a WHERE a.count != 0) as query_count,
  (SELECT COUNT(*)     FROM INFORMATION_SCHEMA.QUERY_RESPONSE_TIME as b WHERE b.count != 0) as not_zero_region_count,
  (SELECT COUNT(*)     FROM INFORMATION_SCHEMA.QUERY_RESPONSE_TIME) as region_count
  FROM INFORMATION_SCHEMA.QUERY_RESPONSE_TIME as c WHERE c.count > 0;

**Note:** If :variable:`query_response_time_stats` is ON, the execution times for these two ``SELECT`` queries will also be collected.

FLUSH
-----

Flushing can be done by setting the :variable:`query_response_time_flush` to ``ON`` (or ``1``): 

.. code-block:: mysql

  mysql> SET GLOBAL query_response_time_flush='ON';

``FLUSH`` does two things:

  * Clears the collected times from the :table:`QUERY_RESPONSE_TIME`, :table:`QUERY_RESPONSE_TIME_READ`, and :table:`QUERY_RESPONSE_TIME_WRITE` tables

  * Reads the value of :variable:`query_response_time_range_base` and uses it to set the range base for the table

**Note:** The execution time for the ``FLUSH`` query will also be collected.

Stored procedures
-----------------

Stored procedure calls count as a single query.

Collect time point
------------------

Time is collected after query execution completes (before clearing data structures).

Version Specific Information
============================

  * :rn:`5.7.10-1`:
    Feature ported from |Percona Server| 5.6 

System Variables
================

.. variable:: query_response_time_flush

     :cli: Yes
     :conf: No
     :scope: Global
     :dyn: No
     :vartype: Boolean
     :default: OFF
     :range: OFF/ON

Setting this variable to ``ON`` will flush the statistics and re-read the :variable:`query_response_time_range_base`.


.. variable::  query_response_time_range_base

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: 10
     :range: 2-1000

Sets up the logarithm base for the scale.

**NOTE:** The variable takes effect only after this command has been executed: 

.. code-block:: mysql
 
   mysql> SET GLOBAL query_response_time_flush=1;

.. variable:: query_response_time_stats

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Boolean
     :default: OFF
     :range: ON/OFF

This global variable enables and disables collection of query times.

.. variable:: query_response_time_session_stats

     :cli: No
     :conf: No
     :scope: Session
     :dyn: Yes
     :vartype: Text
     :default: GLOBAL
     :range: ON/OFF/GLOBAL

This variable enables and disables collection of query times on session level, thus
customizing QRT behavior for individual connections. By default, its value is `GLOBAL`,
which means that its value is taken from the :variable:`query_response_time_stats` variable.

INFORMATION_SCHEMA Tables
=========================

.. table:: INFORMATION_SCHEMA.QUERY_RESPONSE_TIME

   :column VARCHAR TIME: Interval range in which the query occurred
   :column INT(11) COUNT: Number of queries with execution times that fell into that interval
   :column VARCHAR TOTAL: Total execution time of the queries 

.. table:: INFORMATION_SCHEMA.QUERY_RESPONSE_TIME_READ

   :column VARCHAR TIME: Interval range in which the query occurred
   :column INT(11) COUNT: Number of queries with execution times that fell into that interval
   :column VARCHAR TOTAL: Total execution time of the queries 

.. table:: INFORMATION_SCHEMA.QUERY_RESPONSE_TIME_WRITE

   :column VARCHAR TIME: Interval range in which the query occurred
   :column INT(11) COUNT: Number of queries with execution times that fell into that interval
   :column VARCHAR TOTAL: Total execution time of the queries 

