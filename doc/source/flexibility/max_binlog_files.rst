.. _maximum_binlog_files:

========================================
 Restricting the number of binlog files
========================================

Maximum number of binlog files can now be restricted in |Percona Server| with :variable:`max_binlog_files`. This means maximum disk usage of binlogs can be set rather than time. 
This option can be combined with already existing :variable:`expire_logs_days` as they are both taken into the account when rotating the binlog. For example if :variable:`expire_logs_days` is set to 10 days and :variable:`max_binlog_files` to 1GB, if binlog reaches 1GB it will be rotated, or if it hasn't been rotated for 10 days it will be rotated.


Version Specific Information
============================

  * 5.5.27-29.0:
    Variable :variable:`max_binlog_files` introduced.

System Variables
================

.. variable:: max_binlog_files

     :version 5.5.27-29.0: Introduced.
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: ULONG
     :default: 0
     :range: 0-102400

