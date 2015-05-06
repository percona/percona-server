.. rn:: 5.5.43-37.2

==============================
 |Percona Server| 5.5.43-37.2
==============================

Percona is glad to announce the release of |Percona Server| 5.5.43-37.2 on May 8th, 2015. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.43-37.2/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.43 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-43.html>`_, including all the bug fixes in it, |Percona Server| 5.5.43-37.2 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.43-37.2 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.42-37.2>`_. 

Bugs Fixed
==========

 A server binary as distributed in binary tarballs could fail to load on different systems due to an unsatisfied ``libssl.so.6`` dynamic library dependency. This was fixed by replacing the single binary tarball with multiple tarballs depending on the *OpenSSL* library available in the distribution: 1) ssl100 - for all *Debian/Ubuntu* versions except *Squeeze/Lucid* (``libssl.so.1.0.0 => /usr/lib/x86_64-linux-gnu/libssl.so.1.0.0 (0x00007f2e389a5000)``); 2) ssl098 - only for *Debian Squeeze* and *Ubuntu Lucid* (``libssl.so.0.9.8 => /usr/lib/libssl.so.0.9.8 (0x00007f9b30db6000)``); 3) ssl101 - for *CentOS 6* and *CentOS 7* (``libssl.so.10 => /usr/lib64/libssl.so.10 (0x00007facbe8c4000)``); 4) ssl098e - to be used only for *CentOS 5* (``libssl.so.6 => /lib64/libssl.so.6 (0x00002aed5b64d000)``). Bug fixed :bug:`1172916`.
 
 ``mysql_install_db`` would make the server produce an "``Error in my_thread_global_end(): 1 threads didn't exit``" error message. While this error does not prevent ``mysql_install_db`` from completing successfully, its presence might cause any ``mysql_install_db``-calling script to return an error as well. This is a regression introduced by backporting fix for bug :bug:`1319904`. Bug fixed :bug:`1402074`.
 
 A string literal containing an invalid UTF-8 sequence could be treated as falsely equal to a UTF-8 column value with no invalid sequences. This could cause invalid query results. Bug fixed :bug:`1247218` by a fix ported from *MariaDB* (`MDEV-7649 <https://mariadb.atlassian.net/browse/MDEV-7649>`_).

 |Percona Server| ``.deb`` binaries were built without fast mutexes. Bug fixed :bug:`1433980`.

 Installing or uninstalling the :ref:`audit_log_plugin` would crash the server if the :variable:`audit_log_file` variable was pointing to an inaccessible path. Bug fixed :bug:`1435606`.

 The :variable:`audit_log_file` would point to random memory area if the :ref:`audit_log_plugin` was not loaded into server, and then installed with ``INSTALL PLUGIN``, and :file:`my.cnf` contained :variable:`audit_log_file` setting. Bug fixed :bug:`1437505`.

 |Percona Server| client ``.deb`` packages were built with with ``EditLine`` instead of ``Readline``. Further, a client built with ``EditLine`` could display incorrectly on *PuTTY* SSH client after its window resize. Bugs fixed :bug:`1266386` and :bug:`1332822` (upstream :mysqlbug:`63130` and :mysqlbug:`69991`).

Other bugs fixed: :bug:`1436138` (upstream :mysqlbug:`76505`).

