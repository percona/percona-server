.. rn:: 5.5.28-29.1

===============================
 |Percona Server| 5.5.28-29.1
===============================

Percona is glad to announce the release of |Percona Server| 5.5.28-29.1 on October 26th, 2012 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.28-29.1/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.5.28 <http://dev.mysql.com/doc/refman/5.5/en/news-5.5.28.html>`_, including all the bug fixes in it, |Percona Server| 5.5.28-29.1 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.28-29.1 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.28-29.1>`_. 

New Features
============

 |Percona Server| has ported Twitter's |MySQL| ``NUMA`` patch. This patch implements :ref:`innodb_numa_support` as it prevents imbalanced memory allocation across NUMA nodes. 

Bug Fixes
=========

  |Percona Server| would disconnect clients if ``gdb`` was attached and detached. This was caused by wrong signal handling. Bugs fixed :bug:`805805` and :bug:`1060136` (*Laurynas Biveinis*).

  Fixed the upstream |MySQL| :upbug:`62856`, where slave server would crash after update statement. Bug fixed :bug:`1053342` (*George Ormond Lorch III*).

  Reads from tablespaces being deleted would result in buffer pool locking error. Bug fixed :bug:`1053342` (*Stewart Smith*).

  Resolved the *Ubuntu* |Percona Server| package conflicts with upstream packages. Bug fixed :bug:`907499` (*Ignacio Nin*).  

  Crash-resistant replication would break with binlog XA transaction recovery. If a crash would happened between XA PREPARE and COMMIT stages, the prepared |InnoDB| transaction would not have the slave position recorded and thus would fail to update it once it is replayed during binlog crash recovery. Bug fixed :bug:`1012715` (*Laurynas Biveinis*).
