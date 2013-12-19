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

In |Percona Server| :rn:`5.6.11-60.3` priority connection scheduling for thread pool has been implemented. Even though thread pool puts a limit on the number of concurrently running queries, the number of open transactions may remain high, because connections with already started transactions are put to the end of the queue. Higher number of open transactions has a number of implications on the currently running queries. To improve the performance new :variable:`thread_pool_high_prio_tickets` variable has been introduced.

This variable controls the high priority queue policy. Each new connection is assigned this many tickets to enter the high priority queue. Whenever a query has to be queued to be executed later because no threads are available, the thread pool puts the connection into the high priority queue if the following conditions apply:

  1. The connection has an open transaction in the server.
  2. The number of high priority tickets of this connection is non-zero.

If both the above conditions hold, the connection is put into the high priority queue and its tickets value is decremented. Otherwise the connection is put into the common queue with the initial tickets value specified with this option.

Each time the thread pool looks for a new connection to process, first it checks the high priority queue, and picks connections from the common queue only when the high priority one is empty.

The goal is to minimize the number of open transactions in the server. In many cases it is beneficial to give short-running transactions a chance to commit faster and thus deallocate server resources and locks without waiting in the same queue with other connections that are about to start a new transaction, or those that have run out of their high priority tickets.

The default thread pool behavior is to always put events from already started transactions into the high priority queue, as we believe that results in better performance in vast majority of cases.

With the value of ``0``, all connections are always put into the common queue, i.e. no priority scheduling is used as in the original implementation in |MariaDB|. The higher is the value, the more chances each transaction gets to enter the high priority queue and commit before it is put in the common queue.

In some cases it is required to prioritize all statements for a specific connection regardless of whether they are executed as a part of a multi-statement transaction or in the autocommit mode. Or vice versa, some connections may require using the low priority queue for all statements unconditionally. To implement this new :variable:`thread_pool_high_prio_mode` variable has been introduced in |Percona Server| :rn:`5.6.15-63.0`. 

.. _low_priority_queue_throttling:

Low priority queue throttling
-----------------------------

One case that can limit thread pool performance and even lead to deadlocks under high concurrency is a situation when thread groups are oversubscribed due to active threads reaching the oversubscribe limit, but all/most worker threads are actually waiting on locks currently held by a transaction from another connection that is not currently in the thread pool.

What happens in this case is that those threads in the pool that have marked themselves inactive are not accounted to the oversubscribe limit. As a result, the number of threads (both active and waiting) in the pool grows until it hits :variable:`thread_pool_max_threads` value. If the connection executing the transaction which is holding the lock has managed to enter the thread pool by then, we get a large (depending on the :variable:`thread_pool_max_threads` value) number of concurrently running threads, and thus, suboptimal performance as a result. Otherwise, we get a deadlock as no more threads can be created to process those transaction(s) and release the lock(s).

Such situations are prevented by throttling the low priority queue when the total number of worker threads (both active and waiting ones) reaches the oversubscribe limit. That is, if there are too many worker threads, do not start new transactions and create new threads until queued events from the already started transactions are processed.

Handling of Long Network Waits
==============================

Certain types of workloads (large result sets, BLOBs, slow clients) can have longer waits on network I/O (socket reads and writes). Whenever server waits, this should be communicated to the Thread Pool, so it can start new query by either waking a waiting thread or sometimes creating a new one. This implementation has been ported from |MariaDB| patch `MDEV-156 <https://mariadb.atlassian.net/browse/MDEV-156>`_ in |Percona Server| :rn:`5.6.15-63.0`.


Version Specific Information
============================

 * :rn:`5.6.10-60.2`
    ``Thread Pool`` feature implemented. This feature was ported from |MariaDB|.

 * :rn:`5.6.11-60.3`
    Implemented priority connection scheduling and introduced new variable :variable:`thread_pool_high_prio_tickets` to the original implementation introduced in |MariaDB|.

 * :rn:`5.6.15-63.0`
     Default value for :variable:`thread_pool_max_threads` was changed from ``500`` to ``100 000``. This change was introduced because limiting the total number of threads in the :ref:`threadpool` can result in deadlocks and uneven distribution of worker threads between thread groups in case of stalled connections.

 * :rn:`5.6.15-63.0`
     Default value for :variable:`thread_pool_high_prio_tickets` was changed from ``0`` to ``4294967295`` to enable the priority connection scheduling by default.

 * :rn:`5.6.15-63.0`
     Implemented new :variable:`thread_pool_high_prio_mode` variable.

 * :rn:`5.6.15-63.0`
     Implemented :ref:`low_priority_queue_throttling`.

 * :rn:`5.6.15-63.0`
     Ported |MariaDB| patch `MDEV-156 <https://mariadb.atlassian.net/browse/MDEV-156>`_ to improve Thread Pool behavior when network wait times are not insignificant.
     
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

.. variable:: thread_pool_high_prio_mode

     :cli: Yes
     :conf: Yes
     :scope: Global, Session
     :dyn: Yes
     :vartype: String
     :default: ``transactions``
     :allowed: ``transactions``, ``statements``, ``none``

This variable is used to provide more fine-grained control over high priority scheduling either globally or per connection.

The following values are allowed:

  * ``transactions`` (the default). In this mode only statements from already started transactions may go into the high priority queue depending on the number of high priority tickets currently available in a connection (see :variable:`thread_pool_high_prio_tickets`).

  * ``statements``. In this mode all individual statements go into the high priority queue, regardless of connection's transactional state and the number of available high priority tickets. This value can be used to prioritize ``AUTOCOMMIT`` transactions or other kinds of statements such as administrative ones for specific connections. Note that setting this value globally essentially disables high priority scheduling, since in this case all statements from all connections will use a single queue (the high priority one)

  * ``none``. This mode disables high priority queue for a connection. Some connections (e.g. monitoring) may be insensitive to execution latency and/or never allocate any server resources that would otherwise impact performance in other connections and thus, do not really require high priority scheduling. Note that setting :variable:`thread_pool_high_prio_mode` to ``none`` globally has essentially the same effect as setting it to ``statements`` globally: all connections will always use a single queue (the low priority one in this case).

.. variable:: thread_pool_high_prio_tickets

     :cli: Yes
     :conf: Yes
     :scope: Global, Session
     :dyn: Yes
     :vartype: Numeric
     :default: 4294967295

This variable controls the high priority queue policy. Each new connection is assigned this many tickets to enter the high priority queue. Setting this variable to ``0`` will disable the high priority queue.

.. variable:: thread_pool_max_threads

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: 100000

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
