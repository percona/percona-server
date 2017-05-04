.. rn:: 5.5.53-38.4

============================
|Percona Server| 5.5.53-38.4
============================

Percona is glad to announce the release of |Percona Server| 5.5.53-38.4 on
November 18th, 2016. Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.53-38.4/>`_
and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.53
<http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-53.html>`_, including
all the bug fixes in it, |Percona Server| 5.5.53-38.4 is now the current stable
release in the 5.5 series. All of |Percona|'s software is open-source and free,
all the details of the release can be found in the `5.5.53-38.4 milestone at
Launchpad <https://launchpad.net/percona-server/+milestone/5.5.53-38.4>`_. 

Removed Features
================

 :ref:`scalability_metrics_plugin` feature has been removed. **WARNING:** if
 you have :variable:`scalability_metrics_control` variable in your
 :file:`my.cnf` configuration file you'll need to remove it, otherwise server
 won't be able to start.

Bugs Fixed
==========

 When a stored routine would call an "administrative" command such as
 ``OPTIMIZE TABLE``, ``ANALYZE TABLE``, ``ALTER TABLE``, ``CREATE/DROP INDEX``,
 etc. the effective value of :variable:`log_slow_sp_statements` was overwritten
 by the value of :variable:`log_slow_admin_statements`. Bug fixed :bug:`719368`.

 :ref:`threadpool` ``thread limit reached`` and ``failed to create thread``
 messages are now printed on the first occurrence as well. Bug fixed
 :bug:`1636500`.

Other bugs fixed: :bug:`1612076`, :bug:`1633061`, :bug:`1633430`, and
:bug:`1635184`.
