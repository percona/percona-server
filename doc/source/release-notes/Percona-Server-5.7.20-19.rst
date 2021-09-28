.. rn:: 5.7.20-19

========================
Percona Server 5.7.20-19
========================

Percona is glad to announce the release of Percona Server 5.7.20-19 
on January 3, 2018. Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.20-19/>`_
and from the :doc:`Percona Software Repositories </installation>`.

This release is based on `MySQL 5.7.20
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-20.html>`_
and includes all the bug fixes in it.
Percona Server 5.7.20-19 is now the current GA (Generally Available) release 
in the 5.7 series. All software developed by Percona is open-source and free.
Details of this release can be found in the `5.7.20-19 milestone on Launchpad
<https://launchpad.net/percona-server/+milestone/5.7.20-19>`_.

New Features
============

* Now MyRocks Storage Engine has *General Availability* status.

* Binary log encryption has been implemented and can be now switched on using
  the :variable:`encrypt_binlog` variable.

* :variable:`innodb_print_lock_wait_timeout_info` variable, introduced in previous 
  release, but undocumented, allows to log information about all lock wait 
  timeout errors.

Bugs Fixed
==========

* Intermediary slave with Blackhole storage engine couldn't record updates 
  from master to its own binary log in case master has 
  :variable:`binlog_rows_query_log_events` option enabled. Bug fixed :bug:`1722789`
  (upstream :mysqlbug:`88057`).

* Help command command in the mysql command line client provided a link to an older 
  version of the |Percona Server| manual. Bug fixed :bug:`1708073`.

* A regression in the ``mysqld_safe`` script forced it to print an extra error when 
  stopping the MySQL service. Bugs fixed :bug:`1738742`.

* Blackhole storage engine was incompatible with newer length limit of the 
  InnoDB index key prefixes. Bug fixed :bug:`1733049` (upstream :mysqlbug:`53588`).

* Heartbeats received by slave were reacted with :table:`mysql.slave_master_info` table
  sync on each of them even with :variable:`sync_master_info` set to zero, causing huge
  increase in write load. Bug fixed :bug:`1708033` (upstream :mysqlbug:`85158`).

MyRocks Changes
===============

* The replication writebatch functionality has been removed from 
  |Percona Server| 5.7 due to unsafety of the current implementation.

* Variables :variable:`rocksdb_block_cachecompressed_hit`, 
  :variable:`rocksdb_block_cachecompressed_miss`, and 
  :variable:`rocksdb_getupdatessince_calls` were renamed to 
  :variable:`rocksdb_block_cache_compressed_hit`, 
  :variable:`rocksdb_block_cache_compressed_miss`, and 
  :variable:`rocksdb_get_updates_since_calls` respectively. 

