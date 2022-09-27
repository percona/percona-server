.. _innodb_fragmentation_count:

====================================
InnoDB Page Fragmentation Counters
====================================

*InnoDB* page fragmentation is caused by random insertion or deletion from a
secondary index. This means that the physical ordering of the index pages on
the disk is not same as the index ordering of the records on the pages. As a
consequence this means that some pages take a lot more space and that queries
which require a full table scan can take a long time to finish.

To provide more information about the *InnoDB* page fragmentation *Percona Server for MySQL* now provides the following counters as status variables: 
:ref:`Innodb_scan_pages_contiguous`,
:ref:`Innodb_scan_pages_disjointed`, :ref:`Innodb_scan_data_size`,
:ref:`Innodb_scan_deleted_recs_size`, and
:ref:`Innodb_scan_pages_total_seek_distance`.


Version Specific Information
============================

* :ref:`8.0.12-1`: The feature was ported from *Percona Server for MySQL* 5.7

Status Variables
================

.. _Innodb_scan_pages_contiguous:

.. rubric:: ``Innodb_scan_pages_contiguous``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Scope
     - Session
   * - Data type
     - Numeric

This variable shows the number of contiguous page reads inside a query.

.. _Innodb_scan_pages_disjointed:

.. rubric:: ``Innodb_scan_pages_disjointed``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Scope
     - Session
   * - Data type
     - Numeric

This variable shows the number of disjointed page reads inside a query.

.. _Innodb_scan_data_size:

.. rubric:: ``Innodb_scan_data_size``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Scope
     - Session
   * - Data type
     - Numeric

This variable shows the size of data in all *InnoDB* pages read inside a
query (in bytes) - calculated as the sum of ``page_get_data_size(page)`` for
every page scanned.

.. _Innodb_scan_deleted_recs_size:

.. rubric:: ``Innodb_scan_deleted_recs_size``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Scope
     - Session
   * - Data type
     - Numeric

This variable shows the size of deleted records (marked as ``deleted`` in
``page_delete_rec_list_end()``) in all *InnoDB* pages read inside a query
(in bytes) - calculated as the sum of ``page_header_get_field(page,
PAGE_GARBAGE)`` for every page scanned.

.. _Innodb_scan_pages_total_seek_distance:

.. rubric:: ``Innodb_scan_pages_total_seek_distance``

.. list-table::
   :header-rows: 1

   * - Option
     - Description
   * - Scope
     - Session
   * - Data type
     - Numeric

This variable shows the total seek distance when moving between pages.

Related Reading
===============

  * `InnoDB: look after fragmentation
    <https://www.percona.com/blog/2009/11/05/innodb-look-after-fragmentation/>`_
 
  * `Defragmenting a Table
    <https://dev.mysql.com/doc/refman/8.0/en/innodb-file-defragmenting.html>`_
