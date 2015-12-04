.. _per_query_variable_statement:

==============================
 Per-query variable statement
==============================

|Percona Server| has implemented per-query variable statement support in :rn:`5.6.14-62.0`. This feature provides the ability to set variable values only for a certain query, after execution of which the previous values will be restored. Per-query variable values can be set up with the following command:

.. code-block:: mysql

   mysql> SET STATEMENT <variable=value> FOR <statement>;

Examples
========

If we want to increase the :variable:`sort_buffer_size` value just for one specific sort query we can do it like this: 

.. code-block:: mysql

   mysql> SET STATEMENT sort_buffer_size=100000 FOR SELECT name FROM name ORDER BY name;

This feature can also be used with :ref:`statement_timeout` to limit the execution time for a specific query:

.. code-block:: mysql

   mysql> SET STATEMENT max_statement_time=1000 FOR SELECT name FROM name ORDER BY name;

We can provide more than one variable we want to set up:

.. code-block:: mysql

   mysql> SET STATEMENT sort_buffer_size=100000, max_statement_time=1000 FOR SELECT name FROM name ORDER BY name;

Version Specific Information
============================

  * :rn:`5.6.14-62.0`
    Feature implemented

Other Reading
=============
* `WL#681: Per query variable settings <http://dev.mysql.com/worklog/task/?id=681>`_

