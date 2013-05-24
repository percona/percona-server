.. rn:: 5.5.31-30.3

==============================
 |Percona Server| 5.5.31-30.3 
==============================

Percona is glad to announce the release of |Percona Server| 5.5.31-30.3 on May 24th, 2013. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.31-30.3/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.31 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-31.html>`_, including all the bug fixes in it, |Percona Server| 5.5.31-30.3 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.31-30.3 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.31-30.3>`_. 

New Features
============

 |Percona Server| has ported the :ref:`atomic_fio` patch from |MariaDB|. This feature adds atomic write support for ``directFS`` filesystem on ``Fusion-io`` devices. This feature implementation is considered BETA quality.
 
 |Percona Server| has introduced :variable:`innodb_read_views_memory` and :variable:`innodb_descriptors_memory` status variables in the :ref:`innodb_show_status` to improve |InnoDB| memory usage diagnostics.

Bug Fixes
=========

 Fix for bug :bug:`1131187` introduced a regression that could cause a memory leak if query cache was used together with |InnoDB|. Bug fixed :bug:`1170103`.

 Fixed ``RPM`` packaging regression that was introduced with the fix for bug :bug:`710799`. This regression caused mysql schema to be missing after the clean ``RPM`` installation. Bug fixed :bug:`1174426`.

 Fixed the ``Percona-Server-shared-55`` and ``Percona-XtraDB-Cluster-shared`` ``RPM`` package dependences. Bug fixed :bug:`1050654`.

 Fixed the upstream bug :mysqlbug:`68999` which caused compiling |Percona Server| to fail on *CentOS* 5 and *Debian* squeeze due to older ``OpenSSL`` version. Bug fixed :bug:`1183610`.

 If a slave was running with its binary log enabled and then restarted with the binary log disabled, :ref:`innodb_recovery_update_relay_log_page` could overwrite the relay log info log with an incorrect position. Bug fixed :bug:`1092593`.

 Fixed the `CVE-2012-5615 <http://www.securiteam.com/cves/2012/CVE-2012-5615.html>`_ vulnerability. This vulnerability would allow remote attacker to detect what user accounts exist on the server. This bug fix comes originally from *MariaDB* (see `MDEV-3909 <https://mariadb.atlassian.net/browse/MDEV-3909>`_). Bug fixed :bug:`1171941`.

 Fixed the `CVE-2012-5627 <http://www.securiteam.com/cves/2012/CVE-2012-5627.html>`_ vulnerability, where an unprivileged |MySQL| account owner could perform brute-force password guessing attack on other accounts efficiently. This bug fix comes originally from *MariaDB* (see `MDEV-3915 <https://mariadb.atlassian.net/browse/MDEV-3915>`_). Bug fixed :bug:`1172090`.

 ``mysql_set_permission`` was failing on *Debian* due to missing ``libdbd-mysql-perl`` package. Fixed by adding the package dependency. Bug fixed :bug:`1003776`.

 Rebuilding *Debian* source package would fail because ``dpatch`` and ``automake`` were missing from build-dep. Bug fixed :bug:`1023575` (*Stephan Adig*).

 Backported the fix for the upstream bug :mysqlbug:`65077` from the |MySQL| 5.6 version, which removed MyISAM internal temporary table mutex contention. Bug fixed :bug:`1179978`.

