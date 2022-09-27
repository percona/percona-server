.. _myrocks_intro:

============================
Percona MyRocks Introduction
============================

`MyRocks <http://myrocks.io>`_ is a storage engine
for `MySQL <https://www.mysql.com>`_ based on `RocksDB <http://rocksdb.org/>`_,
an embeddable, persistent key-value store.
*Percona MyRocks* is an implementation
for `Percona Server for MySQL <https://www.percona.com/software/percona-server>`_.

The RocksDB store is based on the log-structured merge-tree (or LSM
tree). It is optimized for fast storage and combines outstanding
space and write efficiency with acceptable read performance. As a
result, MyRocks has the following advantages compared to other storage
engines, if your workload uses fast storage, such as SSD:

* Requires less storage space
* Provides more storage endurance
* Ensures better IO capacity

.. toctree::
   :maxdepth: 1

   install
   limitations
   differences
   column-family
   variables
   information-schema-tables
   performance-schema-tables

