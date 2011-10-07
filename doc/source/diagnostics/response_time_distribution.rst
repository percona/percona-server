.. _response_time_distribution:

============================
 Response Time Distribution
============================

The slow query log provides exact information about queries that take a long time to execute. However, sometimes there are a large number of queries that each take a very short amount of time to execute. This feature provides a tool for analyzing that information by counting and displaying the number of queries according to the the length of time they took to execute. The user can define time intervals that divide the range 0 to positive infinity into smaller intervals and then collect the number of commands whose execution times fall into each of those intervals.

Note that in a replication environment, the server will not take into account *any* queries executed by the slave's SQL thread (whether they are slow or not) for the time distribution unless the log_slow_slave_statements variable is set.

The feature isn't implemented in all versions of the server. The variable :variable:`have_response_time_distribution` indicates whether or not it is implemented in the server you are running.

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

Usage
=====

SELECT
------

You can get the distribution using the query: ::

  > SELECT * from INFORMATION_SCHEMA.QUERY_RESPONSE_TIME
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

You can write a complex query like: ::

  SELECT c.count, c.time,
  (SELECT SUM(a.count) FROM INFORMATION_SCHEMA.QUERY_RESPONSE_TIME as a WHERE a.count != 0) as query_count,
  (SELECT COUNT(*)     FROM INFORMATION_SCHEMA.QUERY_RESPONSE_TIME as b WHERE b.count != 0) as not_zero_region_count,
  (SELECT COUNT(*)     FROM INFORMATION_SCHEMA.QUERY_RESPONSE_TIME) as region_count
  FROM INFORMATION_SCHEMA.QUERY_RESPONSE_TIME as c WHERE c.count > 0;

**Note:** If :variable:`query_response_time_stats` is ON, the execution times for these two ``SELECT`` queries will also be collected.

SHOW
----

Also, you can use this syntax: ::

  > SHOW QUERY_RESPONSE_TIME;
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

**Note:** The execution time for the SHOW query will also be collected.

FLUSH
-----

Flushing can be done with: ::

  FLUSH QUERY_RESPONSE_TIME;

``FLUSH`` does two things:

  * Clears the collected times from the :table:`QUERY_RESPONSE_TIME` table

  * Reads the value of :variable:`query_response_time_range_base` and uses it to set the range base for the table

**Note:** The execution time for the ``FLUSH`` query will also be collected.

Stored procedures
-----------------

Stored procedure calls count as single query.

Collect time point
------------------

Time is collected after query execution completes (before clearing data structures).

Limitations
===========

  * ``String width for seconds``

    * Value: 7

    * Compile-time variable: ``QUERY_RESPONSE_TIME_STRING_POSITIVE_POWER_LENGTH``

  * ``String width for microseconds``

    * Value: 6

    * Compile-time variable: ``QUERY_RESPONSE_TIME_STRING_NEGATIVE_POWER_LENGTH``

  * Minimum range base

    * Value: 2

    * Compile-time variable: ``QUERY_RESPONSE_TIME_MINIMUM_BASE``

  * Minimum range base

    * Value: 1000

    * Compile-time variable: ``QUERY_RESPONSE_TIME_MAXIMUM_BASE``

  * Minimum time interval

    * Value:  1 microsecond

  * Maximum time interval

    * Value: 9999999 seconds

Version Specific Information
============================

  * 5.1.49-12.0:
    Full functionality available.

  * 5.1.53-12.4:
    Introduced have_response_time_distribution.

  * 5.5.8-20.0:
    Renamed variable :variable:`enable_query_response_time_stats` to :variable:`query_response_time_stats`.

System Variables
================

.. variable:: have_response_time_distribution

     :version 5.1.53-12.4: Introduced.
     :scope: Global
     :dyn: No
     :vartype: Boolean
     :default: YES
     :range: YES/NO

Contains the value YES if the server you're running supports this feature; contains NO if the feature is not supported. It is enabled by default.


.. variable::  query_response_time_range_base

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: 10
     :range: 2-1000

Sets up the logarithm base for the scale.

**Note:** The variable takes effect only after this command has been executed: ::

  FLUSH QUERY_RESPONSE_TIME;

.. variable:: query_response_time_stats

     :version 5.5.8-20.0: Introduced.
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Boolean
     :default: OFF
     :range: ON/OFF

This variable enables and disables collection of query times if the feature is available in the server that's running. If the value of variable :variable:`have_response_time_distribution` is YES, then you can enable collection of query times by setting this variable to ON using ``SET GLOBAL``.

 Prior to release 5.5.8-20.0, this variable was named :variable:`enable_query_response_time_stats`.


INFORMATION_SCHEMA Tables
=========================

.. table:: INFORMATION_SCHEMA.QUERY_RESPONSE_TIME

   :column VARCHAR TIME: 
   :column INT(11) COUNT: 
   :column VARCHAR TOTAL:  

Implementation Details
======================

Implementation details on this feature are provided here.

Related Reading
===============

  * `Blueprint about Response Time Distribution <https://blueprints.launchpad.net/percona-server/+spec/response-time-distribution>`_
