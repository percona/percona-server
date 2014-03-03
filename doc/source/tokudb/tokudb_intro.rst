.. _tokudb_intro:

=====================
 TokuDB Introduction
=====================

TokuDB is a scalable, ACID and MVCC compliant storage engine that provides indexing-based query improvements, offers online schema modifications, and reduces slave lag for both hard disk drives and flash memory. This storage engine is specifically designed for high performance on write-intensive workloads which is achieved with Fractal Tree indexing.

Available TokuDB features:

 * :ref:`tokudb_compression`

Currently available ALPHA features [#n-1]_:

 * `Multiple Clustering Indexes <http://www.tokutek.com/2009/05/introducing_multiple_clustering_indexes/>`_
 * `Fast Updates with NOAR <http://www.tokutek.com/2013/02/fast-updates-with-tokudb/>`_
 * `Hot Table Optimization <http://www.tokutek.com/2012/06/hot-table-optimization-with-mysql/>`_
 * `TokuDB AUTOINCREMENT implementation <http://www.tokutek.com/2009/07/autoincrement_semantics/>`_
 * `Prelocking index and range scans <https://github.com/Tokutek/mysql56/wiki/Patch-for-prelocking-index-and-range-scans>`_

.. rubric:: Footnotes

.. [#n-1] These features are available only in ALPHA Percona Server with TokuDB builds. They might change or even disappear in a future release.
