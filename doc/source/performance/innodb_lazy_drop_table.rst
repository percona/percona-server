.. _innodb_lazy_drop_table_page:

======================
Drop table performance
======================

When  *innodb_file_per_table* is set to 1, doing a DROP TABLE can take a long time on servers with a large buffer pool, even on an empty |InnoDB| table. This is because InnoDB has to scan through the buffer pool to purge pages that belong to the corresponding tablespace. Furthermore, no other queries can start while that scan is in progress.

This feature allows you to do "background table drop".

Version Specific Information
============================

  * 5.5.10-20.1 Feature added.

System Variables
================

.. variable:: innodb_lazy_drop_table

   :cli: Yes
   :conf: Yes
   :scope: Global       
   :dyn: Yes   
   :vartype: BOOL
   :default: FALSE
   :range: TRUE/FALSE

When this option is ON, XtraDB optimizes that process by only marking the pages corresponding to the tablespace being deleted. It defers the actual work of evicting those pages until it needs to find some free pages in the buffer pool.

When this option is OFF, the usual behavior for dropping tables is in effect.

Related Reading
===============

   * Drop table performance `blog post <http://www.mysqlperformanceblog.com/2011/04/20/drop-table-performance/>`_.
