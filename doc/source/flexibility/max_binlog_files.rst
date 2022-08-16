.. _maximum_binlog_files:

========================================
 Restricting the number of binlog files
========================================

Maximum number of binlog files can now be restricted in *Percona Server for MySQL* with
:ref:`max_binlog_files`. When variable :ref:`max_binlog_files` is set
to non-zero value, the server will remove the oldest binlog file(s) whenever
their number exceeds the value of the variable.

This variable can be used with the existing :ref:`max_binlog_size` variable
to limit the disk usage of the binlog files. If :ref:`max_binlog_size` is
set to 1G and :ref:`max_binlog_files` to 20 this will limit the maximum
size of the binlogs on disk to 20G. The actual size limit is not necessarily
:ref:`max_binlog_size` * :ref:`max_binlog_files`. Server restart or
``FLUSH LOGS`` will make the server start a new log file and thus resulting in
log files that are not fully written in these cases limit will be lower.

Example
=======

Number of the binlog files before setting this variable :: 

  $ ls -l mysql-bin.0* | wc -l
  26

Variable :ref:`max_binlog_files` is set to 20: ::

  max_binlog_files = 20

In order for new value to take effect ``FLUSH LOGS`` needs to be run. After that the number of binlog files is 20 :: 

  $ ls -l mysql-bin.0* | wc -l
  20

Version Specific Information
============================

  * :ref:`5.7.10-1`: Variable :ref:`max_binlog_files` ported from *Percona Server for MySQL* 5.6.
  * :ref:`5.7.23-23`: Variable :ref:`max_binlog_files` is deprecated and replaced with :ref:`binlog_space_limit`.

System Variables
================

.. _max_binlog_files:

.. rubric:: ``max_binlog_files``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global
   * - Dynamic
     - Yes
   * - Data type
     - ULONG
   * - Default
     - 0 (unlimited)
   * - Range
     - 0-102400

.. _binlog_space_limit:

.. rubric:: ``binlog_space_limit``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global
   * - Dynamic
     - No
   * - Data type
     - ULONG
   * - Default
     - 0 (unlimited)
   * - Range
     - 0-102400  
     
This option places an upper limit on the total size in bytes of all binary logs. A value of ``0`` means
“no limit”. This is useful for a server host that has limited disk space.

When the limit is reached, oldest binary logs are purged until the total size is under the limit or only
active log is remaining.

.. note:: You should not set ``--binlog-space-limit`` to less or equal than the value of
          ``--max-binlog-size`` because after the max-binlog-size limit will be reached, logs will be
          rotated and immediately pruned by binlog-space-limit.
