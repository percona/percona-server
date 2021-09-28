.. _innodb_corrupt_table_action_page:

=========================
 Handle Corrupted Tables
=========================

When a server subsystem tries to access a corrupted table,
the server may crash.
If this outcome is not desirable when a corrupted table is encountered,
set the new system :variable:`innodb_corrupt_table_action` variable
to a value which allows the ongoing operation to continue
without crashing the server.

The server error log registers attempts to access corrupted table pages.

.. rubric:: Interacting with the :variable:`innodb_force_recovery` variable

The :variable:`innodb_corrupt_table_action` variable
may work in conjunction with the :variable:`innodb_force_recovery` variable
which considerably reduces
the effect of InnoDB subsystems
running in the background.

If the :variable:`innodb_force_recovery` variable is set to a low value
and you expect the server to crash,
the server may continue to run due to a non-default value of the :variable:`innodb_corrupt_table_action` variable.

For more information about the :variable:`innodb_force_recovery` variable,
see `Forcing InnoDB Recovery
<https://dev.mysql.com/doc/refman/5.5/en/forcing-innodb-recovery.html>`_
from the MySQL Reference Manual.

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
     :default: ``assert``

* With the default value, ``assert``, XtraDB will intentionally crash the server with an assertion failure as it would normally do when detecting corrupted data in a single-table tablespace.

* If the ``warn`` value is used it will pass corruption of the table as ``corrupt table`` instead of crashing itself. For this to work :option:`innodb_file_per_table` should be enabled. All file I/O for the datafile after detected as corrupt is disabled, except for the deletion. 

* When the option value is ``salvage``, |XtraDB| allows read access to a corrupted tablespace, but ignores 
corrupted pages". You must enable the `innodb_file_per_table` option. 

