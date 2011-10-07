.. _innodb_files_extend:

================================
 Support of Multiple Page Sizes
================================

In standard |InnoDB|, the size of the read-ahead area is computed dynamically, but it always has the same value. This change makes the size fixed, avoiding unuseful computations.

This feature adds a new system variable for setting it.


System Variables
================

.. variable:: innodb_page_size

     :cli: Yes
     :conf: Yes
     :scope:
     :dyn: No
     :vartype: ULONG
     :default: 1 « 14
     :range: 1 « 12 to 1 « ``UNIV_PAGE_SIZE_SHIFT_MAX``

**EXPERIMENTAL**: The universal page size of the database. Changing for an existing database is not supported. Use at your own risk!
