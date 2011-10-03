.. rn:: 1.0.6-10

=========================
|Percona Server| 1.0.6-10
=========================

Released on April 13, 2010

    * |MySQL| 5.1.45 as a basis for the release

    * ``I_S SYS_TABLES``

    * New patch ``page_size_and_expand_log``. It allows to change |InnoDB| pages size to 4K or 8K. Also |InnoDB| log files now can be bigger 4G

    * New patch innodb-fast-checksum. It adds new faster checksum algorithm used in |InnoDB| tables

    * New supported platforts are added. The full list includes (karmic is new):

      * CentOS 5 (x86_64 and i386)

      * CenOS 4 (x86_64 and i386)

      * Debian lenny (x86_64 and i386)

      * Debian etch (x86_64 and i386)

      * Ubuntu Karmic  (x86_64 and i386)

      * Ubuntu Jaunty (x86_64 and i386)

      * Ubuntu Intrepid (x86_64 and i386)

      * Ubuntu Hardy (x86_64 and i386)

      * FreeBSD 8 (x86_64 and i386)

      * OpenSolaris (x86_64)

Fixed bugs in the release
=========================

    * Bug :bug:`551379`:Change SE Name at |MySQL| start

    * Bug :bug:`550867`: 5.1.45+|XtraDB|-10 compilation fails on Ubuntu Karmic

    * Bug :bug:`547230`: Add new columns to ``innodb_table_stats``

    * Bug :bug:`548442`: ``INFORMATION_SCHEMA.TABLE_STATISTICS`` is broken in |XtraDB| 9.1

    * Bug :bug:`509017`: ``srv_table_reserve_slot()`` should be protected by ``kernel_mutex``

    * Bug :bug:`520825`: ``ANALYZE TABLE`` doesn't anything when ``innodb_stats_on_metadata=OFF``
