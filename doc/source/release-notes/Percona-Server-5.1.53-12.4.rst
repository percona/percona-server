.. rn:: 5.1.53-12.4

==============================
 |Percona Server| 5.1.53-12.4
==============================

Released on December 29, 2010 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.53-12.4/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_.)

|Percona Server| 5.1.53-12.4 is now the current stable release version.

Functionality Added or Changed
==============================

  * |Percona Server| 5.1.53-12.4 is based on |MySQL| 5.1.53.

  * New Features Added:

    * Precompiled `UDFs for Maatkit <http://code.google.com/p/maatkit/wiki/InstallingUdfs>`_ (``FNV`` and ``MurmurHash`` hash functions to provide faster checksums) are now included in distributions. Fixes feature request :bug:`689992`. (*Aleksandr Kuzminsky*)

  * Other Changes: 

    * ``percona_innodb_doublewrite_path`` - It's no longer necessary to recreate your database and |InnoDB| system files when a dedicated file to contain the doublewrite buffer is specified. (*Yasufumi Kinoshita*)

    * Added system variable :variable:`have_response_time_distribution` and compile option ``--without-response_time_distribution`` in :ref:`response_time_distribution`. (*Oleg Tsarev*)


Bugs Fixed
==========

  * Bug :bug:`643149` - Slow query log entries were not written in the usual parsing format. (*Alexey Kopytov*)

  * Bug :bug:`671764` - ``innochecksum`` wasn't distributed with RPM and .DEB packages. (*Aleksandr Kuzminsky*)

  * Bug :bug:`673426` - Use of these system variables as command-line options could cause a crash or undefined behavior: ``log_slow_timestamp_every``, ``log_slow_sp_statements``, ``slow_query_log_microseconds_timestamp``, ``use_global_long_query_time``. (*Oleg Tsarev*)

  * Bug :bug:`673567` - Compiler could produce spurious warnings when building on non-Linux platforms. A check is now made to see if ``clock_gettime()`` is present in ``librt`` at the configure stage. If yes, `` -lrt`` is added to ``LIBS``. (*Alexey Kopytov*)

  * Bug :bug:`673929` - Query cache misses were being reported for some queries when hits were actually occurring.

  * Bug :bug:`676146` - The development environment test of ``log_slow_verbosity=innodb`` on a slave for row-based replication was not working correctly. (*Oleg Tsarev*)

  * Bug :bug:`676147`, :bug:`676148` - The development environment tests of options ``log_slow_slave_statements`` and ``use_global_long_query_time`` work only on statement-based replication. They were failing when row-based replication was attempted. A check is now made for the replication type to test. (*Oleg Tsarev*)

  * Bug :bug:`676158` - Setting the query cache size to 512M caused test failure on low memory systems (*Aleksandr Kuzminsky*)

  * Bug :bug:`677407` - The ``innodb_information_schema`` test could fail sporadically due to flawed logic in the ``INFORMATION_SCHEMA.INNODB_LOCKS`` caching mechanism. (contributed by *Kristian Nielsen*) (*Alexey Kopytov*)

  * Bug :bug:`681486` - A dependency between Debian packages ``lib|MySQL|client16`` and ``percona-server-common`` was removed. (*Aleksandr Kuzminsky*)

  * Bug :bug:`693815` - The test ``percona_innodb_buffer_pool_shm`` was failing. It should be run with the ``--big-test`` option. As the buffer pool size used in the test is 128M, the shared memory segment should be increased appropriately in order to run the test successfully.

  * Bug :bug:`693814`, :bug:`693815`, :bug:`693816`, :bug:`693817`, :bug:`693819` - Tests in the test environment were updated to reflect past ``INFORMATION_SCHEMA`` changes. (*Aleksandr Kuzminsky*)

  * Bug :bug:`693818` - Warning and error messages for stored routines could incorrectly report row numbers due to a change in the ``slow_extended`` patch. (*Alexey Kopytov*)
