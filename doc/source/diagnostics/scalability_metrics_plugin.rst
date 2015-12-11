.. _scalability_metrics_plugin:

=====================================
 Metrics for scalability measurement
=====================================

|Percona Server| has implemented extra scalability metrics. These metrics allow using Little's Law, queueing theory, and Universal Scalability Law to gain insights into server performance. This feature is implemented as a plugin.

Installation
============

Scalability Metrics plugin is shipped with |Percona Server|, but it is not installed by default. To enable the plugin you must run the following command: 

.. code-block:: mysql

   INSTALL PLUGIN scalability_metrics SONAME 'scalability_metrics.so';

You can check if the plugin is loaded correctly by running:

.. code-block:: mysql

   SHOW PLUGINS;

The plugin should be listed in the output:
    
.. code-block:: mysql

   +--------------------------------+----------+--------------------+------------------------+---------+
   | Name                           | Status   | Type               | Library                | License |
   +--------------------------------+----------+--------------------+------------------------+---------+
   ...
   | scalability_metrics            | ACTIVE   | AUDIT              | scalability_metrics.so | GPL     |
   +--------------------------------+----------+--------------------+------------------------+---------+

System Variables
================

.. variable:: scalability_metrics_control

     :cli: Yes
     :scope: Global
     :dyn: Yes
     :vartype: String
     :default: ``OFF``
     :values: ``OFF``, ``ON``, ``RESET``

This variable can be used to enable and disable the collection of metrics for scalability measurement. By setting the value to ``RESET`` all counters will be reset while continuing to count metrics.

Status Variables
================

.. variable:: scalability_metrics_elapsedtime
   
   :vartype: Numeric

This status variable shows total time elapsed, in microseconds, since metrics collection was started.

.. variable:: scalability_metrics_queries
   
   :vartype: Numeric

This status variable shows number of completed queries since metrics collection was started.

.. variable:: scalability_metrics_concurrency
   
   :vartype: Numeric

This status variable shows number of queries currently executed.

.. variable:: scalability_metrics_totaltime
   
   :vartype: Numeric

This status variable shows total execution time of all queries, including the in-progress time of currently executing queries, in microseconds (ie. if two queries executed with 1 second of response time each, the result is 2 seconds).

.. variable:: scalability_metrics_busytime
   
   :vartype: Numeric

This counter accounts the non-idle server time, that is, time when at least one query was executing. 


Version Specific Information
============================

  * :rn:`5.7.10-1`
    Feature ported from |Percona Server| 5.6

Other Reading
=============

* `Fundamental performance and scalability instrumentation <http://www.xaprb.com/blog/2011/10/06/fundamental-performance-and-scalability-instrumentation/>`_
* `Forecasting MySQL Scalability with the Universal Scalability Law Whitepaper <http://www.percona.com/files/white-papers/forecasting-mysql-scalability.pdf>`_
