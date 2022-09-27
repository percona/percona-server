.. _innodb_corrupt_table_action_page:

=========================
 Handle Corrupted Tables
=========================

When a server subsystem tries to access a corrupted table,
the server may crash.
If this outcome is not desirable when a corrupted table is encountered,
set the new system :ref:`innodb_corrupt_table_action` variable
to a value which allows the ongoing operation to continue
without crashing the server.

The server error log registers attempts to access corrupted table pages.

.. rubric:: Interacting with the :ref:`innodb_force_recovery` variable

The :ref:`innodb_corrupt_table_action` variable
may work in conjunction with the :ref:`innodb_force_recovery` variable
which considerably reduces
the effect of *InnoDB* subsystems
running in the background.

If the :ref:`innodb_force_recovery` option is <4, corrupted pages are lost and the server may continue to run due to the :ref:`innodb_corrupt_table_action` variable having a non-default value.

For more information about the :ref:`innodb_force_recovery` variable,
see `Forcing InnoDB Recovery
<https://dev.mysql.com/doc/refman/5.5/en/forcing-innodb-recovery.html>`_
from the MySQL Reference Manual.

This feature adds a new system variable.

Version Specific Information
============================

  * :ref:`8.0.12-1`: The feature was ported from *Percona Server for MySQL* 5.7.

System Variables
================

.. _innodb_corrupt_table_action:

.. rubric:: ``innodb_corrupt_table_action``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Command-line
     - Yes
   * - Config file
     - Yes
   * - Scope
     - Global
   * - Dynamic
     - Yes
   * - Data type
     - ULONG
   * - Default
     - ``assert``
   * - Range
     - ``assert``, ``warn``, ``salvage``

* With the default value, ``assert``, *XtraDB* will intentionally crash the server with an assertion failure as it would normally do when detecting corrupted data in a single-table tablespace.

* If the ``warn`` value is used it will pass corruption of the table as ``corrupt table`` instead of crashing itself. For this to work :ref:`innodb_file_per_table` should be enabled. All file I/O for the datafile after detected as corrupt is disabled, except for the deletion. 

* When the option value is ``salvage``, *XtraDB* allows read access to a corrupted tablespace, but ignores corrupted pages". You must enable :ref:`innodb_file_per_table`. 


