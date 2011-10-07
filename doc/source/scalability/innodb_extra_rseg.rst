.. _innodb_extra_rseg:

============================
 Multiple Rollback Segments
============================

.. default-domain:: psdom

To provide consistent reads, |InnoDB| writes data modified by active transactions in an area called the rollback segment. This single segment is protected by a single mutex, which is a major cause of contention for write-intensive workloads. With this change you can increase the number of rollback segments, which is likely to help you to greatly improve performance. You can see this `post <http://www.mysqlperformanceblog.com/2009/01/18/partial-fix-of-innodb-scalability-rollback-segments/>`_ for a benchmark.

Some write-intensive workloads on boxes with many CPUs have scalability problems. The contention is caused by the rollback segment, which is single: all transactions are serialized when needing to access the segment. With this feature you can now create and use multiple segments (up to 256).

**NOTE**: This feature is incompatible with |InnoDB|. As long as a single rollback segment is used, there is no problem; the database can still be used by both |XtraDB| and |InnoDB|. However, creating multiple rollback segments will cause an internal format change to the system tablespace. Once multiple segments have been created, the database will no longer be compatible with |InnoDB|.


System Variables
================

The following system variable was introduced by this feature:

.. variable:: innodb_extra_rsegments

   :cli: Yes
   :conf: Yes
   :scope: Global
   :dyn: No
   :type: ULONG
   :def: 0
   :range: 0-126

This option specifies the number of extra user rollback segments.

When you modify this variable, you must restart the |MySQL| server for the change to take effect. Please note that you must perform a slow shutdown (ie with ``innodb_fast_shutdown = 0``). If you just perform a fast shutdown, the |MySQL| server will then restart without error but the additional segments will not be created.

To check that the extra segments have been created, you can run the following query: ::

  SELECT COUNT(*) FROM information_schema.INNODB_RSEG;

The result should be the number of extra segments + 1 (as a default single segment always exists).

 This variable has been removed from |Percona Server| 5.5.11-20.2 because an equivalent variable, ``innodb_rollback_segment``, has been implemented in |MySQL| 5.5.


``INFORMATION_SCHEMA`` Tables
=============================

This feature provides the following table:

.. table:: INFORMATION_SCHEMA.INNODB_RSEG

   :column rseg_id: rollback segment id
   :column space_id: space where the segment placed
   :column zip_size: compressed page size in bytes if compressed otherwise 0
   :column page_no: page number of the segment header
   :column max_size: max size in pages
   :column curr_size: current size in pages

This table shows information about all the rollback segments (the default segment and the extra segments).

Here is an example of output with ``innodb_extra_rsegments = 8`` ::

  mysql> select * from information_schema.innodb_rseg;
  +---------+----------+----------+---------+------------+-----------+
  | rseg_id | space_id | zip_size | page_no | max_size   | curr_size |
  +---------+----------+----------+---------+------------+-----------+
  |       0 |        0 |        0 |       6 | 4294967294 |         1 |
  |       1 |        0 |        0 |      13 | 4294967294 |         2 |
  |       2 |        0 |        0 |      14 | 4294967294 |         1 |
  |       3 |        0 |        0 |      15 | 4294967294 |         1 |
  |       4 |        0 |        0 |      16 | 4294967294 |         1 |
  |       5 |        0 |        0 |      17 | 4294967294 |         1 |
  |       6 |        0 |        0 |      18 | 4294967294 |         1 |
  |       7 |        0 |        0 |      19 | 4294967294 |         1 |
  |       8 |        0 |        0 |      20 | 4294967294 |         1 |
  +---------+----------+----------+---------+------------+-----------+
  9 rows in set (0.00 sec)

Other Reading
=============

  * `Fix of InnoDB/XtraDB scalability of rollback segment <http://www.mysqlperformanceblog.com/2009/01/18/partial-fix-of-innodb-scalability-rollback-segments/>`_

  * `Tuning for heavy writing workloads <http://www.mysqlperformanceblog.com/2009/10/14/tuning-for-heavy-writing-workloads/>`_
