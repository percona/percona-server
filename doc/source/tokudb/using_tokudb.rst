.. _using_tokudb:

==============
 Using TokuDB
==============

TokuDB storage engine can be used as the rest of the already available |MySQL| engines. You can create the TokuDB tables by specifying TokuDB when creating the tables:

.. code-block:: mysql

   mysql> CREATE TABLE `City` (
    `ID` int(11) NOT NULL AUTO_INCREMENT,
    `Name` char(35) NOT NULL DEFAULT '',
    `CountryCode` char(3) NOT NULL DEFAULT '',
    `District` char(20) NOT NULL DEFAULT '',
    `Population` int(11) NOT NULL DEFAULT '0',
    PRIMARY KEY (`ID`),
    KEY `CountryCode` (`CountryCode`) 
   ) ENGINE=TokuDB

You can also convert existing tables to TokuDB by running ``ALTER TABLE``:

.. code-block:: mysql

   mysql> ALTER TABLE City ENGINE=TokuDB;

Other Reading
=============

* `InnoDB vs TokuDB comparison <http://www.tokutek.com/resources/tokudb-vs-innodb/>`_
* `TokuDB vs InnoDB in timeseries INSERT benchmark <http://www.mysqlperformanceblog.com/2013/09/05/tokudb-vs-innodb-timeseries-insert-benchmark/>`_
* `Considering TokuDB as an engine for timeseries data <http://www.mysqlperformanceblog.com/2013/08/29/considering-tokudb-as-an-engine-for-timeseries-data/>`_
* `Benchmarking Percona Server TokuDB vs InnoDB <http://www.mysqlperformanceblog.com/2013/05/07/benchmarking-percona-server-tokudb-vs-innodb/>`_
