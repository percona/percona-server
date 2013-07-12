.. _threadpool:

=============
 Thread Pool
=============

.. note::

 This feature implementation is considered BETA quality.

|MySQL| executes statements using one thread per client connection. Once the number of connections increases past a certain point performance will degrade. 

This feature enables the server to keep the top performance even with large number of client connections by introducing a dynamic thread pool. By using the thread pool server would decrease the number of threads, which will then reduce the context switching and hot locks contentions. Using the thread pool will have the most effect with ``OLTP`` workloads (relatively short CPU-bound queries). 

In order to enable the thread pool variable :variable:`thread_handling` should be set up to ``pool-of-threads`` value. This can be done by adding: ::

 thread_handling=pool-of-threads

to the |MySQL| configuration file :file:`my.cnf`.

Although the default values for the thread pool should provide good performance, additional `tuning <https://kb.askmonty.org/en/threadpool-in-55/#optimizing-server-variables-on-unix>`_ can be performed with the dynamic system variables described below. 

.. note:: 
 
  Current implementation of the thread pool is built in the server, unlike the upstream version which is implemented as a plugin. Another significant implementation difference is that this implementation doesn't try to minimize the number of concurrent transactions like the ``MySQL Enterprise Threadpool``. Because of these things this implementation isn't compatible with the upstream one.

Priority connection scheduling
==============================

In |Percona Server| :rn:`5.5.30-30.2` priority connection scheduling for thread pool has been implemented. Even though thread pool puts a limit on the number of concurrently running queries, the number of open transactions may remain high, because connections with already started transactions are put to the end of the queue. Higher number of open transactions has a number of implications on the currently running queries. To improve the performance new :variable:`thread_pool_high_prio_tickets` variable has been introduced.

This variable controls the high priority queue policy. Each new connection is assigned this many tickets to enter the high priority queue. Whenever a query has to be queued to be executed later because no threads are available, the thread pool puts the connection into the high priority queue if the following conditions apply:

  1. The connection has an open transaction in the server.
  2. The number of high priority tickets of this connection is non-zero.

If both the above conditions hold, the connection is put into the high priority queue and its tickets value is decremented. Otherwise the connection is put into the common queue with the initial tickets value specified with this option.

Each time the thread pool looks for a new connection to process, first it checks the high priority queue, and picks connections from the common queue only when the high priority one is empty.

The goal is to minimize the number of open transactions in the server. In many cases it is beneficial to give short-running transactions a chance to commit faster and thus deallocate server resources and locks without waiting in the same queue with other connections that are about to start a new transaction, or those that have run out of their high priority tickets.

With the default value of 0, all connections are always put into the common queue, i.e. no priority scheduling is used as in the original implementation in |MariaDB|. The higher is the value, the more chances each transaction gets to enter the high priority queue and commit before it is put in the common queue.


Version Specific Information
============================

 * :rn:`5.5.29-30.0`
    ``Thread Pool`` feature implemented. This feature was ported from |MariaDB|.

 * :rn:`5.5.30-30.2`
    Implemented priority connection scheduling and introduced new variable :variable:`thread_pool_high_prio_tickets` to the original implementation introduced in |MariaDB|.

System Variables
================

.. variable:: thread_pool_idle_timeout

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: 60 (seconds)

This variable can be used to limit the time an idle thread should wait before exiting.

.. variable:: thread_pool_high_prio_tickets

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: 0

This variable controls the high priority queue policy. Each new connection is assigned this many tickets to enter the high priority queue. 

.. variable:: thread_pool_max_threads

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: 500

This variable can be used to limit the maximum number of threads in the pool. Once this number is reached no new threads will be created.

.. variable:: thread_pool_oversubscribe

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: 3

The higher the value of this parameter the more threads can be run at the same time, if the values is lower than ``3`` it could lead to more sleeps and wake-ups.

.. variable:: thread_pool_size

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: Number of processors

This variable can be used to define the number of threads that can use the CPU at the same time.

.. variable:: thread_pool_stall_limit

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: Numeric
     :default: 500 (ms)

The number of milliseconds before a running thread is considered stalled. When this limit is reached thread pool will wake up or create another thread. This is being used to prevent a long-running query from monopolizing the pool.

.. variable:: extra_port
      
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: Numeric
     :default: 0

This variable can be used to specify additional port |Percona Server| will listen on. This can be used in case no new connections can be established due to all worker threads being busy or being locked when ``pool-of-threads`` feature is enabled. To connect to the extra port following command can be used: ::

  mysql --port='extra-port-number' --protocol=tcp


.. variable:: extra_max_connections
      
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: 1
     
This variable can be used to specify the maximum allowed number of connections on the extra port. This can be used with the :variable:`extra_port` variable to access the server in case no new connections can be established due to all worker threads being busy or being locked when ``pool-of-threads`` feature is enabled.

Status Variables
=====================

.. variable:: Threadpool_idle_threads

     :cli: Yes
     :vartype: Numeric

This status variable shows the number of idle threads in the pool.

.. variable:: Threadpool_threads

     :cli: Yes
     :vartype: Numeric

This status variable shows the number of threads in the pool.

Other Reading
=============

 * `Thread pool in MariaDB 5.5  <https://kb.askmonty.org/en/threadpool-in-55/>`_

 * `Thread pool implementation in Oracle MySQL <http://mikaelronstrom.blogspot.com/2011_10_01_archive.html>`_
