.. rn:: 5.6.16-64.0-tokudb

========================================================
 |Percona Server| 5.6.16-64.0-tokudb with TokuDB engine
========================================================

Percona is glad to announce the first **ALPHA** release of |Percona Server| 5.6.16-64.0-tokudb with TokuDB engine on March 3rd, 2014. Downloads are available `here <http://www.percona.com/downloads/TESTING/Percona-5.6-TokuDB/545/>`_ and from the :ref:`Percona Software Repositories <tokudb_installation>`.

Based on |Percona Server| :rn:`5.6.16-64.0` including all the features and bug fixes in it, and on TokuDB 7.1.5-rc.3, |Percona Server| 5.6.16-64.0-tokudb is the first **ALPHA** release in the |Percona Server| 5.6 with TokuDB series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.16-64.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.16-64.0>`_.

New Features
============

|Percona Server| now supports TokuDB storage engine. More information on how to :ref:`install <tokudb_installation>` and :ref:`use <using_tokudb>` TokuDB can be found in the documentation. This feature is currently considered ALPHA quality.

Available TokuDB features:

 * :ref:`tokudb_compression`

Currently available ALPHA features [#n-1]_:

 * `Multiple Clustering Indexes <http://www.tokutek.com/2009/05/introducing_multiple_clustering_indexes/>`_
 * `Fast Updates with NOAR <http://www.tokutek.com/2013/02/fast-updates-with-tokudb/>`_
 * `Hot Table Optimization <http://www.tokutek.com/2012/06/hot-table-optimization-with-mysql/>`_
 * `TokuDB AUTOINCREMENT implementation <http://www.tokutek.com/2009/07/autoincrement_semantics/>`_
 * `Prelocking index and range scans <https://github.com/Tokutek/mysql56/wiki/Patch-for-prelocking-index-and-range-scans>`_

.. [#n-1] These features are available only in ALPHA Percona Server with TokuDB builds. They might change or even disappear in a future release.
