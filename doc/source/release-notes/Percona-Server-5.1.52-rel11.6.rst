.. rn:: 5.1.52-rel11.6

===============================
|Percona Server| 5.1.52-rel11.6
===============================

Released on November 23, 2010 (`downloads <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.52-11.6/>`_)

Functionality Added or Changed
===============================

  * |Percona Server| 5.1.52-rel11.6 is based on |MySQL| 5.1.52.

  * New Features Added: None

  * Other Changes: None

Bugs Fixed
===========

  * Bug :bug:`671764` - ``innochecksum`` wasn``t distributed with RPM and .DEB packages (Aleksandr Kuzminsky)

  * Bug :bug:`673426` - Use of some system variables as command-line options caused a crash or undefined behavior. (*Oleg Tsarev*)

  * Bug :bug:`673929` - Query cache misses were being reported for some queries when hits were actually occurring.

  * Bug :bug:`676146` - The development environment test of ``log_slow_verbosity=innodb`` on a slave for row-based replication was not working correctly. (*Oleg Tsarev*)

  * Bug :bug:`676147` - The development environment test of option log_slow_slave_statements for row-based replication was not working correctly. (*Oleg Tsarev*)

  * Bug :bug:`676148` - Similar to :bug:`676147`. A check is now made for the replication type to test. (*Oleg Tsarev*)  

  * Bug :bug:`676158` - Setting the query cache size to 512M caused test failure on low memory systems (Aleksandr Kuzminsky)
