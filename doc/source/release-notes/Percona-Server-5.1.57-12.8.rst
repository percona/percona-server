.. rn:: 5.1.57-12.8

==============================
 |Percona Server| 5.1.57-12.8
==============================

Percona is glad to announce the release of |Percona Server| 5.1.57-12.8 on June 8th, 2011 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.57-12.8/>`_ and from the `Percona Software Repositories <http://www.percona.com/doc/percona-server/5.1/installation.html>`_). 

|Percona Server| 5.1.57-12.8 is now the current stable release in the 5.1 series. It is is based on |MySQL| 5.1.57. 

Bug Fixes
=========

  * Fixed |InnoDB| I/O code so that the interrupted system calls are restarted if they are interrupted by a signal. |InnoDB| I/O code was not fully conforming to the standard on POSIX systems, causing a crash with an assertion failure when receiving a signal on ``pwrite()``. Bug Fixed: LP :bug:`764395` / |MySQL| bug `#60788 <http://bugs.mysql.com/bug.php?id=60788>`_ (*A. Kopytov*).

  * The maximum value for ``innodb_use_purge_threads`` has been corrected to 32 (maximum number of parallel threads in a parallelized operation). The :ref:`innodb_purge_thread` patch accepted a value up to 64 for the ``innodb_use_purge_thread`` variable, leading to an assertion failure for greater than the actual maximum. Bug Fixed: :bug:`755017` (*L. Biveinis*).

  * Increased the treshold of 600 seconds during the import, which was causing issues when importing big tables. Bug Fixed :bug:`684829` (*Y. Kinoshita*).

  * The innodb_use_sys_stats_table feature could cause a crash under high concurrency Bug Fixed :bug:`791092` (*Y. Kinoshita*).

Other Changes
=============

  * ``HandlerSocket``, a NoSQL plugin for |MySQL|, has been updated to the latest stable version as April 11th, 2011.

  * The list of authors of the plugins used have been corrected. Bug Fixes: :bug:`723050` (*Y. Kinoshita*).
