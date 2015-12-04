.. rn:: 5.6.25-73.0

==============================
 |Percona Server| 5.6.25-73.0 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.25-73.0 on June 29th, 2015 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.25-73.0/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.25 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-25.html>`_, including all the bug fixes in it, |Percona Server| 5.6.25-73.0 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.25-73.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.25-73.0>`_.

New Features
============

 |Percona Server| has implemented :ref:`proxy_protocol_support`. The implementation is based on a patch developed by Thierry Fournier.
 
Bugs Fixed
==========

 Symlinks to ``libmysqlclient`` libraries were missing on *CentOS* 6. Bug fixed :bug:`1408500`.

 *RHEL/CentOS* 6.6 ``openssl`` package (``1.0.1e-30.el6_6.9``), containing a fix for CVE-2015-4000, changed the DH key sizes to a minimum of 768 bits. This caused an issue for *MySQL* as it uses 512 bit keys. Fixed by backporting an upstream 5.7 fix that increases the key size to 2048 bits. Bug fixed :bug:`1462856` (upstream :mysqlbug:`77275`).
 
 Some compressed |InnoDB| data pages could be mistakenly considered corrupted, crashing the server. Bug fixed :bug:`1467760` (upstream :mysqlbug:`73689`) *Justin Tolmer*.

 *innochecksum* would fail to check tablespaces in compressed format. The fix for this bug has been ported from *Facebook MySQL* 5.6 patch. Bug fixed :bug:`1100652` (upstream :mysqlbug:`66779`).

 Using concurrent ``REPLACE``, ``LOAD DATA REPLACE`` or ``INSERT ON DUPLICATE KEY UPDATE`` statements in the ``READ COMMITTED`` isolation level or with the :variable:`innodb_locks_unsafe_for_binlog` option enabled could lead to a unique-key constraint violation. Bug fixed :bug:`1308016` (upstream :mysqlbug:`76927`).

 Issuing ``SHOW BINLOG EVENTS`` with an invalid starting binlog position would cause a potentially misleading message in the server error log. Bug fixed :bug:`1409652` (upstream :mysqlbug:`75480`).

 While using :variable:`max_slowlog_size`, the slow query log was rotated every time :variable:`slow_query_log` was enabled, not really checking if the current slow log is indeed bigger than :variable:`max_slowlog_size` or not. Bug fixed :bug:`1416582`.

 Fixed possible server assertions when :ref:`backup_locks` are used. Bug fixed :bug:`1432494`.

 If :variable:`query_response_time_range_base` was set as a command line option or in a configuration file, its value would not take effect until the first flush was made. Bug fixed :bug:`1453277` (*Preston Bennes*).

 ``mysqld_safe`` script is now searching for ``libjemalloc.so.1`` library, needed by TokuDB, in the ``basedir`` directory as well. Bug fixed :bug:`1462338`.

 Prepared XA transactions could cause a debug assertion failure during the shutdown. Bug fixed :bug:`1468326`.

 Variable :variable:`log_slow_sp_statements` now supports skipping the logging of stored procedures into the slow log entirely with new ``OFF_NO_CALLS`` option. Bug fixed :bug:`1432846`.

 *TokuDB HotBackup* library is now automatically loaded with ``mysqld_safe`` script. Bug fixed :bug:`1467443`.
 
Other bugs fixed: :bug:`1457113`, :bug:`1380895`, and :bug:`1413836`.
