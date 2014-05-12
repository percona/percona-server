.. _tokudb_multiple_clustering_keys:

==========================
 Multiple Clustering Keys
==========================

TokuDB allows a secondary key to be defined as a clustering key. This means that all of the columns in the table are clustered with the secondary key. |Percona Server| parser and query optimizer support Multiple Clustering Keys when TokuDB engine is used. This means that the query optimizer will avoid primary clustered index reads and replace them by secondary clustered index reads in certain scenarios.

The parser has been extended to support following syntax:

.. code-block:: mysql

  CREATE TABLE ... ( ..., CLUSTERING KEY identifier (column list), ...
  CREATE TABLE ... ( ..., UNIQUE CLUSTERING KEY identifier (column list), ...
  CREATE TABLE ... ( ..., CLUSTERING UNIQUE KEY identifier (column list), ...
  CREATE TABLE ... ( ..., CONSTRAINT identifier UNIQUE CLUSTERING KEY identifier (column list), ...
  CREATE TABLE ... ( ..., CONSTRAINT identifier CLUSTERING UNIQUE KEY identifier (column list), ...

  CREATE TABLE ... (... column type CLUSTERING [UNIQUE] [KEY], ...)
  CREATE TABLE ... (... column type [UNIQUE] CLUSTERING [KEY], ...)

  ALTER TABLE ..., ADD CLUSTERING INDEX identifier (column list), ...
  ALTER TABLE ..., ADD UNIQUE CLUSTERING INDEX identifier (column list), ...
  ALTER TABLE ..., ADD CLUSTERING UNIQUE INDEX identifier (column list), ...
  ALTER TABLE ..., ADD CONSTRAINT identifier UNIQUE CLUSTERING INDEX identifier (column list), ...
  ALTER TABLE ..., ADD CONSTRAINT identifier CLUSTERING UNIQUE INDEX identifier (column list), ...

  CREATE CLUSTERING INDEX identifier ON ...

Examples
========

To create clustered index in TokuDB table, you need to add ``CLUSTERING`` keyword in front of any index that is to be created.

* To create a clustered index while creating a new table:

.. code-block:: mysql

  CREATE TABLE city (city_id int PRIMARY KEY, name varchar(50), CLUSTERING KEY (citry_id)) engine=tokudb; 

* To add a clustered index on already existing table:

.. code-block:: mysql

  ALTER TABLE city ADD CLUSTERING INDEX clstr_key(city_id);

or 

.. code-block:: mysql

   CREATE CLUSTERING INDEX clstr_key ON city(city_id);

Version Specific Information
============================
* :rn:`5.6.17-65.0` - |Percona Server| implemented support for Multiple Clustering Keys

Other Reading
=============

* `Introducing Multiple Clustering Indexes <http://www.tokutek.com/2009/05/introducing_multiple_clustering_indexes/>`_ 
* `Clustering indexes vs. Covering indexes <HTTP://www.tokutek.com/2009/05/clustering_indexes_vs_covering_indexes/>`_

