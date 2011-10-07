.. rn:: 5.1.53-11.7

==============================
 |Percona Server| 5.1.53-11.7
==============================

Released on December 16, 2010 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.53-11.7/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Functionality Added or Changed
==============================

  * |Percona Server| 5.1.53-11.7 is based on |MySQL| 5.1.53.

  * New Features Added: None

  * Other Changes: None

Bugs Fixed
==========

  * Bug :bug:`643149` - Slow query log entries were not being done in the usual parsing format. (*Alexey Kopytov*)

  * Bug :bug:`677407 - The ``INNODB.innodb_information_schema`` test could fail sporadically due to flawed logic in the ``INFORMATION_SCHEMA.INNODB_LOCKS`` caching mechanism. (*Alexey Kopytov*)
