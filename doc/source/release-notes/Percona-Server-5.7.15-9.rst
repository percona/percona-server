.. rn:: 5.7.15-9

=========================
|Percona Server| 5.7.15-9
=========================

Percona is glad to announce the GA (Generally Available) release of |Percona
Server| 5.7.15-9 on October 21st, 2016 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.7/Percona-Server-5.7.15-9/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.7.15
<http://dev.mysql.com/doc/relnotes/mysql/5.7/en/news-5-7-15.html>`_, including
all the bug fixes in it, |Percona Server| 5.7.15-9 is the current GA release in
the |Percona Server| 5.7 series. All of Percona's software is open-source and
free, all the details of the release can be found in the `5.7.15-9 milestone at
Launchpad <https://launchpad.net/percona-server/+milestone/5.7.15-9>`_

New Features
============

 A new TokuDB :variable:`tokudb_dir_per_db` option has been introduced to
 address two TokuDB shortcomings, the :ref:`renaming of data files
 <improved_table_renaming_functionality>` on table/index rename, and the
 ability to :ref:`group data files together
 <improved_directory_layout_functionality>` within a directory that represents
 a single database. This feature is enabled by default.

Bugs Fixed
==========

 :ref:`audit_log_plugin` malformed record could be written after
 :variable:`audit_log_flush` was set to ``ON`` in ``ASYNC`` and ``PERFORMANCE``
 modes. Bug fixed :bug:`1613650`.

 Running ``SELECT DISTINCT x...ORDER BY y LIMIT N,N`` could lead to a server
 crash. Bug fixed :bug:`1617586`.

 Workloads with statements that take non-transactional locks (``LOCK TABLES``,
 global read lock, and similar) could have caused deadlocks when running
 under :ref:`threadpool` with high priority queue enabled and
 :variable:`thread_pool_high_prio_mode` set to ``transactions``. Fixed by
 placing such statements into the high priority queue even with the above
 :variable:`thread_pool_high_prio_mode` setting. Bugs fixed :bug:`1619559` and
 :bug:`1374930`.

 Fixed memory leaks in :ref:`audit_log_plugin`. Bug fixed :bug:`1620152`
 (upstream :mysqlbug:`71759`).

 Server could crash due to a ``glibc`` bug in handling short-lived detached
 threads. Bug fixed :bug:`1621012` (upstream :mysqlbug:`82886`).

 ``QUERY_RESPONSE_TIME_READ`` and ``QUERY_RESPONSE_TIME_WRITE`` were returning
 ``QUERY_RESPONSE_TIME`` table data if accessed  through a name that is not
 full uppercase. Bug fixed :bug:`1552428`.

 Cipher ``ECDHE-RSA-AES128-GCM-SHA256`` was listed in the `list
 <https://dev.mysql.com/doc/refman/5.7/en/secure-connection-protocols-ciphers.html>`_
 of supported ciphers but it wasn't supported. Bug fixed :bug:`1622034`
 (upstream :mysqlbug:`82935`).

 Successful recovery of a torn page from the doublewrite buffer was showed as a
 warning in the error log. Bug fixed :bug:`1622985`.

 LRU manager threads could run too long on a server shutdown, causing a server
 crash. Bug fixed :bug:`1626069`.

 ``tokudb_default`` was not recognized by |Percona Server| as a valid row
 format. Bug fixed :bug:`1626206`.

 InnoDB ``ANALYZE TABLE`` didn't remove its table from the background
 statistics processing queue. Bug fixed :bug:`1626441` (upstream
 :mysqlbug:`71761`).

 Upstream merge for :mysqlbug:`81657` to 5.6 was incorrect. Bug fixed
 :bug:`1626936` (upstream :mysqlbug:`83124`).

 Fixed multi-threaded slave thread leaks that happened in case of thread create
 failure. Bug fixed :bug:`1619622` (upstream :mysqlbug:`82980`).

 Shutdown waiting for a purge to complete was undiagnosed for the first minute.
 Bug fixed :bug:`1616785`.

Other bugs fixed: :bug:`1614439`, :bug:`1614949`, :bug:`1624993`
(:ftbug:`736`), :bug:`1613647`, :bug:`1615468`, :bug:`1617828`, :bug:`1617833`,
:bug:`1626002` (upstream :mysqlbug:`83073`), :bug:`904714`, :bug:`1610102`,
:bug:`1610110`, :bug:`1613728`, :bug:`1614885`, :bug:`1615959`, :bug:`1616333`,
:bug:`1616404`, :bug:`1616768`, :bug:`1617150`, :bug:`1617216`, :bug:`1617267`,
:bug:`1618478`, :bug:`1618819`, :bug:`1619547`, :bug:`1619572`, :bug:`1620583`,
:bug:`1622449`, :bug:`1623011`, :bug:`1624992` (:tokubug:`1014`), :ftbug:`735`,
:bug:`1626500`, :bug:`1628913`, :bug:`952920`, and :tokubug:`964`.
