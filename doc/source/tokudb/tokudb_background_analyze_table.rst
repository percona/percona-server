.. _tokudb_background_analyze_table:

===============================
TokuDB Background ANALYZE TABLE
===============================

.. Important:: 

   Starting with :ref:`8.0.28-19`, the TokuDB storage engine is no longer supported. We have removed the storage engine from the installation packages and disabled the storage engine in our binary builds.

   Starting with :ref:`8.0.26-16`, the binary builds and packages include but disable the TokuDB storage engine plugins. The ``tokudb_enabled`` option and the ``tokudb_backup_enabled`` option control the state of the plugins and have a default setting of ``FALSE``. The result of attempting to load the plugins are the plugins fail to initialize and print a deprecation message.

   We recommend :ref:`migrate-myrocks`. To enable the plugins to migrate to another storage engine, set the ``tokudb_enabled`` and ``tokudb_backup_enabled`` options to ``TRUE`` in your ``my.cnf`` file and restart your server instance. Then, you can load the plugins.

   The TokuDB Storage Engine was `declared as deprecated <https://www.percona.com/doc/percona-server/8.0/release-notes/Percona-Server-8.0.13-3.html>`__ in Percona Server for MySQL 8.0. For more information, see the Percona blog post: `Heads-Up: TokuDB Support Changes and Future Removal from Percona Server for MySQL 8.0 <https://www.percona.com/blog/2021/05/21/tokudb-support-changes-and-future-removal-from-percona-server-for-mysql-8-0/>`__.

*Percona Server for MySQL* has an option to automatically analyze tables in the background based on a measured change in data. This has been done by implementing the background job manager that can perform operations on a background thread. 

Background Jobs
===============

Background jobs and schedule are transient in nature and are not persisted anywhere. Any currently running job will be terminated on shutdown and all scheduled jobs will be forgotten about on server restart. There can't be two jobs on the same table scheduled or running at any one point in time. If you manually invoke an ``ANALYZE TABLE`` that conflicts with either a pending or running job, the running job will be canceled and the users task will run immediately in the foreground. All the scheduled and running background jobs can be viewed by querying the :ref:`TOKUDB_BACKGROUND_JOB_STATUS` table.

New :ref:`tokudb_analyze_in_background` variable has been implemented in order to control if the ``ANALYZE TABLE`` will be dispatched to the background process or if it will be running in the foreground. 
To control the function of ``ANALYZE TABLE`` a new :ref:`tokudb_analyze_mode` variable has been implemented. This variable offers options to cancel any running or scheduled job on the specified table (``TOKUDB_ANALYZE_CANCEL``), use existing analysis algorithm (``TOKUDB_ANALYZE_STANDARD``), or to recount the logical rows in table and update persistent count (``TOKUDB_ANALYZE_RECOUNT_ROWS``).

``TOKUDB_ANALYZE_RECOUNT_ROWS`` is a new mechanism that is used to perform a logical recount of all rows in a table and persist that as the basis value for the table row estimate. This mode was added for tables that have been upgraded from an older version of *TokuDB* that only reported physical row counts and never had a proper logical row count. Newly created tables/partitions will begin counting logical rows correctly from their creation and should not need to be recounted unless some odd edge condition causes the logical count to become inaccurate over time. This analysis mode has no effect on the table cardinality counts. It will take the currently set session values for :ref:`tokudb_analyze_in_background`, and :ref:`tokudb_analyze_throttle`. Changing the global or session instances of these values after scheduling will have no effect on the job.

Any background job, both pending and running, can be canceled by setting the :ref:`tokudb_analyze_mode` to ``TOKUDB_ANALYZE_CANCEL`` and issuing the ``ANALYZE TABLE`` on the table for which you want to cancel all the jobs for.

Auto analysis
=============

To implement the background analysis and gathering of cardinality statistics on a *TokuDB* tables new ``delta`` value is now maintained in memory for each *TokuDB* table. This value is not persisted anywhere and it is reset to ``0`` on a server start. It is incremented for each ``INSERT/UPDATE/DELETE`` command and ignores the impact of transactions (rollback specifically). When this delta value exceeds the :ref:`tokudb_auto_analyze` percentage of rows in the table an analysis is performed according to the current session's settings. Other analysis for this table will be disabled until this analysis completes. When this analysis completes, the delta is reset to ``0`` to begin recalculating table changes for the next potential analysis. 

Status values are now reported to server immediately upon completion of any analysis (previously new status values were not used until the table has been closed and re-opened). Half-time direction reversal of analysis has been implemented, meaning that if a :ref:`tokudb_analyze_time` is in effect and the analysis has not reached the half way point of the index by the time :ref:`tokudb_analyze_time`/2 has been reached: it will stop the forward progress and restart the analysis from the last/rightmost row in the table, progressing leftwards and keeping/adding to the status information accumulated from the first half of the scan.

For small ratios of ``table_rows`` / :ref:`tokudb_auto_analyze`, auto analysis will be run for almost every change. The trigger formula is: ``if (table_delta >= ((table_rows * tokudb_auto_analyze) / 100))`` then run ``ANALYZE TABLE``. If a user manually invokes an ``ANALYZE TABLE`` and :ref:`tokudb_auto_analyze` is enabled and there are no conflicting background jobs, the users ``ANALYZE TABLE`` will behave exactly as if the delta level has been exceeded in that the analysis is executed and delta reset to ``0`` upon completion.

System Variables
================

.. _tokudb_analyze_in_background:

.. rubric:: ``tokudb_analyze_in_background``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global/Session
   * - Dynamic
     - Yes
   * - Data type
     - Boolean
   * - Default
     - ``ON``

When this variable is set to ``ON``  it will dispatch any ``ANALYZE TABLE`` job to a background process and return immediately, otherwise ``ANALYZE TABLE`` will run in foreground/client context.

.. _tokudb_analyze_mode:

.. rubric:: ``tokudb_analyze_mode``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global/Session
   * - Dynamic
     - Yes
   * - Data type
     - ENUM
   * - Default
     - ``TOKUDB_ANALYZE_STANDARD``
   * - Range
     - ``TOKUDB_ANALYZE_CANCEL``, ``TOKUDB_ANALYZE_STANDARD``, ``TOKUDB_ANALYZE_RECOUNT_ROWS``

This variable is used to control the function of ``ANALYZE TABLE``. Possible values are:

 * ``TOKUDB_ANALYZE_CANCEL`` - Cancel any running or scheduled job on the specified table. 
 * ``TOKUDB_ANALYZE_STANDARD`` - Use existing analysis algorithm. This is the standard table cardinality analysis mode used to obtain cardinality statistics for a tables and its indexes. It will take the currently set session values for :ref:`tokudb_analyze_time`, :ref:`tokudb_analyze_in_background`, and :ref:`tokudb_analyze_throttle` at the time of its scheduling, either via a user invoked ``ANALYZE TABLE`` or an auto schedule as a result of :ref:`tokudb_auto_analyze` threshold being hit. Changing the global or session instances of these values after scheduling will have no effect on the scheduled job.
 * ``TOKUDB_ANALYZE_RECOUNT_ROWS`` - Recount logical rows in table and update persistent count. This is a new mechanism that is used to perform a logical recount of all rows in a table and persist that as the basis value for the table row estimate. This mode was added for tables that have been upgraded from an older version of *TokuDB*/PerconaFT that only reported physical row counts and never had a proper logical row count. Newly created tables/partitions will begin counting logical rows correctly from their creation and should not need to be recounted unless some odd edge condition causes the logical count to become inaccurate over time. This analysis mode has no effect on the table cardinality counts. It will take the currently set session values for :ref:`tokudb_analyze_in_background`, and :ref:`tokudb_analyze_throttle`. Changing the global or session instances of these values after scheduling will have no effect on the job.

.. _tokudb_analyze_throttle:

.. rubric:: ``tokudb_analyze_throttle``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global/Session
   * - Dynamic
     - Yes
   * - Data type
     - Numeric
   * - Default
     - 0

This variable is used to define maximum number of keys to visit per second when performing ``ANALYZE TABLE`` with either a ``TOKUDB_ANALYZE_STANDARD`` or ``TOKUDB_ANALYZE_RECOUNT_ROWS``.

.. _tokudb_analyze_time:

.. rubric:: ``tokudb_analyze_time``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global/Session
   * - Dynamic
     - Yes
   * - Data type
     - Numeric
   * - Default
     - 5

This session variable controls the number of seconds an analyze operation will spend on each index when calculating cardinality. Cardinality is shown by executing the following command:

  .. code-block:: mysql

    SHOW INDEXES FROM table_name;

If an analyze is never performed on a table then the cardinality is ``1`` for primary key indexes and unique secondary indexes, and ``NULL`` (unknown) for all other indexes. Proper cardinality can lead to improved performance of complex SQL statements.

.. _tokudb_auto_analyze:

.. rubric:: ``tokudb_auto_analyze``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global/Session
   * - Dynamic
     - Yes
   * - Data type
     - Numeric
   * - Default
     - 30

Percentage of table change as ``INSERT/UPDATE/DELETE`` commands to trigger an ``ANALYZE TABLE`` using the current session :ref:`tokudb_analyze_in_background`, :ref:`tokudb_analyze_mode`, :ref:`tokudb_analyze_throttle`, and :ref:`tokudb_analyze_time` settings. If this variable is enabled and :ref:`tokudb_analyze_in_background` variable is set to ``OFF``, analysis will be performed directly within the client thread context that triggered the analysis. **NOTE:** *InnoDB* enabled this functionality by default when they introduced it. Due to the potential unexpected new load it might place on a server, it is disabled by default in *TokuDB*.

.. _tokudb_cardinality_scale_percent:

.. rubric:: ``tokudb_cardinality_scale_percent``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global/Session
   * - Dynamic
     - Yes
   * - Data type
     - Numeric
   * - Default
     - 100
   * - Range
     - 0-100

Percentage to scale table/index statistics when sending to the server to make an index appear to be either more or less unique than it actually is. *InnoDB* has a hard coded scaling factor of 50%. So if a table of 200 rows had an index with 40 unique values, InnoDB would return 200/40/2 or 2 for the index. The new TokuDB formula is the same but factored differently to use percent, for the same table.index (200/40 * :ref:`tokudb_cardinality_scale`) / 100, for a scale of 50% the result would also be 2 for the index.

INFORMATION_SCHEMA Tables
=========================

.. _TOKUDB_BACKGROUND_JOB_STATUS:

``INFORMATION_SCHEMA.TOKUDB_BACKGROUND_JOB_STATUS``

.. list-table::
      :header-rows: 1

      * - Column Name
        - Description
      * - 'id'
        - 'Simple monotonically incrementing job id, resets to ``0`` on server start.'
      * - 'database_name'
        - 'Database name'
      * - 'table_name'
        - 'Table name'
      * - 'job_type'
        - 'Type of job, either ``TOKUDB_ANALYZE_STANDARD`` or ``TOKUDB_ANALYZE_RECOUNT_ROWS``'
      * - 'job_params'
        - 'Param values used by this job in string format. For example: ``TOKUDB_ANALYZE_DELETE_TIME=1.0; TOKUDB_ANALYZE_TIME=5; TOKUDB_ANALYZE_THROTTLE=2048;``'
      * - 'scheduler'
        - 'Either ``USER`` or ``AUTO`` to indicate if the job was explicitly scheduled by a user or if it was scheduled as an automatic trigger'
      * - 'scheduled_time'
        - 'The time the job was scheduled'
      * - 'started_time'
        - 'The time the job was started'
      * - 'status'
        - 'Current job status if running. For example: ``ANALYZE TABLE standard db.tbl.idx 3 of 5 50% rows 10% time scanning forward``'

This table holds the information on scheduled and running background ``ANALYZE TABLE`` jobs for *TokuDB* tables.