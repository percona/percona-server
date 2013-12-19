.. rn:: 5.1.72-14.10

===============================
 |Percona Server| 5.1.72-14.10 
===============================

Percona is glad to announce the release of |Percona Server| 5.1.72-14.10 on October 28th, 2013 (Downloads are available from `Percona Server 5.1.72-14.10 downloads <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.72-14.10/>`_ and from the `Percona Software Repositories <http://www.percona.com/doc/percona-server/5.1/installation.html>`_).

Based on `MySQL 5.1.72 <http://dev.mysql.com/doc/relnotes/mysql/5.1/en/news-5-1-72.html>`_, this release will include all the bug fixes in it. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.1.72-14.10 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.1.72-14.10>`_.

Bugs Fixed
==========
 
 Due to an incompatible upstream change that went in unnoticed, the page cleaner thread would attempt to replay any file operations it encountered. In most cases this were a no-op, but there were race conditions for certain ``DDL`` operations that would have resulted in server crash. Bug fixed :bug:`1217002`.

 ``apt-get upgrade`` of |Percona Server| would fail in post-installation step if server failed to start. Bug fixed :bug:`1002500`.

 Fixed the ``libssl.so.6`` dependency issues in binary tarballs releases. Bug fixed :bug:`1172916`.

 |Percona Server| could crash server could crash while accessing ``BLOB`` or ``TEXT`` columns in |InnoDB| tables if :ref:`innodb_fake_changes_page` was enabled. Bug fixed :bug:`1188168`.

 A server could crash due to a race condition between a :table:`INNODB_CHANGED_PAGES` query and a bitmap file delete by ``PURGE CHANGED_PAGE_BITMAP`` or directly on the file system. Bug fixed :bug:`1191580`.

Other bug fixes: bug fixed :bug:`1191589`.
