.. _tokudb_intro:

=====================
 TokuDB Introduction
=====================

|Percona Server| provides the separate TokuDB storage engine package since :rn:`5.6.17-66.0`. This package can be :ref:`installed <tokudb_installation>` along side with |Percona Server| 5.6 releases. Before the separate package implementation |Percona Server| has added support for TokuDB storage engine in the :rn:`5.6.16-64.0-tokudb` release and TokuDB engine was only available in special |Percona Server| with TokuDB storage engine releases. 

TokuDB is a scalable, ACID and MVCC compliant storage engine that provides indexing-based query improvements, offers online schema modifications, and reduces slave lag for both hard disk drives and flash memory. This storage engine is specifically designed for high performance on write-intensive workloads which is achieved with Fractal Tree indexing.

Available TokuDB features:

 * :ref:`tokudb_compression`
 * `Multiple Clustering Indexes <http://www.tokutek.com/2009/05/introducing_multiple_clustering_indexes/>`_
 * `Hot Table Optimization <http://www.tokutek.com/2012/06/hot-table-optimization-with-mysql/>`_
 * `Prelocking index and range scans <https://github.com/Tokutek/tokudb-engine/wiki/Patch-for-prelocking-index-and-range-scans>`_


These features were available only in older alpha, :rn:`5.6.16-64.0-tokudb` and :rn:`5.6.16-64.2-tokudb`, Percona Server with TokuDB releases:

 * `Fast Updates with NOAR <http://www.tokutek.com/2013/02/fast-updates-with-tokudb/>`_
 * `TokuDB AUTOINCREMENT implementation <http://www.tokutek.com/2009/07/autoincrement_semantics/>`_

