.. _user_stats:

=================
 User Statistics
=================

This feature adds several ``INFORMATION_SCHEMA`` tables, several commands, and the userstat variable. The tables and commands can be used to understand the server activity better and identify the source of the load.

The functionality is disabled by default, and must be enabled by setting ``userstat`` to ``ON``. It works by keeping several hash tables in memory. To avoid contention over global mutexes, each connection has its own local statistics, which are occasionally merged into the global statistics, and the local statistics are then reset to 0.


Version Specific Information
============================

  * :rn:`5.5.10-20.1`:
     Renamed variable :variable:`userstat_running` to :variable:`userstat`.
  * :rn:`5.5.24-26.0`:
     TOTAL_SSL_CONNECTIONS column has been added to CLIENT_STATISTICS, THREAD_STATISTICS and USER_STATISTICS tables 

Other Information
=================

  * Author/Origin:
     *Google*; *Percona* added the ``INFORMATION_SCHEMA`` tables and the :variable:`userstat_running` variable.

System Variables
================

.. variable:: userstat

     :version 5.5.10-20.1: Renamed from :variable:`userstat_running`
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: BOOLEAN
     :default: OFF
     :range: ON/OFF

Enables or disables collection of statistics. The default is ``OFF``, meaning no statistics are gathered. This is to ensure that the statistics collection doesn't cause any extra load on the server unless desired.

.. variable:: thread_statistics

     :version 5.5.8-20.0: Feature ported from |Percona Server| 5.1
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: BOOLEAN
     :default: OFF
     :range: ON/OFF

Enables or disables collection of thread statistics. The default is ``OFF``, meaning no thread statistics are gathered. This is to ensure that the statistics collection doesn't cause any extra load on the server unless desired. Variable :variable:`userstat` needs to be enabled as well in order for thread statistics to be collected.

.. note::

 In Percona Server 5.5 :variable:`thread_statistics` is a reserved word. Which means you have to quote it when using the system variable with the same name: ``mysql> set global `thread_statistics`=1;``

INFORMATION_SCHEMA Tables
=========================

.. table:: INFORMATION_SCHEMA.CLIENT_STATISTICS

  :column CLIENT: The IP address or hostname from which the connection originated.
  :column TOTAL_CONNECTIONS: The number of connections created for this client.
  :column CONCURRENT_CONNECTIONS: The number of concurrent connections for this client.
  :column CONNECTED_TIME: The cumulative number of seconds elapsed while there were connections from this client.
  :column BUSY_TIME: The cumulative number of seconds there was activity on connections from this client.
  :column CPU_TIME: The cumulative CPU time elapsed, in seconds, while servicing this client``s connections.
  :column BYTES_RECEIVED: The number of bytes received from this client's connections.
  :column BYTES_SENT: The number of bytes sent to this client's connections.
  :column BINLOG_BYTES_WRITTEN:	The number of bytes written to the binary log from this client's connections.
  :column ROWS_FETCHED: The number of rows fetched by this client's connections.
  :column ROWS_UPDATED: The number of rows updated by this client's connections.
  :column TABLE_ROWS_READ: The number of rows read from tables by this client's connections. (It may be different from ``ROWS_FETCHED``.)
  :column SELECT_COMMANDS: The number of ``SELECT`` commands executed from this client's connections.
  :column UPDATE_COMMANDS: The number of ``UPDATE`` commands executed from this client's connections.
  :column OTHER_COMMANDS: The number of other commands executed from this client's connections.
  :column COMMIT_TRANSACTIONS: The number of ``COMMIT`` commands issued by this client's connections.
  :column ROLLBACK_TRANSACTIONS: The number of ``ROLLBACK`` commands issued by this client's connections.
  :column DENIED_CONNECTIONS: The number of connections denied to this client.
  :column LOST_CONNECTIONS: The number of this client's connections that were terminated uncleanly.
  :column ACCESS_DENIED: The number of times this client's connections issued commands that were denied.
  :column EMPTY_QUERIES: The number of times this client's connections sent empty queries to the server.
  :column TOTAL_SSL_CONNECTIONS: The number of times this client's connections connected using SSL to the server.


This table holds statistics about client connections. The Percona version of the feature restricts this table's visibility to users who have the ``SUPER`` or ``PROCESS`` privilege.

Example: ::

  mysql> SELECT * FROM INFORMATION_SCHEMA.CLIENT_STATISTICS\G
  *************************** 1. row ***************************
                  CLIENT: 10.1.12.30
       TOTAL_CONNECTIONS: 20
  CONCURRENT_CONNECTIONS: 0
          CONNECTED_TIME: 0
               BUSY_TIME: 93
                CPU_TIME: 48
          BYTES_RECEIVED: 5031
              BYTES_SENT: 276926
    BINLOG_BYTES_WRITTEN: 217
            ROWS_FETCHED: 81
            ROWS_UPDATED: 0
         TABLE_ROWS_READ: 52836023
         SELECT_COMMANDS: 26
         UPDATE_COMMANDS: 1
          OTHER_COMMANDS: 145
     COMMIT_TRANSACTIONS: 1
   ROLLBACK_TRANSACTIONS: 0
      DENIED_CONNECTIONS: 0
        LOST_CONNECTIONS: 0
           ACCESS_DENIED: 0
           EMPTY_QUERIES: 0
   TOTAL_SSL_CONNECTIONS: 0


.. table:: INFORMATION_SCHEMA.INDEX_STATISTICS

  :column TABLE_SCHEMA: The schema (database) name.
  :column TABLE_NAME: The table name.
  :column INDEX_NAME: The index name (as visible in ``SHOW CREATE TABLE``).
  :column ROWS_READ: The number of rows read from this index.

This table shows statistics on index usage. An older version of the feature contained a single column that had the ``TABLE_SCHEMA``, ``TABLE_NAME`` and ``INDEX_NAME`` columns concatenated together. The |Percona| version of the feature separates these into three columns. Users can see entries only for tables to which they have ``SELECT`` access.

This table makes it possible to do many things that were difficult or impossible previously. For example, you can use it to find unused indexes and generate DROP commands to remove them. If the index has not been used it won't be in this table.

Example: ::

  mysql> SELECT * FROM INFORMATION_SCHEMA.INDEX_STATISTICS
     WHERE TABLE_NAME='tables_priv';
  +--------------+-----------------------+--------------------+-----------+
  | TABLE_SCHEMA | TABLE_NAME            | INDEX_NAME         | ROWS_READ |
  +--------------+-----------------------+--------------------+-----------+
  | mysql        | tables_priv           | PRIMARY            |         2 |
  +--------------+-----------------------+--------------------+-----------+

.. note:: 

   Current implementation of index statistics doesn't support partitioned tables.


.. table:: INFORMATION_SCHEMA.TABLE_STATISTICS

  :column TABLE_SCHEMA: The schema (database) name.
  :column TABLE_NAME: The table name.
  :column ROWS_READ: The number of rows read from the table.
  :column ROWS_CHANGED: The number of rows changed in the table.
  :column ROWS_CHANGED_X_INDEXES: The number of rows changed in the table, multiplied by the number of indexes changed.

This table is similar in function to the ``INDEX_STATISTICS`` table.

Example: ::

  mysql> SELECT * FROM INFORMATION_SCHEMA.TABLE_STATISTICS
     WHERE TABLE_NAME=``tables_priv``;
  +--------------+-------------------------------+-----------+--------------+------------------------+
  | TABLE_SCHEMA | TABLE_NAME                    | ROWS_READ | ROWS_CHANGED | ROWS_CHANGED_X_INDEXES |
  +--------------+-------------------------------+-----------+--------------+------------------------+
  | mysql        | tables_priv                   |         2 |            0 |                      0 | 
  +--------------+-------------------------------+-----------+--------------+------------------------+

.. note:: 

   Current implementation of table statistics doesn't support partitioned tables.

.. table:: INFORMATION_SCHEMA.THREAD_STATISTICS

  :column THREAD_ID: Thread ID
  :column TOTAL_CONNECTIONS: The number of connections created from this thread.
  :column CONCURRENT_CONNECTIONS: The number of concurrent connections from this thread.
  :column CONNECTED_TIME: The cumulative number of seconds elapsed while there were connections from this thread.
  :column BUSY_TIME: The cumulative number of seconds there was activity from this thread.
  :column CPU_TIME: The cumulative CPU time elapsed while servicing this thread.
  :column BYTES_RECEIVED: The number of bytes received from this thread.
  :column BYTES_SENT: The number of bytes sent to this thread.
  :column BINLOG_BYTES_WRITTEN: The number of bytes written to the binary log from this thread.
  :column ROWS_FETCHED: The number of rows fetched by this thread.
  :column ROWS_UPDATED: The number of rows updated by this thread.
  :column TABLE_ROWS_READ: The number of rows read from tables by this tread.
  :column SELECT_COMMANDS: The number of ``SELECT`` commands executed from this thread.
  :column UPDATE_COMMANDS: The number of ``UPDATE`` commands executed from this thread.
  :column OTHER_COMMANDS: The number of other commands executed from this thread.
  :column COMMIT_TRANSACTIONS: The number of ``COMMIT`` commands issued by this thread.
  :column ROLLBACK_TRANSACTIONS: The number of ``ROLLBACK`` commands issued by this thread.
  :column DENIED_CONNECTIONS: The number of connections denied to this thread.
  :column LOST_CONNECTIONS: The number of thread connections that were terminated uncleanly.
  :column ACCESS_DENIED: The number of times this thread issued commands that were denied.
  :column EMPTY_QUERIES: The number of times this thread sent empty queries to the server.
  :column TOTAL_SSL_CONNECTIONS:  The number of thread connections that used SSL.

In order for this table to be populated with statistics, additional variable :variable:`thread_statistics` should be set to ``ON``.

.. table:: INFORMATION_SCHEMA.USER_STATISTICS

  :column USER: The username. The value ``#mysql_system_user#`` appears when there is no username (such as for the slave SQL thread).
  :column TOTAL_CONNECTIONS: The number of connections created for this user.
  :column CONCURRENT_CONNECTIONS: The number of concurrent connections for this user.
  :column CONNECTED_TIME: The cumulative number of seconds elapsed while there were connections from this user.
  :column BUSY_TIME: The cumulative number of seconds there was activity on connections from this user.
  :column CPU_TIME: The cumulative CPU time elapsed, in seconds, while servicing this user's connections.
  :column BYTES_RECEIVED: The number of bytes received from this user's connections.
  :column BYTES_SENT: The number of bytes sent to this user's connections.
  :column BINLOG_BYTES_WRITTEN: The number of bytes written to the binary log from this user's connections.
  :column ROWS_FETCHED: The number of rows fetched by this user's connections.
  :column ROWS_UPDATED: The number of rows updated by this user's connections.
  :column TABLE_ROWS_READ: The number of rows read from tables by this user's connections. (It may be different from ``ROWS_FETCHED``.)
  :column SELECT_COMMANDS: The number of ``SELECT`` commands executed from this user's connections.
  :column UPDATE_COMMANDS: The number of ``UPDATE`` commands executed from this user's connections.
  :column OTHER_COMMANDS: The number of other commands executed from this user's connections.
  :column COMMIT_TRANSACTIONS: The number of ``COMMIT`` commands issued by this user's connections.
  :column ROLLBACK_TRANSACTIONS: The number of ``ROLLBACK`` commands issued by this user's connections.
  :column DENIED_CONNECTIONS: The number of connections denied to this user.
  :column LOST_CONNECTIONS: The number of this user's connections that were terminated uncleanly.
  :column ACCESS_DENIED: The number of times this user's connections issued commands that were denied.
  :column EMPTY_QUERIES: The number of times this user's connections sent empty queries to the server.
  :column TOTAL_SSL_CONNECTIONS: The number of times this user's connections connected using SSL to the server.

This table contains information about user activity. The |Percona| version of the patch restricts this table's visibility to users who have the ``SUPER`` or ``PROCESS`` privilege.

The table gives answers to questions such as which users cause the most load, and whether any users are being abusive. It also lets you measure how close to capacity the server may be. For example, you can use it to find out whether replication is likely to start falling behind.

Example: ::

  mysql> SELECT * FROM INFORMATION_SCHEMA.USER_STATISTICS\G
  *************************** 1. row ***************************
                    USER: root
       TOTAL_CONNECTIONS: 5592
  CONCURRENT_CONNECTIONS: 0
          CONNECTED_TIME: 6844
               BUSY_TIME: 179
                CPU_TIME: 72
          BYTES_RECEIVED: 603344
              BYTES_SENT: 15663832
    BINLOG_BYTES_WRITTEN: 217
            ROWS_FETCHED: 9793
            ROWS_UPDATED: 0
         TABLE_ROWS_READ: 52836023
         SELECT_COMMANDS: 9701
         UPDATE_COMMANDS: 1
          OTHER_COMMANDS: 2614
     COMMIT_TRANSACTIONS: 1
   ROLLBACK_TRANSACTIONS: 0
      DENIED_CONNECTIONS: 0
        LOST_CONNECTIONS: 0
           ACCESS_DENIED: 0
           EMPTY_QUERIES: 0
   TOTAL_SSL_CONNECTIONS: 0


Commands Provided
=================

  * ``FLUSH CLIENT_STATISTICS``

  * ``FLUSH INDEX_STATISTICS``

  * ``FLUSH TABLE_STATISTICS``

  * ``FLUSH THREAD_STATISTICS``

  * ``FLUSH USER_STATISTICS``

These commands discard the specified type of stored statistical information.

  * ``SHOW CLIENT_STATISTICS``
  * ``SHOW INDEX_STATISTICS``
  * ``SHOW TABLE_STATISTICS``
  * ``SHOW THREAD_STATISTICS``
  * ``SHOW USER_STATISTICS``

These commands are another way to display the information you can get from the ``INFORMATION_SCHEMA`` tables. The commands accept ``WHERE`` clauses. They also accept but ignore ``LIKE`` clauses.


