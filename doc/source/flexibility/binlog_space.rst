.. _binlog_space:

============================================================
Limiting the disk space used by binary log files
============================================================

.. contents::
   :local:

It is a challenge to control how much disk space is used by the binary logs. The size of a binary log can vary because a single transaction must be written to a single binary log and cannot be split between multiple binary log files.

.. _binlog_space_limit:

binlog_space_limit(=x) definition
-------------------------------------------

.. list-table::
   :header-rows: 1

   * - Attribute
     - Description
   * - Uses the command line 
     - Yes
   * - Uses the configuration file
     - Yes
   * - Scope
     - Global
   * - Dynamic
     - No
   * - Variable type
     - ULONG_MAX
   * - Default value
     - 0 (unlimited)
   * - Maximum value - 64-bit platform
     - 18446744073709547520


This variable places an upper limit on the total size in bytes of all binary logs. When the limit is reached, the oldest binary logs are purged until the total size is under the limit or only the active log remains.

The default value of ``0`` disables the feature. No limit is set on the log space. The binary logs accumulate indefinitely until the disk space is full. 

Example
-----------

Set the `binlog_space_limit` to 30000 in the ``my.cnf`` file: ::

    [mysqld]
    bin = 15G

.. seealso:: For more information, see the `Percona Blog - Percona Server for MySQL Highlights - binlog_space_limit <https://www.percona.com/blog/2019/07/03/percona-server-for-mysql-highlights-binlog_space_limit/>`__.
     

