.. _slowlog_rotation:

========================================
 Slow Query Log Rotation and Expiration
========================================

.. note:: 

   This feature is currently considered BETA quality.

Percona has implemented two new variables, :variable:`max_slowlog_size` and :variable:`max_slowlog_files` to provide users with ability to control the slow query log disk usage. These variables have the same behavior as upstream variable `max_binlog_size <https://dev.mysql.com/doc/refman/5.6/en/replication-options-binary-log.html#sysvar_max_binlog_size>`_ and :variable:`max_binlog_files` variable used for controlling the binary log.

.. warning::

   For this feature to work variable :variable:`slow_query_log_file` needs to be set up manually and without the ``.log`` sufix. The slow query log files will be named using :variable:`slow_query_log_file` as a stem, to which a dot and a sequence number will be appended.

Version Specific Information
============================

  * :rn:`5.7.10-1`:
    Feature ported from |Percona Server| 5.6

System Variables
================

.. variable:: max_slowlog_size

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: numeric
     :default: 0 (unlimited)
     :range: 4096 - 1073741824

Slow query log will be rotated automatically when its size exceeds this value. The default is ``0``, don't limit the size. When this feature is enabled slow query log file will be renamed to :variable:`slow_query_log_file`.000001. 

.. variable:: max_slowlog_files

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: numeric
     :default: 0 (unlimited)
     :range: 0 - 102400

Maximum number of slow query log files. Used with :variable:`max_slowlog_size` this can be used to limit the total amount of slow query log files. When this number is reached server will create a new slow query log file with increased sequence number. Log file with the lowest sequence number will be deleted.
