.. _slow_extended:

================
 Slow Query Log
================

This feature adds microsecond time resolution and additional statistics to the slow query log output. It lets you enable or disable the slow query log at runtime, adds logging for the replica SQL thread, and adds fine-grained control over what and how much to log into the slow query log.

You can use *Percona-Toolkit*'s `pt-query-digest <http://www.percona.com/doc/percona-toolkit/2.1/pt-query-digest.html>`_ tool to aggregate similar queries together and report on those that consume the most execution time.


Version Specific Information
============================

  * `8.0.12-1`: The feature was ported from *Percona Server for MySQL* 5.7.

System Variables
================

.. _log_slow_filter:

.. rubric:: ``log_slow_filter``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global, Session
   * - Dynamic
     - Yes

Filters the slow log by the query's execution plan. The value is a comma-delimited string, and can contain any combination of the following values:

  * ``full_scan``:
    The query performed a full table scan.

  * ``full_join``:
    The query performed a full join (a join without indexes).

  * ``tmp_table``:
    The query created an implicit internal temporary table.

  * ``tmp_table_on_disk``:
    The query's temporary table was stored on disk.

  * ``filesort``:
    The query used a filesort.

  * ``filesort_on_disk``:
    The filesort was performed on disk.

Values are OR'ed together. If the string is empty, then the filter is disabled. If it is not empty, then queries will only be logged to the slow log if their execution plan matches one of the types of plans present in the filter.

For example, to log only queries that perform a full table scan, set the value to ``full_scan``. To log only queries that use on-disk temporary storage for intermediate results, set the value to ``tmp_table_on_disk,filesort_on_disk``.

.. _log_slow_rate_type:

.. rubric:: ``log_slow_rate_type``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global
   * - Dynamic
     - Yes
   * - Data type
     - Enumerated
   * - Default
     - ``session``
   * - Range
     - ``session``, ``query``

Specifies semantic of :ref:`log_slow_rate_limit` - ``session`` or ``query``.

.. _log_slow_rate_limit:

.. rubric:: ``log_slow_rate_limit``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global, session
   * - Dynamic
     - Yes
   * - Default
     - 1
   * - Range
     - 1-1000

Behavior of this variable depends from :ref:`log_slow_rate_type`.

Specifies that only a fraction of ``session/query`` should be logged. Logging is enabled for every nth ``session/query``. By default, n is 1, so logging is enabled for every ``session/query``. Please note: when :ref:`log_slow_rate_type` is ``session`` rate limiting is disabled for the replication thread.

Logging all queries might consume I/O bandwidth and cause the log file to grow large.
 * When :ref:`log_slow_rate_type` is ``session``, this option lets you log full sessions, so you have complete records of sessions for later analysis; but you can rate-limit the number of sessions that are logged. Note that this feature will not work well if your application uses any type of connection pooling or persistent connections. Note that you change :ref:`log_slow_rate_limit` in ``session`` mode, you should reconnect for get effect.

 * When :ref:`log_slow_rate_type` is ``query``, this option lets you log just some queries for later analysis. For example, if you set the value to 100, then one percent of queries will be logged.

Note that every query has global unique ``query_id`` and every connection can has it own (session) :ref:`log_slow_rate_limit`.
Decision "log or no" calculated in following manner:

 * if ``log_slow_rate_limit`` is 1 - log every query

 * If ``log_slow_rate_limit`` > 1 - randomly log every 1/``log_slow_rate_limit`` query. 

This allows flexible setup logging behavior.

For example, if you set the value to 100, then one percent of ``sessions/queries`` will be logged. In *Percona Server for MySQL* information about the :ref:`log_slow_rate_limit` has been added to the slow query log. This means that if the :ref:`log_slow_rate_limit` is effective it will be reflected in the slow query log for each written query. Example of the output looks like this: ::
 
  Log_slow_rate_type: query  Log_slow_rate_limit: 10

.. _log_slow_sp_statements:

.. rubric:: ``log_slow_sp_statements``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global
   * - Dynamic
     - Yes
   * - Data type
     - Boolean
   * - Default
     - TRUE
   * - Range
     - TRUE/FALSE

If ``TRUE``, statements executed by stored procedures are logged to the slow if it is open.

.. _improved_sp_reporting:

*Percona Server for MySQL* implemented improvements for logging of stored procedures to the slow query log:
 * Each query from a stored procedure is now logged to the slow query log individually
 * ``CALL`` itself isn't logged to the slow query log anymore as this would be counting twice for the same query which would lead to incorrect results
 * Queries that were called inside of stored procedures are annotated in the slow query log with the stored procedure name in which they run.

Example of the improved stored procedure slow query log entry:

.. code-block:: mysql

   mysql> DELIMITER //
   mysql> CREATE PROCEDURE improved_sp_log()
          BEGIN
           SELECT * FROM City;
           SELECT * FROM Country;
          END//
   mysql> DELIMITER ;
   mysql> CALL improved_sp_log();

When we check the slow query log after running the stored procedure ,with variable:`log_slow_sp_statements` set to ``TRUE``, it should look like this: ::

   # Time: 150109 11:38:55
   # User@Host: root[root] @ localhost []
   # Thread_id: 40  Schema: world  Last_errno: 0  Killed: 0
   # Query_time: 0.012989  Lock_time: 0.000033  Rows_sent: 4079  Rows_examined: 4079  Rows_affected: 0  Rows_read: 4079
   # Bytes_sent: 161085
   # Stored routine: world.improved_sp_log
   SET timestamp=1420803535;
   SELECT * FROM City;
   # User@Host: root[root] @ localhost []
   # Thread_id: 40  Schema: world  Last_errno: 0  Killed: 0
   # Query_time: 0.001413  Lock_time: 0.000017  Rows_sent: 4318  Rows_examined: 4318  Rows_affected: 0  Rows_read: 4318
   # Bytes_sent: 194601
   # Stored routine: world.improved_sp_log
   SET timestamp=1420803535;

If variable :ref:`log_slow_sp_statements` is set to ``FALSE``:

 * Entry is added to a slow-log for a ``CALL`` statement only and not for any of the individual statements run in that stored procedure
 * Execution time is reported for the ``CALL`` statement as the total execution time of the ``CALL`` including all its statements

If we run the same stored procedure with the variable :ref:`log_slow_sp_statements` is set to ``FALSE`` slow query log should look like this: ::

  # Time: 150109 11:51:42
  # User@Host: root[root] @ localhost []
  # Thread_id: 40  Schema: world  Last_errno: 0  Killed: 0
  # Query_time: 0.013947  Lock_time: 0.000000  Rows_sent: 4318  Rows_examined: 4318  Rows_affected: 0  Rows_read: 4318
  # Bytes_sent: 194612
  SET timestamp=1420804302;
  CALL improved_sp_log();

.. note::

 Support for logging stored procedures doesn't involve triggers, so they won't be logged even if this feature is enabled.

.. _log_slow_verbosity:

.. rubric:: ``log_slow_verbosity``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global, session
   * - Dynamic
     - Yes

Specifies how much information to include in your slow log. The value is a comma-delimited string, and can contain any combination of the following values:

  * ``microtime``:
    Log queries with microsecond precision.

  * ``query_plan``:
    Log information about the query's execution plan.

  * ``innodb``:
    Log *InnoDB* statistics.

  * ``minimal``:
    Equivalent to enabling just ``microtime``.

  * ``standard``:
    Equivalent to enabling ``microtime,query_plan``.

  * ``full``:
    Equivalent to all other values OR'ed together without the ``profiling`` and ``profiling_use_getrusage`` options.

  * ``profiling``:
    Enables profiling of all queries in all connections.

  * ``profiling_use_getrusage``:
    Enables usage of the getrusage function.

  * ``query_info``: 
    Enables printing ``Query_tables`` and ``Query_digest`` into the slow query log. These fields are disabled by default.

Values are OR'ed together.

For example, to enable microsecond query timing and *InnoDB* statistics, set this option to ``microtime,innodb`` or ``standard``. To turn all options on, set the option to ``full``.

.. _slow_query_log_use_global_control:

.. rubric:: ``slow_query_log_use_global_control``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global
   * - Dynamic
     - Yes
   * - Default
     - None

Specifies which variables have global scope instead of local. For such variables, the global variable value is used in the current session, but without copying this value to the session value. Value is a "flag" variable - you can specify multiple values separated by commas

  * ``none``:
    All variables use local scope

  * ``log_slow_filter``:
    Global variable :ref:`log_slow_filter` has effect (instead of local)

  * ``log_slow_rate_limit``:
    Global variable :ref:`log_slow_rate_limit` has effect (instead of local)

  * ``log_slow_verbosity``:
    Global variable :ref:`log_slow_verbosity` has effect (instead of local)

  * ``long_query_time``:
    Global variable :ref:`long_query_time` has effect (instead of local)

  * ``min_examined_row_limit``:
    Global variable ``min_examined_row_limit`` has effect (instead of local)

  * ``all``
    Global variables has effect (instead of local)

.. _slow_query_log_always_write_time:

.. rubric:: ``slow_query_log_always_write_time``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global
   * - Dynamic
     - Yes
   * - Default
     - 10

This variable can be used to specify the query execution time after which the query will be written to the slow query log. It can be used to specify an additional execution time threshold for the slow query log, that, when exceeded, will cause a query to be logged unconditionally, that is, :ref:`log_slow_rate_limit` will not apply to it.

Other Information
=================

Changes to the Log Format
-------------------------

The feature adds more information to the slow log output. Here is a sample log entry: ::

  # Time: 130601  8:01:06.058915
  # User@Host: root[root] @ localhost []  Id:    42
  # Schema: imdb  Last_errno: 0  Killed: 0
  # Query_time: 7.725616  Lock_time: 0.000328  Rows_sent: 4  Rows_examined: 1543720  Rows_affected: 0
  # Bytes_sent: 272  Tmp_tables: 0  Tmp_disk_tables: 0  Tmp_table_sizes: 0
  # Full_scan: Yes  Full_join: No  Tmp_table: No  Tmp_table_on_disk: No
  # Filesort: No  Filesort_on_disk: No  Merge_passes: 0
  SET timestamp=1370073666;
  SELECT id,title,production_year FROM title WHERE title = 'Bambi';


Another example (:ref:`log_slow_verbosity` ``=profiling``): ::

  # Time: 130601  8:03:20.700441
  # User@Host: root[root] @ localhost []  Id:    43
  # Schema: imdb  Last_errno: 0  Killed: 0
  # Query_time: 7.815071  Lock_time: 0.000261  Rows_sent: 4  Rows_examined: 1543720  Rows_affected: 0
  # Bytes_sent: 272
  # Profile_starting: 0.000125 Profile_starting_cpu: 0.000120 
  Profile_checking_permissions: 0.000021 Profile_checking_permissions_cpu: 0.000021 
  Profile_Opening_tables: 0.000049 Profile_Opening_tables_cpu: 0.000048 Profile_init: 0.000048 
  Profile_init_cpu: 0.000049 Profile_System_lock: 0.000049 Profile_System_lock_cpu: 0.000048 
  Profile_optimizing: 0.000024 Profile_optimizing_cpu: 0.000024 Profile_statistics: 0.000036 
  Profile_statistics_cpu: 0.000037 Profile_preparing: 0.000029 Profile_preparing_cpu: 0.000029 
  Profile_executing: 0.000012 Profile_executing_cpu: 0.000012 Profile_Sending_data: 7.814583 
  Profile_Sending_data_cpu: 7.811634 Profile_end: 0.000013 Profile_end_cpu: 0.000012 
  Profile_query_end: 0.000014 Profile_query_end_cpu: 0.000014 Profile_closing_tables: 0.000023 
  Profile_closing_tables_cpu: 0.000023 Profile_freeing_items: 0.000051 
  Profile_freeing_items_cpu: 0.000050 Profile_logging_slow_query: 0.000006 
  Profile_logging_slow_query_cpu: 0.000006 
  # Profile_total: 7.815085 Profile_total_cpu: 7.812127 
  SET timestamp=1370073800;
  SELECT id,title,production_year FROM title WHERE title = 'Bambi';

Notice that the ``Killed: `` keyword is followed by zero when the
query successfully completes. If the query was killed, the ``Killed:``
keyword is followed by a number other than zero:

====================  =================================================
Killed Numeric Code   Exception
====================  =================================================
0                     NOT_KILLED
1                     KILL_BAD_DATA
1053                  ER_SERVER_SHUTDOWN (see |MySQL| Documentation)
1317                  ER_QUERY_INTERRUPTED (see |MySQL| Documentation)
3024                  ER_QUERY_TIMEOUT (see |MySQL| Documentation)
Any other number      KILLED_NO_VALUE (Catches all other cases)
====================  =================================================

.. seealso::

   |MySQL| Documentation: |MySQL| Server Error Codes
      https://dev.mysql.com/doc/mysql-errors/8.0/en/server-error-reference.html

Connection and Schema Identifier
--------------------------------

Each slow log entry now contains a connection identifier, so you can trace all the queries coming from a single connection. This is the same value that is shown in the Id column in ``SHOW FULL PROCESSLIST`` or returned from the ``CONNECTION_ID()`` function.

Each entry also contains a schema name, so you can trace all the queries whose default database was set to a particular schema. ::

  # Id: 43  Schema: imdb

Microsecond Time Resolution and Extra Row Information
-----------------------------------------------------

This is the original functionality offered by the ``microslow`` feature. ``Query_time`` and ``Lock_time`` are logged with microsecond resolution.

The feature also adds information about how many rows were examined for ``SELECT`` queries, and how many were analyzed and affected for ``UPDATE``, ``DELETE``, and ``INSERT`` queries, ::

  # Query_time: 0.962742  Lock_time: 0.000202  Rows_sent: 4  Rows_examined: 1543719  Rows_affected: 0

Values and context:

  * ``Rows_examined``:
    Number of rows scanned - ``SELECT``

  * ``Rows_affected``:
    Number of rows changed - ``UPDATE``, ``DELETE``, ``INSERT``

Memory Footprint
----------------

The feature provides information about the amount of bytes sent for the result of the query and the number of temporary tables created for its execution - differentiated by whether they were created on memory or on disk - with the total number of bytes used by them. :: 

  # Bytes_sent: 8053  Tmp_tables: 1  Tmp_disk_tables: 0  Tmp_table_sizes: 950528

Values and context:

  * ``Bytes_sent``:
    The amount of bytes sent for the result of the query

  * ``Tmp_tables``:
    Number of temporary tables created on memory for the query

  * ``Tmp_disk_tables``:
    Number of temporary tables created on disk for the query

  * ``Tmp_table_sizes``:
    Total Size in bytes for all temporary tables used in the query


Query Plan Information
----------------------

Each query can be executed in various ways. For example, it may use indexes or do a full table scan, or a temporary table may be needed. These are the things that you can usually see by running ``EXPLAIN`` on the query. The feature will now allow you to see the most important facts about the execution in the log file. ::

  # Full_scan: Yes  Full_join: No  Tmp_table: No  Tmp_table_on_disk: No
  # Filesort: No  Filesort_on_disk: No  Merge_passes: 0

The values and their meanings are documented with the :ref:`log_slow_filter` option.

*InnoDB* Usage Information
--------------------------

The final part of the output is the *InnoDB* usage statistics. *MySQL* currently shows many per-session statistics for operations with ``SHOW SESSION STATUS``, but that does not include those of |InnoDB|, which are always global and shared by all threads. This feature lets you see those values for a given query. ::

  #   InnoDB_IO_r_ops: 6415  InnoDB_IO_r_bytes: 105103360  InnoDB_IO_r_wait: 0.001279
  #   InnoDB_rec_lock_wait: 0.000000  InnoDB_queue_wait: 0.000000
  #   InnoDB_pages_distinct: 6430

Values:

  * ``innodb_IO_r_ops``:
    Counts the number of page read operations scheduled. The actual number of read operations may be different, but since this can be done asynchronously, there is no good way to measure it.

  * ``innodb_IO_r_bytes``:
    Similar to innodb_IO_r_ops, but the unit is bytes.

  * ``innodb_IO_r_wait``:
    Shows how long (in seconds) it took *InnoDB* to actually read the data from storage.

  * ``innodb_rec_lock_wait``:
    Shows how long (in seconds) the query waited for row locks.

  * ``innodb_queue_wait``:
    Shows how long (in seconds) the query spent either waiting to enter the *InnoDB* queue or inside that queue waiting for execution.

  * ``innodb_pages_distinct``:
    Counts approximately the number of unique pages the query accessed. The approximation is based on a small hash array representing the entire buffer pool, because it could take a lot of memory to map all the pages. The inaccuracy grows with the number of pages accessed by a query, because there is a higher probability of hash collisions.

If the query did not use *InnoDB* tables, that information is written into the log instead of the above statistics.

Related Reading
===============

  * `Impact of logging on MySQL's performance <http://www.mysqlperformanceblog.com/2009/02/10/impact-of-logging-on-mysql%E2%80%99s-performance/>`_

  * `log_slow_filter Usage <http://www.mysqlperformanceblog.com/2008/09/22/finding-what-created_tmp_disk_tables-with-log_slow_filter/>`_

  * `Added microseconds to the slow query log event time <https://jira.percona.com/browse/PS-1136>`_
