.. _innodb_extra_rseg:

============================
 Multiple Rollback Segments
============================

In Percona Server 5.1, an improvement was provided for write-intensive
workloads that allowed multiple rollback segments to be used. It has
been removed from |Percona Server| 5.5.11-20.2 because an equivalent
variable, ``innodb_rollback_segment``, has been implemented in |MySQL|
5.5, and so the MySQL 5.5 implementation is available in Percona
Server 5.5.

Percona Server, in addition to the upstream multiple rollback segment implementation, provides the additonal Information Schema table: INFORMATION_SCHEMA.INNODB_RSEG.

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

