.. rn:: 5.5.23-25.3

==============================
 |Percona Server| 5.5.23-25.3
==============================

Percona is glad to announce the release of |Percona Server| 5.5.23-25.3 on May 16, 2012 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.23-25.3/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.5.23 <http://dev.mysql.com/doc/refman/5.5/en/news-5-5-23.html>`_, including all the bug fixes in it, |Percona Server| 5.5.23-25.3 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.23-25.3 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.23-25.3>`_. 


Bug Fixes
=========

  * Percona Server would crash on a DDL statement if an XtraDB internal SYS_STATS table was corrupted or overwritten. This is now fixed by detecting the corruption and creating a new SYS_STATS table. Bug fixed :bug:`978036` (*Laurynas Biveinis*).
