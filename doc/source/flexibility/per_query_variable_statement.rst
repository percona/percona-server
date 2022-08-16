.. _per_query_variable_statement:

==============================
 Per-query variable statement
==============================

*Percona Server for MySQL* has implemented per-query variable statement support. This feature provides the ability to set variable values only for a certain query, after execution of which the previous values will be restored. Per-query variable values can be set up with the following command:

.. code-block:: mysql

   mysql> SET STATEMENT <variable=value> FOR <statement>;

Examples
========

If we want to increase the :ref:`sort_buffer_size` value just for one specific sort query we can do it like this: 

.. code-block:: mysql

   mysql> SET STATEMENT sort_buffer_size=100000 FOR SELECT name FROM name ORDER BY name;

This feature can also be used with `max_execution_time <http://dev.mysql.com/doc/refman/5.7/en/server-system-variables.html#sysvar_max_execution_time>`_ to limit the execution time for a specific query:

.. code-block:: mysql

   mysql> SET STATEMENT max_execution_time=1000 FOR SELECT name FROM name ORDER BY name;

We can provide more than one variable we want to set up:

.. code-block:: mysql

   mysql> SET STATEMENT sort_buffer_size=100000, max_statement_time=1000 FOR SELECT name FROM name ORDER BY name;

Version Specific Information
============================

  * :ref:`5.7.10-1`: Feature ported from *Percona Server for MySQL* 5.6 

Other Reading
=============
* `WL#681: Per query variable settings <http://dev.mysql.com/worklog/task/?id=681>`_

