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


Return error ``1194 (ER_CRASHED_ON_USAGE)`` instead of crashing with an assertion failure, when used with :variable:`innodb_file_per_table`. Once corruption is detected, access to the corrupted tablespace is disabled. The only allowed operation on a corrupted tablespace is ``DROP TABLE``. The only exception to this rule is when the option value is ``salvage`` (see below).
This variable was renamed to :variable:`innodb_corrupt_table_action`, beginning in release :rn:`5.5.10-20.1`. The option name was :variable:`innodb_pass_corrupt_table` in versions prior to that.

.. variable:: innodb_corrupt_table_action

     :version 5.5.10-20.1: Introduced.
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: ENUM
     :default: ``assert``
     :range: ``assert``, ``warn``, ``salvage``

* With the default value |XtraDB| will intentionally crash the server with an assertion failure as it would normally do when detecting corrupted data in a single-table tablespace.

* If the ``warn`` value is used it will pass corruption of the table as ``corrupt table`` instead of crashing itself. For this to work :option:`innodb_file_per_table` should be enabled. All file I/O for the datafile after detected as corrupt is disabled, except for the deletion. 

* When the option value is ``salvage``, |XtraDB| allows read access to a corrupted tablespace, but ignores corrupted pages".

This variable was added in release :rn:`5.5.10-20.1`. Prior to that, it was named :variable:`innodb_pass_corrupt_table`, which still exists in earlier versions.
