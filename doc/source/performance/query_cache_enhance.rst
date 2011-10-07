.. _query_cache_enhance:

==========================
 Query Cache Enhancements
==========================

This page describes the enhancements for the query cache. At the moment three features are available:

  * Disabling the cache completely

  * Diagnosing contention more easily

  * Ignoring comments

Disabling the cache completely
==============================

This feature allows the user to completely disable use of the query cache. When the server is compiled with the query cache enabled, the query cache is locked during use by the query cache mutex. This lock can cause performance to decrease in some situations. By disabling use of the query cache altogether when the server is started, any possibility of locking it is eliminated, and performance may be improved.

The query cache can now be disabled at server startup or in an option file by: ::

  --query_cache_type=0

The default is 1 (query cache enabled).

**Note:** This variable already exists in standard |MySQL|, but when setting query_cache_type=0, the query cache mutex will still be in used. Setting query_cache_type=0 in |Percona Server| ensures that both the cache is disabled and the mutex is not used.

If query caching is off and a user tries to turn it on from within a session, the following error will be reported: ::

  SET GLOBAL query_cache_type=ON;
  ERROR 1651(HY000): Query cache is disabled; restart the server with query_cache_type=1 to enable it

**Note:** This variable is implemented in standard |MySQL| from version 5.5.0.


Diagnosing contention more easily
=================================

This features provides a new thread state - ``Waiting on query cache mutex``. It has always been difficult to spot query cache bottlenecks because these bottlenecks usually happen intermittently and are not directly reported by the server. This new thread state appear in the output of SHOW PROCESSLIST, easing diagnostics.

Imagine that we run three queries simultaneously (each one in a separate thread):

  > SELECT number from t where id > 0;
  > SELECT number from t where id > 0;
  > SELECT number from t where id > 0;

If we experience query cache contention, the output of SHOW PROCESSLIT will look like this: ::

  > SHOW PROCESSLIST;
  Id      User    Host            db      Command Time    State                          Info
  2       root    localhost       test    Sleep   2       NULL
  3       root    localhost       test    Query   2       Waiting on query cache mutex  SELECT number from t where id > 0;
  4       root    localhost       test    Query   1       Waiting on query cache mutex   SELECT number from t where id > 0;
  5       root    localhost       test    Query   0       NULL

Ignoring comments
=================

This feature adds an option to make the server ignore comments when checking for a query cache hit. For example, consider these two queries: ::

  /* first query  */ select name from users where users.name like 'Bob%';
  /* retry search */ select name from users where users.name like 'Bob%';

By default (option off), the queries are considered different, so the server will execute them both and cache them both.

If the option is enabled, the queries are considered identical, so the server will execute and cache the first one and will serve the second one directly from the query cache.


.. Version Specific Information
.. ----------------------------

.. Disabling the query cache completely

..  Percona Server Version	 Comments
.. 5.1.49-12.0	 Full functionality available.
.. Diagnosing contention more easily

..  Percona Server Version	 Comments
.. 5.1.49-12.0	 Full functionality available.
.. Ignoring comments

..  Percona Server Version	 Comments
.. 5.1.47-11.0	 Critical bug (see MySQL bug 55032). Release was recalled.
.. 5.1.47-11.1	 Fixed critical bug from previous release. MySQL bug 55032 actual. Bug b603618 actual. Bug 603619 actual.
.. 5.1.47-11.2	 Full functionality available.
.. 5.1.48-12.0	 Full functionality available.

.. Other Information

.. Disabling the query cache completely

.. Author/Origin	 Percona
.. Bugs fixed	LP bug 609027, MySQL bug 38551
.. Diagnosing contention more easily

.. Author/Origin	 Percona
.. Bugs fixed	LP bug589484

System Variables
================

.. variable:: query_cache_strip_comments

   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: Yes
   :vartype: Boolean
   :default: Off


Makes the server ignore comments when checking for a query cache hit.

Other Reading
-------------

  * `MySQL general thread states <http://dev.mysql.com/doc/refman/5.1/en/general-thread-states.html>`_

  * `RAII <http://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Resource_Acquisition_Is_Initialization>`_

  * `Scope guard <http://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Scope_Guard>`_

  * `Query cache freezes <http://www.mysqlperformanceblog.com/2009/03/19/mysql-random-freezes-could-be-the-query-cache/>`_
