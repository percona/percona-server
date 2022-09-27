.. _ps_versions_comparison:

========================================================
List of features available in |Percona Server| releases
========================================================

.. tabularcolumns:: p{5cm}|p{5cm}|

.. list-table::
   :header-rows: 1

   * - |Percona Server| 5.7
     - |Percona Server| 8.0
   * - Improved Buffer Pool Scalability
     - Improved Buffer Pool Scalability
   * - Improved InnoDB I/O Scalability
     - Improved InnoDB I/O Scalability
   * - Multiple Adaptive Hash Search Partitions
     - Multiple Adaptive Hash Search Partitions
   * - Atomic write support for Fusion-io devices
     - Atomic write support for Fusion-io devices
   * - Query Cache Enhancements
     - |-implemented|
   * - Improved NUMA support
     - |-implemented|
   * - Thread Pool 
     - Thread Pool
   * - Suppress Warning Messages
     - Suppress Warning Messages
   * - Ability to change database for mysqlbinlog
     - Ability to change database for mysqlbinlog
   * - Fixed Size for the Read Ahead Area
     - Fixed Size for the Read Ahead Area
   * - Improved MEMORY Storage Engine 
     - Improved MEMORY Storage Engine 
   * - Restricting the number of binlog files 
     - Restricting the number of binlog files
   * - Ignoring missing tables in mysqldump
     - Ignoring missing tables in mysqldump
   * - Too Many Connections Warning
     - Too Many Connections Warning
   * - Handle Corrupted Tables
     - Handle Corrupted Tables
   * - Lock-Free SHOW SLAVE STATUS
     - Lock-Free SHOW REPLICA STATUS
   * - Expanded Fast Index Creation
     - Expanded Fast Index Creation
   * - Percona Toolkit UDFs
     - Percona Toolkit UDFs
   * - Support for Fake Changes
     - Support for Fake Changes
   * - Kill Idle Transactions
     - Kill Idle Transactions
   * - XtraDB changed page tracking
     - XtraDB changed page tracking
   * - Enforcing Storage Engine
     - |replaced|
   * - Utility user
     - Utility user
   * - Extending the secure-file-priv server option
     - Extending the secure-file-priv server option
   * - Expanded Program Option Modifiers
     - |-implemented|
   * - PAM Authentication Plugin
     - PAM Authentication Plugin
   * - Log Archiving for XtraDB
     - Log Archiving for XtraDB
   * - User Statistics
     - User Statistics
   * - Slow Query Log
     - Slow Query Log
   * - Count InnoDB Deadlocks
     - Count InnoDB Deadlocks
   * - Log All Client Commands (syslog)
     - Log All Client Commands (syslog)
   * - Response Time Distribution
     - |-implemented|
   * - Show Storage Engines
     - Show Storage Engines
   * - Show Lock Names
     - Show Lock Names
   * - Process List
     - Process List
   * - Misc. INFORMATION_SCHEMA Tables
     - Misc. INFORMATION_SCHEMA Tables
   * - Extended Show Engine InnoDB Status
     - Extended Show Engine InnoDB Status
   * - Thread Based Profiling
     - Thread Based Profiling
   * - XtraDB Performance Improvements for I/O-Bound Highly-Concurrent Workloads
     - XtraDB Performance Improvements for I/O-Bound Highly-Concurrent Workloads
   * - Page cleaner thread tuning
     - Page cleaner thread tuning
   * - Statement Timeout
     - Statement Timeout
   * - Extended SELECT INTO OUTFILE/DUMPFILE
     - Extended SELECT INTO OUTFILE/DUMPFILE
   * - Per-query variable statement
     - Per-query variable statement
   * - Extended mysqlbinlog
     - Extended mysqlbinlog
   * - Slow Query Log Rotation and Expiration
     - Slow Query Log Rotation and Expiration
   * - Metrics for scalability measurement
     - |-implemented|
   * - Audit Log
     - Audit Log
   * - Backup Locks
     - Backup Locks
   * - CSV engine mode for standard-compliant quote and comma parsing
     - CSV engine mode for standard-compliant quote and comma parsing
   * - Super read-only
     - Super read-only


Other Reading
=============

* `What Is New in MySQL 5.7 <http://dev.mysql.com/doc/refman/5.7/en/mysql-nutshell.html>`_

* `What Is New in MySQL 8.0 <http://dev.mysql.com/doc/refman/8.0/en/mysql-nutshell.html>`_

.. |replaced| replace:: Replaced with upstream implementation
.. |-implemented| replace:: Feature not implemented
