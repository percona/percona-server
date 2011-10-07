.. rn:: 1.0.3-4

===============================
|Percona Server| 1.0.3-4 Sakura
===============================

**UPDATE April 27, 2009:** Unfortunately we found significant issues in this release, please use or upgrade to 1.0.3-5

Released on April 8, 2009.

  * move to `MySQL 5.1.33 <http://dev.mysql.com/doc/refman/5.1/en/news-5-1-33.html>`_.

  * move to |InnoDB| Plugin 1.0.3.

  * More reliable replication, ``innodb_overwrite_relay_log_info`` patch now included in the release.

  * Bug :bug:`354302`: Fix assertions for ``UNIV_DEBUG``

  * Bug :bug:`333750` new fix for ``rw_lock`` patch
