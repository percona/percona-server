.. _threadpool:

=============
 Thread Pool
=============

|MySQL| assigns one thread for each client connection which executes every statement for that connection. As the number of connections increase and each connection executes a series of statements, the performance deteriorates. Threads cost RAM. Increasing the number of threads can cause other issues, such as resource contention or excessive context switching.

A dynamic thread pool enables the server to maintain top performance with a large number of clients. A thread pool decreases the number of threads on the server, which also reduces the context switching and hot locks contentions. A thread pool has the most effect with ``OLTP`` workloads (relatively short CPU-bound queries).

In order to enable the thread pool, the variable :variable:`thread_handling` should be set up to ``pool-of-threads`` value. This can be done by adding this setting to the |MySQL| configuration file :file:`my.cnf`: ::

 thread_handling=pool-of-threads

.. seealso::

   |MySQL| Documentation
   `thread-handling <https://dev.mysql.com/doc/refman/8.0/en/server-system-variables.html#sysvar_thread_handling>`_

Although the default values for the thread pool should provide good performance, you can perform additional tuning with dynamic system variables described in system-variables_.

.. note::
 
 Unlike the upstream version, which is installed as a plugin, the current thread pool operation is built in the server. Another significant difference is that this thread pool version does not attempt to minimize the number of concurrent transactions like the ``MySQL Enterprise Threadpool``. Due to these differences, this implementation is not compatible with the upstream one.

Priority connection scheduling
==============================

Though the thread pool limits the number of concurrently running queries, the number of open transactions may remain high. The queue puts connections with already started transactions at the end. A high number of open transactions has several implications on the currently running queries.

The :variable:`thread_pool_high_prio_tickets` variable controls the high priority queue policy. If set to ``0``, the thread pool adds all connections to the common queue with no priority scheduling. A higher value provides more chances for each transaction to enter the high priority queue.

Each new connection is assigned this many tickets to enter the high priority queue. Whenever a query is queued to be executed later because no threads are available, the thread pool puts the connection into the high priority queue, and its ticket's value is decremented if the following conditions apply:

  * Connection has an open transaction in the server
  * Connection contains a non-zero number of high priority tickets

Otherwise, the connection is put into the common queue with the initial ticket value specified with this variable. When the thread pool processes a new connection, the thread pool checks the high priority queue, and, if that queue is empty, selects connections from the common queue.

This process minimizes the number of open transactions in the server. The server resources benefit by allowing short-running transactions which commit faster. The completion of these transactions deallocate resources and locks.

The default thread pool behavior always puts events from already started transactions into the high priority queue, as we believe that results in better performance in a majority of cases.



In some cases it is required to prioritize all statements for a specific connection regardless of whether they are executed as a part of a multi-statement transaction or in the autocommit mode. Or vice versa, some connections may require using the low priority queue for all statements unconditionally. To implement this new :variable:`thread_pool_high_prio_mode` variable has been introduced in |Percona Server|.

.. _low_priority_queue_throttling:

Low priority queue throttling
-----------------------------

One case that can limit thread pool performance and even lead to deadlocks under high concurrency is when thread groups are oversubscribed due to active threads reaching the oversubscribe limit, but all/most worker threads are waiting on locks currently held by a transaction from another connection that is not currently in the thread pool.

In this case, those threads in the pool that have marked themselves inactive are not accounted to the oversubscribe limit. As a result, the number of threads (both active and waiting) in the pool grows until it hits :variable:`thread_pool_max_threads` value. If the connection executing the transaction holding the lock has managed to enter the thread pool by then, we have a large (depending on the :variable:`thread_pool_max_threads` value) number of concurrently running threads, and thus, sub-optimal performance as a result. Otherwise, we have a deadlock since no more threads are created to process those transaction(s) and release the lock(s).

Such situations are prevented by throttling the low priority queue when the total number of worker threads (both active and waiting ones) reaches the oversubscribe limit. That is, if there are too many worker threads, do not start new transactions and create new threads until queued events from the already started transactions are processed.

Handling of Long Network Waits
==============================

Certain workloads (large result sets, BLOBs, slow clients) can have long waits on the network I/O (socket reads and writes). Whenever the server waits, this should be communicated to the thread pool, so it can start new query by either waking a waiting thread or sometimes creating a new one. This implementation has been ported from |MariaDB| patch `MDEV-156 <https://mariadb.atlassian.net/browse/MDEV-156>`_.

Version Specific Information
============================

 * :rn:`8.0.12-1`
    ``Thread Pool`` feature ported from |Percona Server| 5.7.


.. _system-variables:

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

.. list-table::
    :widths: 15 25
    :header-rows: 1
    
    * - Values
      - Description
    * - ``transactions``
      - The default value. In this mode only statements from already started   transactions may go into the high priority queue depending on the number of high priority tickets currently available in a connection (see :variable:`thread_pool_high_prio_tickets`).
    * - ``statements``
      - In this mode all individual statements go into the high priority queue, regardless of connection's transactional state and the number of available high priority tickets. This value can be used to prioritize ``AUTOCOMMIT`` transactions or other kinds of statements such as administrative ones for specific connections. Note that setting this value globally essentially disables high priority scheduling, since in this case all statements from all connections will use a single queue (the high priority queue)
    * - ``none``
      - This mode disables the high priority queue for a connection. Some connections (e.g. monitoring) may be insensitive to execution latency and/or never allocate any server resources that would otherwise impact performance in other connections and thus, do not really require high priority scheduling. Note that setting :variable:`thread_pool_high_prio_mode` to ``none`` globally has the same effect as setting it to ``statements`` globally: all connections will always use a single queue (the low priority one in this case).

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

.. rubric:: Upgrading from a version before 8.0.15-5 to 8.0.15-5 or higher

Starting with the release of version `8.0.15-5`, |Percona Server| uses the upstream implementation of the `admin_port <https://dev.mysql.com/doc/refman/8.0/en/server-system-variables.html#sysvar_admin_port>`_. The variables :variable:`extra_port` and :variable:`extra_max_connections` are removed and not supported. It is essential to remove the ``extra_port`` and ``extra_max_connections`` variables from your configuration file before you attempt to upgrade from a release before `8.0.15-5` to |Percona Server| version `8.0.15-5` or higher. Otherwise, the server produces a boot error and refuses to start.

.. variable:: extra_port

     :version_info: removed in `8.0.15-5`
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: Numeric
     :default: 0

This variable can be used to specify an additional port for |Percona Server| to listen on. This port can be used in case no new connections can be established due to all worker threads being busy or being locked when ``pool-of-threads`` feature is enabled.

To connect to the extra port following command can be used:

.. code-block:: bash

  mysql --port='extra-port-number' --protocol=tcp


.. variable:: extra_max_connections

     :version_info: removed in `8.0.15-5`
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: 1

This variable can be used to specify the maximum allowed number of connections plus one extra ``SUPER`` users connection on the :variable:`extra_port`. This can be used with the :variable:`extra_port` variable to access the server in case no new connections can be established due to all worker threads being busy or being locked when ``pool-of-threads`` feature is enabled.

Status Variables
=====================

.. variable:: Threadpool_idle_threads

     :vartype: Numeric
     :scope: Global

This status variable shows the number of idle threads in the pool.

.. variable:: Threadpool_threads

     :vartype: Numeric
     :scope: Global

This status variable shows the number of threads in the pool.

Other Reading
=============

 * `Thread pool in MariaDB 5.5  <https://kb.askmonty.org/en/threadpool-in-55/>`_

 * `Thread pool implementation in Oracle MySQL <http://mikaelronstrom.blogspot.com/2011_10_01_archive.html>`_
