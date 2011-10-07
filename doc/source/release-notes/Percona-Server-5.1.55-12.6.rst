.. rn:: 5.1.55-12.6

==============================
 |Percona Server| 5.1.55-12.6
==============================

Released on March 7, 2011 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.55-12.6/>`_ and `installation instructions <http://www.percona.com/docs/wiki/percona-server:start#installation_instructions>`_.)

|Percona Server| 5.1.55-12.6 is now the current stable release version. It is is based on |MySQL| 5.1.55.

Changes
=======

  * Fixed compiler warnings in both the core server and in |XtraDB|. (*Alexey Kopytov*, *Yasufumi Kinoshita*)

=Bugs Fixed
===========

  * Bug :bug:`602047` - The ``ROWS_READ`` columns of ``TABLE_STATISTICS`` and ``INDEX_STATISTICS`` were not properly updated when a query involved index lookups on an |InnoDB| table. (*Yasufumi Kinoshita*)

  * Bug :bug:`707742` - The server could crash when trying to import a table which had not been previously prepared using :command:`xtrabackup --prepare --export`. Also, on servers with huge buffer pools, adding or removing an index even on an empty |InnoDB| table could take a long time due to excessive locking when :variable:`innodb_dict_size_limit` was non-zero. Locking was relaxed to alleviate this. (*Yasufumi Kinoshita*)

  * Bug :bug:`724674 - Ported an updated version of the original implementation of the :ref:`remove_fcntl_excessive_calls` feature, which removes some ``fcntl`` calls to improve performance. (*Oleg Tsarev*)
