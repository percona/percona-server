.. rn:: 5.5.24-26.0

==============================
 |Percona Server| 5.5.24-26.0
==============================

Percona is glad to announce the release of |Percona Server| 5.5.24-26.0 on June 1st, 2012 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.24-26.0/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.5.24 <http://dev.mysql.com/doc/refman/5.5/en/news-5-5-24.html>`_, including all the bug fixes in it, |Percona Server| 5.5.24-26.0 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.24-26.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.24-26.0>`_. 

Features
========

  * Percona :ref:`pam_plugin` has now been integrated into the |Percona Server|. 

  * |Percona Server| has implemented variable :variable:`enforce_storage_engine` which can be used for enforcing the use of a specific storage engine. 
 
  * New column, TOTAL_CONNECTIONS_SSL, has been added in the CLIENT_STATISTICS, THREAD_STATISTICS and USER_STATISTICS tables in the INFORMATION_SCHEMA database.

Bug Fixes
=========

  * A Server acting as a replication slave with the query cache enabled could crash with glibc detected memory corruption. This bug was introduced in MySQL 5.5.18 and Percona Server inherited it from MySQL. Bug fixed :bug:`915814` (*George Ormond Lorch III*).

  * Loading LRU dump was preventing shutdown. Bug fixed :bug:`712055` (*George Ormond Lorch III*).

  * A crash could leave behind an InnoDB temporary table with temporary indexes resulting in an unbootable server. Bug fixed :bug:`999147` (*Laurynas Biveinis*).

  * Since the output file is simply overwritten when dumping the LRU list, we could end up with a partially written dump file in case of a crash, or when making a backup copy of it. Safer approach has been implemente. It now dumps to a temporary file first, and then rename it to the actual dump file. Bug fixed :bug:`686392` (*George Ormond Lorch III*).

  * LRU messages are now more verbose for LRU dump. Bug fixed :bug:`713481` (*George Ormond Lorch III*).

  * Building Percona Server with the Clang compiler resulted in a compiler error. Bug fixed :bug:`997496` (*Alexey Kopytov*).

  * Variable :variable:`thread_statistics` was a reserved word in Percona Server 5.5. As a result, the server variable with that name had to be quoted with backticks when used. Bug fixed :bug:`997036` (*Vladislav Lesin*).
