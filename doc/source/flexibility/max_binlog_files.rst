.. _maximum_binlog_files:

========================================
 Restricting the number of binlog files
========================================

Maximum number of binlog files can now be restricted in |Percona Server| with :variable:`max_binlog_files`. When variable :variable:`max_binlog_files` is set to non-zero value, sever will remove the oldest binlog file(s) whenever their number exceeds the value of the variable. This variable can be used with the existing :variable:`max-binlog-size` variable to limit the disk usage of the binlog files.

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
     :default: 0 (unlimited)
     :range: 0-102400

Example
=======
Number of the binlog files before setting this variable :: 

  $ ls -l mysql-bin.0* | wc -l
  26

Variable :variable:`max_binlog_files` is set to 20: ::

  max_binlog_files = 20

After server restart the number of binlog files is now 20 :: 

  $ls -l mysql-bin.0* | wc -l
  20

