.. _maximum_binlog_size:

========================================
 Restricting the number of binlog files
========================================

The task of estimating the number and size of binary logs and the disk space use
is a challenge. |Percona Server| has the variable
:variable:`binlog_space_limit`, which you can use to
define the maximum amount of available space for all binary logs. A limit of
`0`, the default value, disables the variable, and the binary log files have no limit. 

.. note::

    The value specified is not a hard limit. A single
    transaction is written to a single binary log, and the size 
    may exceed the limit. To store a large transaction, the :variable:`binlog_space_limit` removes old binary logs as needed, up to the three oldest ones, to remain under the limit.
    
    The last binary log is never pruned.

Example
=======

The following code is an example of configuring the size of a binary log file
and the space allowed for all binary file logs in my.cnf:

.. code-block:: guess

[mysqld]
max_binlog_size = 1G
binlog_space_limit = 10G

System Variables
================

.. variable:: max_binlog_size

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: INTEGER
     :default: 1GB
     :range: 4096 - 1GB

This option specifies the maximum size of the current log file. 

.. seealso::

    MySQL Documentation
    `maximum_binlog_size <https://dev.mysql.com/doc/refman/8.0/en/replication-options-binary-log.html#sysvar_max_binlog_size>`__ 

.. variable:: binlog_space_limit

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: ULONG
     :default: 0 (unlimited)
     :range: 0-102400

This option places an upper limit on the total size in bytes of all binary logs.

.. note:: You should not set ``--binlog-space-limit`` to less or equal than the value of
          ``--max-binlog-size`` because if the max-binlog-size limit is reached, the logs are
          rotated and immediately pruned by binlog-space-limit.
