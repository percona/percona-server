.. rn:: 5.1.54-12.5

============================
|Percona Server| 5.1.54-12.5
============================

Released on January 11, 2011 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.54-12.5/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_.)

|Percona Server| 5.1.54-12.5 is now the current stable release version.

=Functionality Added or Changed
===============================

  * |Percona Server| 5.1.54-12.5 is based on |MySQL| 5.1.54.

  * New Features Added:

    * Added system variable :variable:`innodb_log_block_size` and new value "keep_average" to system variable, and :variable:`innodb_adaptive_checkpoint` in :ref:`innodb_io_page`. (*Yasufumi Kinoshita*)

    * Added new value "ALL_O_DIRECT" to system variable :variable:`innodb_flush_method` in :ref:`innodb_io_page`. (*Yasufumi Kinoshita*)

  * Other Changes: None

Bugs Fixed
===========

  * Bug :bug:`689830` - The development environment tests of ``show_slave_status_nolock`` work only on statement-based replication. They were failing when row-based replication was attempted. A check is now made for the replication type to test.  (*Oleg Tsarev*)

  * Bugs :bug:`688643`, :bug:`691234` - Boolean command line options and configuration variables in the ``slow_extended`` patch were not being processed properly. (*Oleg Tsarev*)

  * Bug :bug:`692211` - Starting the server with a non-zero ``innodb_auto_lru_dump`` value could crash the server if the dump file did not exist. (*Alexey Kopytov*)
