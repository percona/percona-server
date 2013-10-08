.. rn:: 5.1.69-14.7

==============================
 |Percona Server| 5.1.69-14.7 
==============================

Percona is glad to announce the release of |Percona Server| 5.1.69-14.7 on June 10th, 2013 (Downloads are available from `Percona Server 5.1.69-14.7 downloads <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.69-14.7/>`_ and from the `Percona Software Repositories <http://www.percona.com/doc/percona-server/5.1/installation.html>`_).

Based on `MySQL 5.1.69 <http://dev.mysql.com/doc/relnotes/mysql/5.1/en/news-5-1-69.html>`_, this release will include all the bug fixes in it. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.1.69-14.7 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.1.69-14.7>`_.

Bug Fixes
=========

 In *Ubuntu* Precise ``libmysqlclient18`` package was chosen from the distribution's repository instead of Percona's which could lead to package conflicts. Bug fixed :bug:`1174271`.
 
 Fixed the ``RPM`` ``Percona-Server-shared-compat`` package naming issue that could lead to unresolved package dependencies when installing |Percona Server| 5.1. Bug fixed :bug:`893860`.
 
 If a slave was running with its binary log enabled and then restarted with the binary log disabled, :ref:`innodb_recovery_update_relay_log_page` could overwrite the relay log info log with an incorrect position. Bug fixed :bug:`1092593`.
 
 The log tracker thread was unaware of the situation when the oldest untracked log records are overwritten by the new log data. In some corner cases this could lead to assertion errors in the log parser or bad changed page data. Bug fixed :bug:`1108613`.
 
 |Percona Server| wouldn't start if the :ref:`changed_page_tracking` was enabled and variable :variable:`innodb_flush_method` was set to ``ALL_O_DIRECT``. Bug fixed :bug:`1131949`.
 
 Fixed the ``RPM`` package dependencies for different major versions of |Percona Server|. Bug fixed :bug:`1167109`.
 
 Fixed the `CVE-2012-5627 <http://www.securiteam.com/cves/2012/CVE-2012-5627.html>`_ vulnerability, where an unprivileged |MySQL| account owner could perform brute-force password guessing attack on other accounts efficiently. This bug fix comes originally from *MariaDB* (see `MDEV-3915 <https://mariadb.atlassian.net/browse/MDEV-3915>`_). Bug fixed :bug:`1172090`.

 ``OpenSSL`` libraries were not found in 32-bit builds due to a typo. Bug fixed :bug:`1175447`.

 Query to the :table:`INNODB_CHANGED_PAGES` table would cause server to stop with an I/O error if a bitmap file in the middle of requested LSN range was missing. Bug fixed :bug:`1179974`.

 Server would crash if an :table:`INNODB_CHANGED_PAGES` query is issued that has an empty LSN range and thus does not need to read any bitmap files. Bug fixed :bug:`1184427`.
 
 Incorrect schema definition for the :ref:`user_stats` tables in ``INFORMATION_SCHEMA`` (:table:`CLIENT_STATISTICS`, :table:`INDEX_STATISTICS`, :table:`TABLE_STATISTICS`, :table:`THREAD_STATISTICS`, and :table:`USER_STATISTICS`) led to the maximum counter values being limited to 32-bit signed integers. Fixed so that these values can be 64-bit unsigned integers now. Bug fixed :bug:`714925`.
 
 ``mysql_set_permission`` was failing on *Debian* due to missing ``libdbd-mysql-perl`` package. Fixed by adding the package dependency. Bug fixed :bug:`1003776`.
 
 :ref:`changed_page_tracking` used to hold the log system mutex for the log reads needlessly, potentially limiting performance on write-intensive workloads. Bug fixed :bug:`1171699`.
 
 Missing path separator between the directory and file name components in a bitmap file name could stop the server starting if the :variable:`innodb_data_home_dir` variable didn't have the path separator at the end. Bug fixed :bug:`1181887`.
 
 A warning is now returned if a bitmap file I/O error occurs after an :table:`INNODB_CHANGED_PAGES` query started returning data to indicate an incomplete result set. Bug fixed :bug:`1185040`.
 
 Fixed the upstream bug :mysqlbug:`69379` which caused |MySQL| clients to return bogus error number for ``host-not-found`` errors on *Ubuntu* 13.04. Bug fixed :bug:`1186690`.
 
 Under very rare circumstances, deleting a zero-size bitmap file at the right moment would make server stop with an I/O error if changed page tracking is enabled. Bug fixed :bug:`1184517`.
 
 The :table:`INNODB_CHANGED_PAGES` table couldn't be queried if the log tracker wasn't running. Bug fixed :bug:`1185304`.
 
Other bug fixes: bug fixed :bug:`1174346`, bug fixed :bug:`1160951`, bug fixed :bug:`1079688`, bug fixed :bug:`1132412`, bug fixed :bug:`1153651`.
