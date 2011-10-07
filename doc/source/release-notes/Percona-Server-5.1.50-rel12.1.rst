.. rn:: 5.1.50-rel12.1

=================================
 |Percona Server| 5.1.50-rel12.1
=================================


Functionality Added or Changed
==============================

  * |Percona Server| 5.1.50-rel12.1 RC is now based on |MySQL| 5.1.50.

  * New Features Added:

    * :ref:`innodb_lru_dump_restore` - Implemented automatic dumping of the buffer pool at specified intervals.

    * :ref:`innodb_buffer_pool_shm` - Implemented option :variable:`innodb_buffer_pool_shm_checksum`; when enabled, shared memory buffer pool is checksum validated. Bugs fixed: :bug:`643724`, :bug:`649408`, and :bug:`649393`. (*Yasufumi Kinoshita*)

    * :ref:`innodb_expand_import_page` - Implemented more exact checking to avoid corruption of the ``.ibd`` file using :variable:`innodb_expand_export`. (*Yasufumi Kinoshita*)

    * :ref:`mysql_remove_eol_carret` - Implemented a |MySQL| client option to handle end-of-line in BLOB fields differently. Bug fixed: :bug:`625066`. (*Sasha Pachev*)

    * :ref:`response_time_distribution` - Counts queries with very short execution times and groups them by time interval. (*Oleg Tsarev*)

Bugs Fixed
==========

  * Bug :bug:`608992` - Added symbolic info to |Percona Server| binary to help in debugging. (*Aleksandr Kuzminsky*)

  * Bug :bug:`624362` - Corrected wording in an |InnoDB| error message regarding too many locks printed. (*Vadim Tkachenko*) 

  * Bug :bug:`625066` - Fixed a problem handling end-of-line in ``BLOB`` fields in the |MySQL| client. (*Sasha Pachev*)

  * Bug :bug:`640924` - Fixed a crash caused by ``innodb_doublewrite_file``. (*Yasufumi Kinoshita*)

  * Bug :bug:`643650` - Speeded up |InnoDB| shutdown when using shared memory buffer pool. (*Yasufumi Kinoshita*)

  * Bug :bug:`643724` - Fixed an |InnoDB| crash when shared memory buffer pool was enabled. (*Yasufumi Kinoshita*)

  * Bug :bug:`649408` - Fixed a problem causing a crash on startup when using shared memory buffer pool. (*Yasufumi Kinoshita*)

  * Bug :bug:`649393` - |InnoDB| now recreates the shared memory segment for the buffer pool automatically after a crash. (*Yasufumi Kinoshita*)

  * Bug :bug:`649623` - Fixed an error when compiling |Percona Server| 5.1.50-rel12.1 on FreeBSD (*Oleg Tsarev*)

  * Bug :bug:`650977` - Fixed failed tests. (*Oleg Tsarev*)
