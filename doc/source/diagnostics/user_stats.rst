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

Other Information
=================

  * Author/Origin:
     *Google*; *Percona* added the ``INFORMATION_SCHEMA`` tables and the :variable:`userstat_running` variable.

System Variables
================

.. variable:: userstat_running

     :version 5.5.10-20.1: Renamed to :variable:`userstat`
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: BOOLEAN
     :default: OFF
     :range: ON/OFF

Enables or disables collection of statistics. The default is ``OFF``, meaning no statistics are gathered. This is to ensure that the statistics collection doesn't cause any extra load on the server unless desired.


INFORMATION_SCHEMA Tables
=========================

.. table:: INFORMATION_SCHEMA.CLIENT_STATISTICS

  :column CLIENT: The IP address or hostname from which the connection originated.
  :column TOTAL_CONNECTIONS: The number of connections created for this client.
  :column CONCURRENT_CONNECTIONS: The number of concurrent connections for this client.
  :column CONNECTED_TIME: The cumulative number of seconds elapsed while there were connections from this client.
  :column BUSY_TIME: The cumulative number of seconds there was activity on connections from this client.
  :column CPU_TIME: The cumulative CPU time elapsed while servicing this client``s connections.
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


.. table:: INFORMATION_SCHEMA.INDEX_STATISTICS

  :column TABLE_SCHEMA: The schema (database) name.
  :column TABLE_NAME: The table name.
  :column INDEX_NAME: The index name (as visible in ``SHOW CREATE TABLE``).
  :column ROWS_READ: The number of rows read from this index.

This table shows statistics on index usage. An older version of the feature contained a single column that had the ``TABLE_SCHEMA``, ``TABLE_NAME`` and ``INDEX_NAME`` columns concatenated together. The |Percona| version of the feature separates these into three columns. Users can see entries only for tables to which they have ``SELECT`` access.

This table makes it possible to do many things that were difficult or impossible previously. For example, you can use it to find unused indexes and generate DROP commands to remove them.

Example: ::

  mysql> SELECT * FROM INFORMATION_SCHEMA.INDEX_STATISTICS
     WHERE TABLE_NAME='tables_priv';
  +--------------+-----------------------+--------------------+-----------+
  | TABLE_SCHEMA | TABLE_NAME            | INDEX_NAME         | ROWS_READ |
  +--------------+-----------------------+--------------------+-----------+
  | mysql        | tables_priv           | PRIMARY            |         2 |
  +--------------+-----------------------+--------------------+-----------+



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


.. table:: INFORMATION_SCHEMA.THREAD_STATISTICS

  :column THREAD_ID: int(21)
  :column TOTAL_CONNECTIONS: int(21)
  :column CONCURRENT_CONNECTIONS: int(21)
  :column CONNECTED_TIME: int(21)
  :column BUSY_TIME: int(21)
  :column CPU_TIME: int(21)
  :column BYTES_RECEIVED: int(21)
  :column BYTES_SENT: int(21)
  :column BINLOG_BYTES_WRITTEN: int(21)
  :column ROWS_FETCHED: int(21)
  :column ROWS_UPDATED: int(21)
  :column TABLE_ROWS_READ: int(21)
  :column SELECT_COMMANDS: int(21)
  :column UPDATE_COMMANDS: int(21)
  :column OTHER_COMMANDS: int(21)
  :column COMMIT_TRANSACTIONS: int(21)
  :column ROLLBACK_TRANSACTIONS: int(21)
  :column DENIED_CONNECTIONS: int(21)
  :column LOST_CONNECTIONS: int(21)
  :column ACCESS_DENIED: int(21)
  :column EMPTY_QUERIES: int(21)

.. table:: INFORMATION_SCHEMA.USER_STATISTICS

  :column USER: The username. The value ``#mysql_system_user#`` appears when there is no username (such as for the slave SQL thread).
  :column TOTAL_CONNECTIONS: The number of connections created for this user.
  :column CONCURRENT_CONNECTIONS: The number of concurrent connections for this user.
  :column CONNECTED_TIME: The cumulative number of seconds elapsed while there were connections from this user.
  :column BUSY_TIME: The cumulative number of seconds there was activity on connections from this user.
  :column CPU_TIME: The cumulative CPU time elapsed while servicing this user's connections.
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


