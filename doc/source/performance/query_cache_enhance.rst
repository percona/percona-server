.. _query_cache_enhance:

==========================
 Query Cache Enhancements
==========================

This page describes the enhancements for the query cache. At the moment three features are available:

  * Disabling the cache completely

  * Diagnosing contention more easily

  * Ignoring comments

Diagnosing contention more easily
=================================

This features provides a new thread state - ``Waiting on query cache mutex``. It has always been difficult to spot query cache bottlenecks because these bottlenecks usually happen intermittently and are not directly reported by the server. This new thread state appear in the output of SHOW PROCESSLIST, easing diagnostics.

Imagine that we run three queries simultaneously (each one in a separate thread): ::

  > SELECT number from t where id > 0;
  > SELECT number from t where id > 0;
  > SELECT number from t where id > 0;

If we experience query cache contention, the output of ``SHOW PROCESSLIST`` will look like this: ::

  > SHOW PROCESSLIST;
  Id      User    Host            db      Command Time    State                          Info
  2       root    localhost       test    Sleep   2       NULL
  3       root    localhost       test    Query   2       Waiting on query cache mutex  SELECT number from t where id > 0;
  4       root    localhost       test    Query   1       Waiting on query cache mutex  SELECT number from t where id > 0;
  5       root    localhost       test    Query   0       NULL

.. _ignoring_comments:

Ignoring comments
=================

This feature adds an option to make the server ignore comments when checking for a query cache hit. For example, consider these two queries: ::

  /* first query  */ select name from users where users.name like 'Bob%';
  /* retry search */ select name from users where users.name like 'Bob%';

By default (option off), the queries are considered different, so the server will execute them both and cache them both.

If the option is enabled, the queries are considered identical, so the server will execute and cache the first one and will serve the second one directly from the query cache.


System Variables
================

.. _query_cache_strip_comments:

.. rubric:: ``query_cache_strip_comments``

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
     - Off

Makes the server ignore comments when checking for a query cache hit.

Other Reading
-------------

  * `MySQL general thread states <http://dev.mysql.com/doc/refman/5.7/en/general-thread-states.html>`_

  * `Query cache freezes <http://www.mysqlperformanceblog.com/2009/03/19/mysql-random-freezes-could-be-the-query-cache/>`_
