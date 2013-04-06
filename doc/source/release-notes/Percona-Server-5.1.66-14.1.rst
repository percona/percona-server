.. rn:: 5.1.66-14.1

=============================
 |Percona Server| 5.1.66-14.1
=============================

Percona is glad to announce the release of |Percona Server| 5.1.66-14.1 on October 26th, 2012 (Downloads are available from `Percona Server 5.1.66-14.1 downloads <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.66-14.1/>`_ and from the `Percona Software Repositories <http://www.percona.com/doc/percona-server/5.1/installation.html>`_).

Based on `MySQL 5.1.66 <http://dev.mysql.com/doc/refman/5.1/en/news-5-1-66.html>`_, including all the bug fixes in it, |Percona Server| 5.1.66-14.1 is now the current stable release in the 5.1 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.1.66-14.1 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.1.66-14.1>`_.

Bug Fixes
=========

 |Percona Server| would disconnect clients if ``gdb`` was attached and detached. This was caused by wrong signal handling. Bugs fixed :bug:`805805` and :bug:`1060136` (*Laurynas Biveinis*).

 Fixed the upstream |MySQL| bug `#61509 <http://bugs.mysql.com/bug.php?id=61509>`_. Crash in YaSSL was causing SSL tests failures on *Ubuntu Oneiric* hosts. Bug fixed :bug:`902471` (*Laurynas Biveinis*).

 Fixed the upstream |MySQL| bug `#62856 <http://bugs.mysql.com/bug.php?id=62856>`_ where check for "stack overrun" wouldn't work with gcc-4.6 and caused the server crash. Bug fixed :bug:`902472` (*Laurynas Biveinis*).

 Resolved the *Ubuntu* |Percona Server| package conflicts with upstream packages. Bug fixed :bug:`907499` (*Ignacio Nin*).

 |Percona Server| would crash on a DDL statement if an |XtraDB| internal SYS_STATS table was corrupted or overwritten. This is now fixed by detecting the corruption and creating a new SYS_STATS table. Bug fixed :bug:`978036` (*Laurynas Biveinis*).

 Postfix would crash on CentOS/RHEL 6.x when using shared dependency (``libmysqlclient.so``). Fixed by building packages with OpenSSL support rather than the bundled YaSSL library. Bug fixed :bug:`1028240` (*Ignacio Nin*).

 Fix for bug :bug:`905334` regressed by adding debug-specific code with missing local variable that would break the debug build. Bug fixed :bug:`1046389` (*Laurynas Biveinis*).

 Fix for bug :bug:`686534` caused a regression by mishandling LRU list mutex. Bug fixed :bug:`1053087` (*George Ormond Lorch III*).
 
 Fixed the upstream |MySQL| bug `#67177 <http://bugs.mysql.com/bug.php?id=67177>`_ |Percona Server| 5.1 was incompatible with *Automake* 1.12. Bug fixed :bug:`1064953` (*Alexey Kopytov*).
 
 Flashcache support resulted in confusing messages in the error log on |Percona Server| startup, even when flashcache was not used. This was fixed by adding new boolean option :variable:`flashcache`. When set to 0 (default), flashcache checks are disabled and when set to 1 checks are enabled. Error message has been made more verbose including error number and system error message as well. Bug fixed :bug:`747032` (*Sergei Glushchenko*).

 Custom server builds would crash when compiled with a non-default maximum number of indexes per table. Upstream MySQL bugs: `#54127 <http://bugs.mysql.com/bug.php?id=54127>`_, `#61178 <http://bugs.mysql.com/bug.php?id=61178>`_, `#61179 <http://bugs.mysql.com/bug.php?id=61179>`_ and `#61180 <http://bugs.mysql.com/bug.php?id=61180>`_. Bug fixed :bug:`1042517` (*Sergei Glushchenko*).

 Cleaned up the test duplicates in regular atomic operation tests. Bug fixed :bug:`1039931` (*Laurynas Biveinis*).
