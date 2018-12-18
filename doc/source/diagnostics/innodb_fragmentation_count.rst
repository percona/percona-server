.. _innodb_fragmentation_count:

====================================
|InnoDB| Page Fragmentation Counters
====================================

|InnoDB| page fragmentation is caused by random insertion or deletion from a
secondary index. This means that the physical ordering of the index pages on
the disk is not same as the index ordering of the records on the pages. As a
consequence this means that some pages take a lot more space and that queries
which require a full table scan can take a long time to finish.

To provide more information about the |InnoDB| page fragmentation |Percona
Server| now provides the following counters as status variables: 
:variable:`Innodb_scan_pages_contiguous`,
:variable:`Innodb_scan_pages_disjointed`, :variable:`Innodb_scan_data_size`,
:variable:`Innodb_scan_deleted_recs_size`, and
:variable:`Innodb_scan_pages_total_seek_distance`.


Version Specific Information
============================

* :rn:`8.0.12-1`: The feature was ported from |Percona Server| 5.7

Status Variables
================

.. variable:: Innodb_scan_pages_contiguous

     :vartype: Numeric
     :scope: Session

This variable shows the number of contiguous page reads inside a query.

.. variable:: Innodb_scan_pages_disjointed

     :vartype: Numeric
     :scope: Session

This variable shows the number of disjointed page reads inside a query.

.. variable:: Innodb_scan_data_size

     :vartype: Numeric
     :scope: Session

This variable shows the size of data in all |InnoDB| pages read inside a
query (in bytes) - calculated as the sum of ``page_get_data_size(page)`` for
every page scanned.

.. variable:: Innodb_scan_deleted_recs_size

     :vartype: Numeric
     :scope: Session

This variable shows the size of deleted records (marked as ``deleted`` in
``page_delete_rec_list_end()``) in all |InnoDB| pages read inside a query
(in bytes) - calculated as the sum of ``page_header_get_field(page,
PAGE_GARBAGE)`` for every page scanned.

.. variable:: Innodb_scan_pages_total_seek_distance

     :vartype: Numeric
     :scope: Session

This variable shows the total seek distance when moving between pages.

Related Reading
===============

  * `InnoDB: look after fragmentation
    <https://www.percona.com/blog/2009/11/05/innodb-look-after-fragmentation/>`_
 
  * `Defragmenting a Table
    <https://dev.mysql.com/doc/refman/8.0/en/innodb-file-defragmenting.html>`_
