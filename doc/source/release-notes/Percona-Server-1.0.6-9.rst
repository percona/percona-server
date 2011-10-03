.. rn:: 1.0.6-9

========================
|Percona Server| 1.0.6-9
========================

The release includes following new features:

    * The release is base on 1.0.6 version of |InnoDB| plugin.

    * |MySQL| 5.1.42 as a base release

    * Separate purge thread and LRU dump is implemented (this feature was actually added in Release 8, but somehow it was forgotten)

    * New patch ``innodb_relax_table_creation``

    * Added extended statistics to slow log

    * Adjust defaults with performance intention

    * Added parameter to control checkpoint age

    * Added recovery statistics output when crash recovery (disabled by default)

    * Adjusted intraction of patches

    * Patch to dump and restore ``innodb_buffer_pool``

Fixed bugs:

    * Bug :bug:`488315`: rename columns and add unique index cause serious inconsistent with :command:`mysqld`

    * Bug :bug:495342]]: “MySQL 5.1.41 + InnoDB 1.0.6 + XtraDB patches extensions-1.0.6, rev. 127 fails to compile on OpenSolaris”

    * |MySQL| bug `#47621 <http://bugs.mysql.com/47621>`_: change the fix of http://bugs.mysql.com/47621 more reasonable
