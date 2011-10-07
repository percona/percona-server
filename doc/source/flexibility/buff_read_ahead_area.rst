.. _buff_read_ahead_area:

====================================
 Fixed Size for the Read Ahead Area
====================================

|InnoDB| dynamically calculates the size of the read-ahead area in case it has to trigger its read-ahead algorithm. When the workload involves heavy I/O operations, this size is computed so frequently that it has a non-negligeable impact on the CPU usage.

This variable only depends on the size of the buffer pool set by the :variable:`innodb_buffer_pool_size` variable, and as soon as the buffer pool has a size properly greater than 1024 pages (or 16 MB), it is always 64. With this change, its value is fixed to 64, thus removing a bottleneck experienced by some users.

Please note that the minimum allowed value for the |InnoDB| buffer pool is de facto set to 32 MB.

This change is a port of the feature from Facebook:

  *  http://bazaar.launchpad.net/~mysqlatfacebook/mysqlatfacebook/5.1/revision/3538


Version Specific Information
============================

  * :rn:`5.1.47-12.0` :
    Full functionality available.

Other Information
=================

  * Author/Origin:
    Facebook

  * Bugs fixed:
    :bug:`606811`

Other Reading
=============

  * `BUF_READ_AHEAD_AREA Bottleneck <http://www.facebook.com/notes/|MySQL|facebook/using-pmp-to-double-|MySQL|-throughput-part-2/405092575932>`_
