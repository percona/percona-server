.. _innodb_files_extend:

================================
 Support of Multiple Page Sizes
================================

.. warning:: This feature has been deprecated in the |Percona Server| :rn:`5.1.68-14.6`. It has been replaced by the `upstream <http://dev.mysql.com/doc/refman/5.6/en/innodb-parameters.html#sysvar_innodb_page_size>`_ version released in |MySQL| 5.6.4.

|Percona Server| has implemented support for multiple |InnoDB| page sizes. This can be used to increase the IO performance by setting this value close to storage device block size. |InnoDB| page size can be set up with the :variable:`innodb_page_size` variable.

System Variables
================

.. variable:: innodb_page_size

     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: No
     :vartype: ULONG
     :default: 16384
     :range: 4096, 8192, 16384

**EXPERIMENTAL**: The universal page size of the database. Changing for an existing database is not supported. Use at your own risk!
