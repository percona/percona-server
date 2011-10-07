.. _innodb_corrupt_table_action_page:

=========================
 Handle Corrupted Tables
=========================

Instead of crashing the server as they used to do, corrupted |InnoDB| tables are simply disabled, so that the database remains available while the corruption is being fixed.

This feature adds a new system variable.

Version Specific Information
============================

  * 5.5.10-20.1:
    Renamed variable :variable:`innodb_pass_corrupt_table` to :variable:`innodb_corrupt_table_action`.

System Variables
================

.. variable:: innodb_pass_corrupt_table

     :version 5.5.10-20.1: Renamed.
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: ULONG
     :default: 0
     :range: 0 - 1


Pass corruptions of user tables as ``corrupt table`` instead of crashing itself, when used with innodb_file_per_table. All file I/O for the datafile after detected as corrupt is disabled, except for the deletion.

 This variable was renamed to innodb_corrupt_table_action, beginning in release 5.5.10-20.1. It still exists as :variable:`innodb_pass_corrupt_table` in versions prior to that.


.. variable:: innodb_corrupt_table_action

     :version 5.5.10-20.1: Introduced.
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: ULONG
     :default: ``assert``
     :range: ``assert``, ``warn``

Pass corruptions of user tables as ``corrupt table`` instead of not crashing itself, when used with file_per_table. All file I/O for the datafile after detected as corrupt is disabled, except for the deletion.

 This variable was added in release 5.5.10-20.1. Prior to that, it was named :variable:`innodb_pass_corrupt_table`, which still exists in earlier versions.
