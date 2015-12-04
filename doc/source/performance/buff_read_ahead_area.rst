.. _buff_read_ahead_area:

====================================
 Fixed Size for the Read Ahead Area
====================================

|InnoDB| dynamically calculates the size of the read-ahead area in case it has to trigger its read-ahead algorithm. When the workload involves heavy I/O operations, this size is computed so frequently that it has a non-negligible impact on the CPU usage. With this fix, buffer read-ahead area size is precalculated once per buffer pool instance initialization, and this value is used each time read-ahead is invoked. This implementation should remove a bottleneck experienced by some users.

Version Specific Information
============================

  * :rn:`5.6.13-60.5` :
    |Percona Server| 5.5 feature re-implemented

Other Information
=================

  * Bugs fixed:
    :bug:`606811`

Other Reading
=============

  * `BUF_READ_AHEAD_AREA Bottleneck <http://www.facebook.com/notes/mysqlfacebook/using-pmp-to-double-mysql-throughput-part-2/405092575932>`_
