.. rn:: 5.6.34-79.1

============================
|Percona Server| 5.6.34-79.1
============================

Percona is glad to announce the release of |Percona Server| 5.6.34-79.1 on
November 23rd, 2016 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.34-79.1/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.34
<http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-34.html>`_, including
all the bug fixes in it, |Percona Server| 5.6.34-79.1 is the current GA release
in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source
and free, all the details of the release can be found in the `5.6.34-79.1
milestone at Launchpad
<https://launchpad.net/percona-server/+milestone/5.6.34-79.1>`_.

Deprecated Features
===================

 :ref:`scalability_metrics_plugin` feature is now deprecated. Users who have
 installed this plugin but are not using its capability are advised to
 uninstall the plugin due to known crashing bugs.

Bugs Fixed
==========

 When a stored routine would call an administrative command such as
 ``OPTIMIZE TABLE``, ``ANALYZE TABLE``, ``ALTER TABLE``, ``CREATE/DROP INDEX``,
 etc. the effective value of :variable:`log_slow_sp_statements` was overwritten
 by the value of :variable:`log_slow_admin_statements`. Bug fixed
 :bug:`719368`.

 :ref:`threadpool` ``thread limit reached`` and ``failed to create thread``
 messages are now printed on the first occurrence as well. Bug fixed
 :bug:`1636500`.

 :table:`INFORMATION_SCHEMA.TABLE_STATISTICS` and
 :table:`INFORMATION_SCHEMA.INDEX_STATISTICS` tables were not correctly updated
 for |TokuDB|. Bug fixed :bug:`1629448`.

Other bugs fixed: :bug:`1633061`, :bug:`1633430`, and :bug:`1635184`.
