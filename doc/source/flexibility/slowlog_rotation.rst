.. _slowlog_rotation:

========================================
 Slow Query Log Rotation and Expiration
========================================

.. note:: 

   This feature is currently **technical preview** quality.

This feature was implemented in *Percona Server for MySQL* 8.0.27-18.

Percona has implemented two new variables, :variable:`max_slowlog_size` and :variable:`max_slowlog_files` to provide users with ability to control the slow query log disk usage. These variables have the same behavior as the `max_binlog_size variable <https://dev.mysql.com/doc/refman/8.0/en/replication-options-binary-log.html#sysvar_max_binlog_size>`__ and the `max_binlog_files variable <https://dev.mysql.com/doc/refman/8.0/en/replication-options-binary-log.html#sysvar_max_binlog_size>`__ used for controlling the binary log.

System Variables
================

.. variable:: max_slowlog_size

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: numeric
     :default: 0 (unlimited)
     :range: 0 - 1073741824

The server rotates the slow query log when the log's size reaches this value. The default value is ``0``. If you limit the size and this feature is enabled, the server renames the slow query log file to `slow_query_log_file`.000001. 

.. variable:: max_slowlog_files

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: numeric
     :default: 0 (unlimited)
     :range: 0 - 102400

This variable limits the total amount of slow query log files and is used with :variable:`max_slowlog_size`. 

The server creates and adds slow query logs until reaching the range's upper value. When the upper value is reached, the server creates a new slow query log file with a higher sequence number and deletes the log file with the lowest sequence number maintaining the total amount defined in the range.