.. _innodb_corrupt_table_action_page:

=========================
 Handle Corrupted Tables
=========================

Instead of crashing the server as they used to do, corrupted |InnoDB| tables are simply disabled, so that the database remains available while the corruption is being fixed.

This feature adds a new system variable.


System Variables
================

.. variable:: innodb_pass_corrupt_table

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: ULONG
     :default: 0
     :range: 0 - 1


Pass corruptions of user tables as ``corrupt table`` instead of crashing itself, when used with innodb_file_per_table. All file I/O for the datafile after detected as corrupt is disabled, except for the deletion.

