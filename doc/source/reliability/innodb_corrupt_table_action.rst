.. _innodb_corrupt_table_action_page:

=========================
 Handle Corrupted Tables
=========================

Instead of crashing the server as they used to do, corrupted |InnoDB| tables are simply disabled, so that the database remains available while the corruption is being fixed.

This feature adds a new system variable.

Version Specific Information
============================

  * 5.6.10-60.2:
    Feature ported from |Percona Server| 5.5

System Variables
================

.. variable:: innodb_corrupt_table_action

     :version 5.6.10-60.2: Introduced.
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: ULONG
     :default: ``assert``
     :range: ``assert``, ``warn``

When the default value is used InnoDB will stop the server if it finds a checksum mismatch on the tables. If warn values is used it will pass corruption of the table as corrupt table instead of crashing itself. For this to work variable :variable:`innodb_file_per_table` should be enabled. All file I/O for the datafile after detected as corrupt is disabled, except for the deletion.
