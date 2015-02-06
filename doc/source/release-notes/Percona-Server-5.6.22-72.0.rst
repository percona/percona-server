.. rn:: 5.6.22-72.0

==============================
 |Percona Server| 5.6.22-72.0 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.22-72.0 on February 6th, 2015 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.22-72.0/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.22 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-22.html>`_, including all the bug fixes in it, |Percona Server| 5.6.22-72.0 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.22-72.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.22-72.0>`_.

New Features
============

 |Percona Server| is now able to log the query's response times into :ref:`separate <rtd_rw_split>` ``READ`` and ``WRITE`` ``INFORMATION_SCHEMA`` tables. Two new ``INFORMATION_SCHEMA`` tables :table:`QUERY_RESPONSE_TIME_READ` and :table:`QUERY_RESPONSE_TIME_WRITE` have been implemented for ``READ`` and ``WRITE`` queries correspondingly.

 New ``ps_tokudb_admin`` script has been implemented to make the TokuDB Storage engine installation :ref:`easier <tokudb_quick_install>`.

 |Percona Server| now supports :ref:`online_gtid_deployment`. This enables GTID to be deployed on existing replication setups without making the master ``read-only`` and stopping all the slaves. This feature was ported from the Facebook `branch <https://github.com/facebook/mysql-5.6>`_.
 
Bugs Fixed
==========

 ``SET STATEMENT ... FOR <statement>`` would crash the server if it could not execute the ``<statement>`` due to: 1) if the ``<statement>`` was Read-Write in a Read-Only transaction (bug :bug:`1387951`), 2) if the ``<statement>`` needed to re-open an already open temporary table and would fail to do so (bug :bug:`1412423`), 3) if the ``<statement>`` needed to commit implicitly the ongoing transaction and the implicit commit would fail (bug :bug:`1418049`). 

 TokuDB storage engine would fail to load after the upgrade on *CentOS* 5 and 6. Bug fixed :bug:`1413956`.

 Fixed a potential low-probability crash in |XtraDB| linear read-ahead code. Bug fixed :bug:`1417953`.

 Setting the :variable:`max_statement_time` per query had no effect. Bug fixed :bug:`1376934`.

Other bugs fixed: :bug:`1407941`, :bug:`1415843` (upstream :mysqlbug:`75642`).

