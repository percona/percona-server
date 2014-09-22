.. _tokudb_intro:

=====================
 TokuDB Introduction
=====================

|Percona Server| is fully compatible with the separately available TokuDB storage engine package. TokuDB engine is not packaged with any |Percona Server| 5.6 download. The TokuDB engine must be separately downloaded and then enabled as a plug-in component. This package can be installed alongside standard |Percona Server| 5.6 releases starting with :rn:`5.6.19-67.0` and does not require any specially adapted version of |Percona Server|.

The TokuDB storage engine is a scalable, ACID and MVCC compliant storage engine that provides indexing-based query improvements, offers online schema modifications, and reduces slave lag for both hard disk drives and flash memory. This storage engine is specifically designed for high performance on write-intensive workloads which is achieved with Fractal Tree indexing.

Key TokuDB features available in the current production release: 

 * :ref:`tokudb_compression`
 * `Multiple Clustering Indexes <http://www.tokutek.com/2009/05/introducing_multiple_clustering_indexes/>`_
 * `Hot Table Optimization <http://www.tokutek.com/2012/06/hot-table-optimization-with-mysql/>`_
 * `Prelocking index and range scans <https://github.com/Tokutek/tokudb-engine/wiki/Patch-for-prelocking-index-and-range-scans>`_


Some features that were available in older alpha releases have since been discontinued:

 * `Fast Updates with NOAR <http://www.tokutek.com/2013/02/fast-updates-with-tokudb/>`_
 * `TokuDB AUTOINCREMENT implementation <http://www.tokutek.com/2009/07/autoincrement_semantics/>`_

TokuDB is released by Tokutek Inc. under a modified form of the GNU GPL v2 license. You can view a copy of the modified license `here <https://github.com/Tokutek/ft-index/blob/master/README-TOKUDB>`_. 


