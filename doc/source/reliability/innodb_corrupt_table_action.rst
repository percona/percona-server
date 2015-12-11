.. _innodb_corrupt_table_action_page:

=========================
 Handle Corrupted Tables
=========================

Instead of crashing the server as they used to do, corrupted |InnoDB| tables are simply disabled, so that the database remains available while the corruption is being fixed.

This feature adds a new system variable.

Version Specific Information
============================

  * :rn:`5.7.10-1`:
    Feature ported from |Percona Server| 5.6

System Variables
================

.. variable:: innodb_corrupt_table_action

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: ULONG
     :range: ``assert``, ``warn``, ``salvage``

* With the default value |XtraDB| will intentionally crash the server with an assertion failure as it would normally do when detecting corrupted data in a single-table tablespace.

* If the ``warn`` value is used it will pass corruption of the table as ``corrupt table`` instead of crashing itself. For this to work :option:`innodb_file_per_table` should be enabled. All file I/O for the datafile after detected as corrupt is disabled, except for the deletion. 

* When the option value is ``salvage``, |XtraDB| allows read access to a corrupted tablespace, but ignores corrupted pages".

