.. _innodb_fast_index_creation:

=====================
 Fast Index Creation
=====================

Percona has implemented several changes related to |MySQL|'s fast index creation feature. Extended features, besides disabling :variable:`fast_index_creation`, can be enabled with :variable:`expand_fast_index_creation`. 

Disabling Fast Index Creation
=============================

Fast index creation was implemented in |MySQL| as a way to speed up the process of adding or dropping indexes on tables with many rows. However, cases have been found in which fast index creation creates an inconsistency between |MySQL| and |InnoDB| data dictionaries.

This feature implements a session variable that disables fast index creation. This causes indexes to be created in the way they were created before fast index creation was implemented. While this is slower, it avoids the problem of data dictionary inconsistency between |MySQL| and |InnoDB|.


Tunable buffer size for fast index creation
===========================================

|Percona Server| supports tunable buffer size for fast index creation in |InnoDB|. This value was calculated based on the merge block size (which was hardcoded to 1 MB) and the minimum index record size. By adding the session variable :variable:`innodb_merge_sort_block_size` block size that is used in the merge sort can now be adjusted for better performance.


Version Specific Information
============================

  * 5.5.8-20.0: 
    Variable :variable:`fast_index_creation` implemented.

  * 5.5.11-21.2:
    Expanded the applicability of fast index creation to :command:`mysqldump`, ``ALTER TABLE``, and ``OPTIMIZE TABLE``.

  * 5.5.27-28.0
    Variable :variable:`innodb_merge_sort_block_size` implemented.

System Variables
================

.. variable:: fast_index_creation

     :cli: Yes
     :conf: No
     :scope: Local
     :dyn: Yes
     :vartype: Boolean
     :default: ON
     :range: ON/OFF

.. variable:: innodb_merge_sort_block_size
 
     :cli: Yes
     :conf: Yes
     :scope: Global
     :dyn: Yes
     :vartype: ULONG
     :default: 1048576 (1M)
     :range: 1048576 - 1073741824 (1G)

Other Reading
=============

  * `Thinking about running OPTIMIZE on your InnoDB Table? Stop! <http://www.mysqlperformanceblog.com/2010/12/09/thinking-about-running-optimize-on-your-innodb-table-stop/>`_

  * `Building Indexes by Sorting In Innodb <http://www.mysqlperformanceblog.com/2012/06/19/building-indexes-by-sorting-in-innodb-aka-fast-index-creation/>`_
