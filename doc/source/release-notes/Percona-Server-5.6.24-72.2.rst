.. rn:: 5.6.24-72.2

==============================
 |Percona Server| 5.6.24-72.2 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.24-72.2 on May 8th, 2015 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.24-72.2/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.24 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-24.html>`_, including all the bug fixes in it, |Percona Server| 5.6.24-72.2 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.24-72.2 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.24-72.2>`_.

New Features
============

 TokuDB storage engine package has been updated to version `7.5.7 <https://github.com/Tokutek/tokudb-engine/wiki/Release-Notes-for-TokuDB-7.5.7>`_
 
Bugs Fixed
==========

 A server binary as distributed in binary tarballs could fail to load on different systems due to an unsatisfied ``libssl.so.6`` dynamic library dependency. This was fixed by replacing the single binary tarball with multiple tarballs depending on the *OpenSSL* library available in the distribution: 1) ssl100 - for all *Debian/Ubuntu* versions except *Squeeze/Lucid* (``libssl.so.1.0.0 => /usr/lib/x86_64-linux-gnu/libssl.so.1.0.0 (0x00007f2e389a5000)``); 2) ssl098 - only for *Debian Squeeze* and *Ubuntu Lucid* (``libssl.so.0.9.8 => /usr/lib/libssl.so.0.9.8 (0x00007f9b30db6000)``); 3) ssl101 - for *CentOS 6* and *CentOS 7* (``libssl.so.10 => /usr/lib64/libssl.so.10 (0x00007facbe8c4000)``); 4) ssl098e - to be used only for *CentOS 5* (``libssl.so.6 => /lib64/libssl.so.6 (0x00002aed5b64d000)``). Bug fixed :bug:`1172916`.
 
 Executing a stored procedure containing a subquery would leak memory. Bug fixed :bug:`1380985` (upstream :mysqlbug:`76349`).

 A slave server restart could cause a ``1755`` slave SQL thread error if multi-threaded slave was enabled. This is a regression introduced by fix for bug :bug:`1331586` in :rn:`5.6.21-70.0`. Bug fixed :bug:`1380985`.

 A string literal containing an invalid UTF-8 sequence could be treated as falsely equal to a UTF-8 column value with no invalid sequences. This could cause invalid query results. Bug fixed :bug:`1247218` by a fix ported from *MariaDB* (`MDEV-7649 <https://mariadb.atlassian.net/browse/MDEV-7649>`_).

 |Percona Server| ``.deb`` binaries were built without fast mutexes. Bug fixed :bug:`1433980`.

 Installing or uninstalling the :ref:`audit_log_plugin` would crash the server if the :variable:`audit_log_file` variable was pointing to an inaccessible path. Bug fixed :bug:`1435606`.

 The :variable:`audit_log_file` would point to random memory area if the :ref:`audit_log_plugin` was not loaded into server, and then installed with ``INSTALL PLUGIN``, and :file:`my.cnf` contained :variable:`audit_log_file` setting. Bug fixed :bug:`1437505`.

 A specific trigger execution on the master server could cause a slave assertion error under row-based replication. The trigger would satisfy the following conditions: 1) it sets a savepoint; 2) it declares a condition handler which releases this savepoint; 3) the trigger execution passes through the condition handler. Bug fixed :bug:`1438990` (upstream :mysqlbug:`76727`).

 |Percona Server| client packages were built with with ``EditLine`` instead of ``Readline``. This was causing history file produced by the client no longer easy to read. Further, a client built with ``EditLine`` could display incorrectly on *PuTTY* SSH client after its window resize. Bugs fixed :bug:`1266386`, :bug:`1296192` and :bug:`1332822` (upstream :mysqlbug:`63130`, upstream :mysqlbug:`72108` and :mysqlbug:`69991`).

 Unlocking a table while holding the backup binlog lock would cause an implicit erroneous backup lock release, and a subsequent server crash or hang at the later explicit backup lock release request. Bug fixed :bug:`1371827`.

 Initializing slave threads or executing ``CHANGE MASTER TO`` statement would crash a debug build if autocommit was disabled and at least one of slave info tables were configured as tables. Bug fixed :bug:`1393682`.

Other bugs fixed: :bug:`1372263` (upstream :mysqlbug:`72080`), :bug:`1436138` (upstream :mysqlbug:`76505`), :bug:`1182949` (upstream :mysqlbug:`69453`), :bug:`1111203` (upstream :mysqlbug:`68291`), and :bug:`1384566` (upstream :mysqlbug:`74615`).

