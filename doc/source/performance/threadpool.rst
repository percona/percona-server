.. _threadpool:

=============
 Thread Pool
=============


This feature enables the server to keep the top performance even with a large
number of client connections by introducing a dynamic thread pool. By using the
thread pool server would decrease the number of threads, which will then reduce
the context switching and hot locks contentions. Using the thread pool will have
the most effect with ``OLTP`` workloads (relatively short CPU-bound queries).

The |thread-pool| feature introduces a dynamic |thread-pool| that enables the
server to keep the top performance even with a large number of client
connections. With |thread-pool| enabled, the server decreases the number of
threads, which then reduces the context switching and hot locks
contentions. Using the |thread-pool| has the most effect on ``OLTP`` workloads
(relatively short CPU-bound queries).

To enable the |thread-pool| feature, the :variable:`thread_handling` variable
should be set to the ``pool-of-threads`` value. This can be done by adding the
following to the MySQL configuration file :file:`my.cnf`: ::

 thread_handling=pool-of-threads

Although the default values for the |thread-pool| should provide good
performance, additional tuning
can be performed with the dynamic system variables described in :ref:`tuning`.

.. important:: 
 
  The current implementation of the |thread-pool| feature is built into the
  server, unlike the upstream version which is implemented as a plugin. Another
  significant implementation difference is that this implementation doesn't try
  to minimize the number of concurrent transactions like the ``MySQL Enterprise
  Threadpool``. Because of these differences, this implementation is not
  compatible with the upstream implementation.

Priority connection scheduling
==============================

Even though thread pool puts a limit on the number of concurrently running queries, the number of open transactions may remain high, because connections with already started transactions are put to the end of the queue. Higher number of open transactions has a number of implications on the currently running queries. To improve the performance new :variable:`thread_pool_high_prio_tickets` variable has been introduced.

This variable controls the high priority queue policy. Each new connection is assigned this many tickets to enter the high priority queue. Whenever a query has to be queued to be executed later because no threads are available, the thread pool puts the connection into the high priority queue if the following conditions apply:

  1. The connection has an open transaction in the server.
  2. The number of high priority tickets of this connection is non-zero.

If both the above conditions hold, the connection is put into the high priority queue and its tickets value is decremented. Otherwise the connection is put into the common queue with the initial tickets value specified with this option.

Each time the thread pool looks for a new connection to process, first it checks the high priority queue, and picks connections from the common queue only when the high priority one is empty.

The goal is to minimize the number of open transactions in the server. In many cases it is beneficial to give short-running transactions a chance to commit faster and thus deallocate server resources and locks without waiting in the same queue with other connections that are about to start a new transaction, or those that have run out of their high priority tickets.

The default thread pool behavior is to always put events from already started transactions into the high priority queue, as we believe that results in better performance in vast majority of cases.

With the value of ``0``, all connections are always put into the common queue, i.e. no priority scheduling is used as in the original implementation in MariaDB. The higher is the value, the more chances each transaction gets to enter the high priority queue and commit before it is put in the common queue.

In some cases it is required to prioritize all statements for a specific connection regardless of whether they are executed as a part of a multi-statement transaction or in the autocommit mode. Or vice versa, some connections may require using the low priority queue for all statements unconditionally. To implement this new :variable:`thread_pool_high_prio_mode` variable has been introduced in |Percona Server|. 

.. _low_priority_queue_throttling:

Low priority queue throttling
-----------------------------

One case that can limit the performance of |thread-pool| and even lead to
deadlocks under high concurrency is the situation when thread groups are
oversubscribed due to active threads reaching the oversubscribe limit, but all
or most worker threads are actually waiting on locks currently held by a
transaction from another connection that is not currently in the |thread-pool|.

In this case, those threads in the pool that have marked themselves inactive are
not accounted to the oversubscribe limit. As a result, the number of threads
(both active and waiting) in the pool grows until it hits
:variable:`thread_pool_max_threads` value. If the connection executing the
transaction which is holding the lock has managed to enter the |thread-pool| by
then, we get a large (depending on the :variable:`thread_pool_max_threads`
value) number of concurrently running threads, and thus, suboptimal performance
as a result. Otherwise, we get a deadlock as no more threads can be created to
process those transactions and release the lock.

Such situations are prevented by throttling the low priority queue when the
total number of worker threads (both active and waiting ones) reaches the
oversubscribe limit. That is, if there are too many worker threads, do not start
new transactions and create new threads until queued events from the already
started transactions are processed.

Handling of Long Network Waits
==============================

Certain types of workloads (large result sets, BLOBs, slow clients) can have longer waits on network I/O (socket reads and writes). Whenever server waits, this should be communicated to the Thread Pool, so it can start new query by either waking a waiting thread or sometimes creating a new one. This implementation has been ported from MariaDB patch `MDEV-156 <https://mariadb.atlassian.net/browse/MDEV-156>`_. 


Version Specific Information
============================

 * :rn:`5.7.10-1`
    ``Thread Pool`` feature ported from |Percona Server| 5.6.
    
.. _tuning:

System Variables
================

.. variable:: thread_handling

    :cli: Yes
    :conf: Yes
    :scope: Global
    :dyn: No
    :vartype: String
    :default: one-thread-per-connection
    
This variable defines how the server handles threads for connections from the client.

.. list-table::
    :widths: 30 30
    :header-rows: 1
    
    * - Values
      - Description
    * - one-thread-per-connection
      - One thread handles all requests for a connection
    * - pool-of-threads
      - A thread pool handles requests for all connections
    * - no-threads
      - A single thread for all connections for debugging mode

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

This variable is used to provide more fine-grained control over high priority
scheduling either globally or per connection.

The following values are allowed:

  * ``transactions`` (the default). In this mode, only statements from already
    started transactions may go into the high priority queue depending on the
    number of high priority tickets currently available in a connection (see
    :variable:`thread_pool_high_prio_tickets`).

  * ``statements``. In this mode, all individual statements go into the high
    priority queue, regardless of connection's transactional state and the
    number of available high priority tickets. This value can be used to
    prioritize ``AUTOCOMMIT`` transactions or other kinds of statements such as
    administrative ones for specific connections. Note that setting this value
    globally essentially disables high priority scheduling, since in this case
    all statements from all connections will use a single queue (the high
    priority one)

  * ``none``. This mode disables high priority queue for a connection. Some
    connections (e.g. monitoring) may be insensitive to execution latency and/or
    never allocate any server resources that would otherwise impact performance
    in other connections and thus, do not really require high priority
    scheduling. Note that setting :variable:`thread_pool_high_prio_mode` to
    ``none`` globally has essentially the same effect as setting it to
    ``statements`` globally: all connections will always use a single queue (the
    low priority one in this case).

.. variable:: thread_pool_high_prio_tickets

     :cli: Yes
     :conf: Yes
     :scope: Global, Session
     :dyn: Yes
     :vartype: Numeric
     :default: 4294967295

This variable controls the high priority queue policy. Each new connection is
assigned this many tickets to enter the high priority queue. Setting this
variable to ``0`` disables the high priority queue.

.. variable:: thread_pool_max_threads

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: 100000

This variable can be used to limit the maximum number of threads in the
pool. Once this number is reached no new threads will be created.

.. variable:: thread_pool_oversubscribe

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: 3

The higher the value of this parameter the more threads can be run at the same
time, if the values is lower than ``3`` it could lead to more sleeps and
wake-ups.

.. variable:: thread_pool_size

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: Number of processors

This variable can be used to define the number of threads that can use the CPU
at the same time.

.. variable:: thread_pool_stall_limit

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: Numeric
     :default: 500 (ms)

The number of milliseconds before a running thread is considered stalled. When
this limit is reached thread pool will wake up or create another thread. This is
being used to prevent a long-running query from monopolizing the pool.

.. variable:: extra_port
      
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: Numeric
     :default: 0

This variable can be used to specify an additional port that |Percona Server|
will listen on. This can be used in case no new connections can be established
due to all worker threads being busy or being locked when ``pool-of-threads``
feature is enabled. To connect to the extra port the following command can be
used: ::

  mysql --port='extra-port-number' --protocol=tcp

.. Question:

   The port number assigned to this variable must be different from the value of
   the *port* server variable.

.. variable:: extra_max_connections
      
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: Numeric
     :default: 1
     
This variable can be used to specify the maximum allowed number of connections
plus one extra ``SUPER`` users connection on the :variable:`extra_port`. This
can be used with the :variable:`extra_port` variable to access the server in
case no new connections can be established due to all worker threads being busy
or being locked when ``pool-of-threads`` feature is enabled.

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

.. note::

   When |thread-pool| is enabled, the value of the :variable:`thread_cache_size`
   variable is ignored. The :variable:`Threads_cached` status variable contains
   ``0`` in this case.


Other Reading
=============

 * `Thread pool in MariaDB 5.5  <https://kb.askmonty.org/en/threadpool-in-55/>`_

 * `Thread pool implementation in Oracle MySQL <http://mikaelronstrom.blogspot.com/2011_10_01_archive.html>`_

.. |thread-pool| replace:: *thread pool*
