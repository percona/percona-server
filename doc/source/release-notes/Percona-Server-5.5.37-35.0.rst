.. rn:: 5.5.37-35.0

==============================
 |Percona Server| 5.5.37-35.0 
==============================

Percona is glad to announce the release of |Percona Server| 5.5.37-35.0 on May 6th, 2014. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.37-35.0/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.37 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-37.html>`_, including all the bug fixes in it, |Percona Server| 5.5.37-35.0 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.37-35.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.37-35.0>`_. 

New Features
============

 |Percona Server| now supports :ref:`scalability_metrics_plugin`.

 |Percona Server| now supports :ref:`audit_log_plugin`.

 |Percona Server| packages are now available for *Ubuntu* 14.04.
 
Bugs Fixed
==========

 |Percona Server| couldn't be built with *Bison* 3.0. Bug fixed :bug:`1262439`,upstream :mysqlbug:`71250` (*Ryan Gordon*).

 Backported the upstream fix for overflow which would caused replication SQL thread to fail to execute events. Bug fixed :bug:`1070255` (upstream :mysqlbug:`67352`).
 
 |Percona Server| debug packages were not built for the previous releases. Bug fixed :bug:`1298352`.

 Queries that no longer exceed :variable:`long_query_time` were written to the slow query log if they matched the previous :variable:`long_query_time` value when :variable:`slow_query_log_use_global_control` variable was set to ``all``. Bug fixed :bug:`1016991`.

 When writing audit plugins it was not possible to get notifications for general-log events without enabling the general-log. Bug fixed :bug:`1182535` (upstream :mysqlbug:`60782`).

 ``mysqld_safe`` did not correctly parse :variable:`flush_caches` and :variable:`numa_interleave` options. Bug fixed :bug:`1231110`.

 :ref:`threadpool` would handle a new client connection without notifying Audit Plugin. Bug fixed :bug:`1282008`.

 Fixed a performance issue in extending tablespaces if running under ``fusionIO`` with :ref:`atomic writes <atomic_fio>` enabled. Bug fixed :bug:`1286114` (*Jan Lindström*).
 
 Previous implementation of the :variable:`log_slow_rate_type` set to ``query`` with :variable:`log_slow_rate_limit` feature would log every nth query deterministically instead of each query having a 1/n probability to get logged. Fixed by randomly selecting the queries to be logged instead of logging every nth query. Bug fixed :bug:`1287650`.
 
 |Percona Server| source files were referencing *Maatkit* instead of |Percona Toolkit|. Bug fixed :bug:`1174779`.

 Maximum allowed value for :variable:`log_slow_rate_limit` was ``ULONG_MAX`` (ie. either ``4294967295`` or ``18446744073709551615``, depending on the platform). As it was unreasonable to configure the slow log for every four billionth session/query, new maximum allowed value is set to ``1000``. Bug fixed :bug:`1290714`.

Other bugs fixed: :bug:`1272732`.
