.. _threadpool:

=============
 Thread Pool
=============

|MySQL| executes statements using one thread per client connection. Once the number of connections increases past a certain point performance will degrade. 

This feature enables the server to keep the top performance even with large number of client connections by introducing the dynamic thread pool. By using the thread pool server would decrease the number of threads, which will then reduce the context switching and hot locks contentions. Using the thread pool will have the most effect with ``OLTP`` workloads (relatively short CPU-bound queries). 

In order to enable the thread pool variable :variable:`thread_handling` should be set up to ``pool-of-threads`` value. This can be done by adding: ::

 thread_handling=pool-of-threads

to the |MySQL| configuration file :file:`my.cnf`.

Although the default values for the thread pool should provide good performance, additional `tuning <https://kb.askmonty.org/en/threadpool-in-55/#optimizing-server-variables-on-unix>`_ can be made with the dynamic system variables described below. 

.. note:: 
 
  Current implementation of the thread pool is built in, unlike the upstream version which is implemented as a plugin. Other significant implementation difference is that this implementation doesn't try to minimize the number of concurrent transactions like the ``MySQL Enterprise Threadpool``. Because of these things this implementation isn't compatible with the upstream one.

Version Specific Information
============================

 * :rn:`5.5.29-30.0`
    ``Thread Pool`` feature implemented. This feature was ported from |MariaDB| patches.

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
     :conf: No
     :scope: Global
     :dyn: No
     :vartype: Numeric
     :default: 500 (ms)

The number of milliseconds before a running thread is considered stalled. When this limit is reached thread pool will wake up or create another thread. This is being used to prevent a long-running query from monopolizing the pool.

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
