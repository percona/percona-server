.. _PS-5.6.51-92.0:

================================================================================
*Percona Server for MySQL* 5.6.51-92.0
================================================================================

:Date: November 11, 2021

`Percona Server for MySQL <https://www.percona.com/software/mysql-database/percona-server>`_ 5.6.51-92.0
includes all the features and bug fixes available in
`MySQL 5.6.51 Community Edition <https://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-51.html>`_
in addition to enterprise-grade features developed by Percona. 

Percona Server for MySQL 5.6.51-92.0 is an update of the Percona Server for MySQL 5.6 series. This update is only available to `MySQL 5.6 Post EOL Support <https://www.percona.com/services/support/mysql-support/5-6-eol-support>`_ subscribers. 

Bugs Fixed
================================================================================

* `Bug #32391415 <https://github.com/percona/percona-server/pull/4426/commits/91772a342b9d767c0715b7873a164138324c3e8e>`__: The mysql_change_user() fails with unknown collation. 

* `Bug #31576731 <https://github.com/percona/percona-server/pull/4426/commits/192ea7f8bc063593903be0a811b3e79316558e8e>`__: The `innodb_ft_total_cache_size <https://dev.mysql.com/doc/refman/5.6/en/innodb-parameters.html#sysvar_innodb_ft_total_cache_size>`__ was not capped when set to total memory.

* `Bug#32620378, Bug#32620398 <https://github.com/percona/percona-server/pull/4426/commits/44728b4063f8bffb4ff0ad287184cab3d73f16cf>`__: The innodb_memcached plugin code allowed integer underflow. 

* `BUG#30366310 <https://github.com/percona/percona-server/pull/4422/commits/34da47f235180cb07d5495630c8990e042e15dba>`__: Using a function to assign default values to two or more variables caused server exit. 

* `Bug #31674599 <https://github.com/percona/percona-server/pull/4422/commits/4537a61d610950a0225a8f54618cc8075d6d2108>`__: The udf_int() function could cause a server exit when encountering an incorrect name error.

* `BUG#31774422 <https://github.com/percona/percona-server/pull/4422/commits/8f21eca42fce7adaa45829df3c5fe7ea680f1d7e>`__: A transaction commit during the update of a system variable, which causes a binary log rotation, while concurrently reading variables from another connection may cause a deadlock situation.

* `Bug #31899685 <https://github.com/percona/percona-server/pull/4422/commits/489bb255119503f4667e95290ee5c6c0635a078f>`__: The Data Dictionary table instance open and close sequence may cause the InnoDB table share instance to be evicted from the dictionary cache.

* `Bug #32316323 <https://github.com/percona/percona-server/pull/4422/commits/b4824d6237c4cf3ed57f5ab5cab4db56a7b575c7>`__: A buffer miscalculation in a Microsoft Windows 10 zipped archive could cause a server exit. 

* :jirabug:`PS-7746`: Possible double call to `free_share()` in ha_innobase::open()



