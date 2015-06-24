.. rn:: 5.5.44-37.3

==============================
 |Percona Server| 5.5.44-37.3
==============================

Percona is glad to announce the release of |Percona Server| 5.5.44-37.3 on June 29th, 2015. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.44-37.3/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.44 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-44.html>`_, including all the bug fixes in it, |Percona Server| 5.5.44-37.3 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.44-37.3 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.44-37.3>`_. 

Bugs Fixed
==========

 Symlinks to ``libmysqlclient`` libraries were missing on *CentOS* 6. Bug fixed :bug:`1408500`.

 *RHEL/CentOS* 6.6 ``openssl`` package (1.0.1e-30.el6_6.9), containing a fix for CVE-2015-4000, changed the DH key sizes to a minimum of 768 bits. This caused an issue for *MySQL* as it uses 512 bit key by default. Bug fixed :bug:`1462856` (upstream :mysqlbug:`77275`).

 *innochecksum* would fail to check table files when compressed row format with ``KEY_BLOCK_SIZE`` set to ``4`` was used. Fix for this bug has been ported from *Facebook MySQL* 5.1 patch. Bug fixed :bug:`1100652` (upstream :bug:`66779`).

 Selecting wrong position with ``SHOW BINLOG EVENTS`` would cause a potentially misleading message in the server error log. Bug fixed :bug:`1409652`.

 While using :variable:`max_slowlog_size`, the slow query log was rotated every time :variable:`slow_query_log` was enabled, not really checking if the current slow log is indeed bigger than :variable:`max_slowlog_size` or not. Bug fixed :bug:`1416582`.

 If :variable:`query_response_time_range_base` was set as a command line option or in a configuration file, its value would not take effect until the first flush was made. Bug fixed :bug:`1453277` (*Preston Bennes*).

 XA prepared transactions, that have ``UPDATE`` (and not ``INSERT``) undo records, were not properly recovered. Bug fixed :bug:`1468301`.

Other bugs fixed: :bug:`1380895`.  
