.. rn:: 5.6.33-79.0

============================
|Percona Server| 5.6.33-79.0
============================

Percona is glad to announce the release of |Percona Server| 5.6.33-79.0 on
October 18th, 2016 (Downloads are available `here
<http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.33-79.0/>`_
and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.33
<http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-33.html>`_, including
all the bug fixes in it, |Percona Server| 5.6.33-79.0 is the current GA release
in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source
and free, all the details of the release can be found in the `5.6.33-79.0
milestone at Launchpad
<https://launchpad.net/percona-server/+milestone/5.6.33-79.0>`_.

New Features
============

 |Percona Server| has implemented support for per-column ``VARCHAR/BLOB``
 :ref:`compression <compressed_columns>` for the |XtraDB| storage engine. This
 also features compression dictionary support, to improve compression ratio for
 relatively short individual rows, such as JSON data.

 A new |TokuDB| :variable:`tokudb_dir_per_db` option has been introduced to
 address two |TokuDB| shortcomings, the :ref:`renaming of data files
 <improved_table_renaming_functionality>` on table/index rename, and the
 ability to :ref:`group data files together
 <improved_directory_layout_functionality>` within a directory that represents
 a single database. This feature is disabled by default.

Bugs Fixed
==========

 After fixing bug :bug:`1540338`, system table engine validation check is no
 longer skipped for tables that don't have an explicit ``ENGINE`` clause in
 a ``CREATE TABLE`` statement. If |MySQL| upgrade statements are replicated,
 and slave does not have the |MyISAM| set as a default storage engine, then
 the ``CREATE TABLE mysql.server`` statement would attempt to create an
 |InnoDB| table and fail because ``mysql_system_tables.sql`` script omitted
 explicit engine setting for this table. Bug fixed :bug:`1600056`.

 :ref:`audit_log_plugin` malformed record could be written after
 :variable:`audit_log_flush` was set to ``ON`` in ``ASYNC`` and ``PERFORMANCE``
 modes. Bug fixed :bug:`1613650`.

 A race condition between :ref:`handlersocket_page` and server shutdown could
 lead to a server stall if shutdown was issued immediately after
 :ref:`handlersocket_page` startup. Bug fixed :bug:`1617198`.

 :ref:`handlersocket_page` could access freed memory on startup. Bug fixed
 :bug:`1617998`.

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

 Fixed memory leaks in :ref:`handlersocket_page`. Bug fixed :bug:`1617949`.

 ``KILL QUERY`` was not behaving consistently and it would hang in some cases.
 Bug fixed :bug:`1621046` (upstream :mysqlbug:`45679`).

 Cipher ``ECDHE-RSA-AES128-GCM-SHA256`` was listed in the `list
 <https://dev.mysql.com/doc/refman/5.6/en/secure-connection-protocols-ciphers.html>`_
 of supported ciphers but it wasn't supported. Bug fixed :bug:`1622034`
 (upstream :mysqlbug:`82935`).

 Successful doublewrite recovery was showed as a warning in the error log. Bug
 fixed :bug:`1622985`.

 Variable :variable:`query_cache_type` couldn't be set to ``0`` if it was
 already set to that value. Bug fixed :bug:`1625501` (upstream
 :mysqlbug:`69396`).

 LRU manager thread could run too long on a server shutdown, causing a server
 crash. Bug fixed :bug:`1626069`.

 ``tokudb_default`` was not recognized by |Percona Server| as a valid row
 format. Bug fixed :bug:`1626206`.

 |InnoDB| ``ANALYZE TABLE`` didn't remove its table from the background
 statistics processing queue. Bug fixed :bug:`1626441` (upstream
 :mysqlbug:`71761`).

 Upstream merge for :mysqlbug:`81657` to 5.6 was incorrect. Bug fixed
 :bug:`1626936` (upstream :mysqlbug:`83124`).

 Fixed multi-threaded slave thread leaks that happened in case of thread create
 failure. Bug fixed :bug:`1619622` (upstream :mysqlbug:`82980`).

 Shutdown waiting for a purge to complete was undiagnosed for the first minute.
 Bug fixed :bug:`1616785`.

 Unnecessary |InnoDB| change buffer merge attempts are now skipped upon reading
 disk pages of non-applying types. Bug fixed :bug:`1618393` (upstream
 :mysqlbug:`75235`).

Other bugs fixed: :bug:`1614439`, :bug:`1614949`, :bug:`1616392` (upstream
:mysqlbug:`82798`), :bug:`1624993` (:ftbug:`736`), :bug:`1613647`,
:bug:`1626002` (upstream :mysqlbug:`83073`), :bug:`904714`, :bug:`1610102`,
:bug:`1610110`, :bug:`1613663`, :bug:`1613728`, :bug:`1613986`, :bug:`1614885`,
:bug:`1615959`, :bug:`1615970`, :bug:`1616333`, :bug:`1616404`, :bug:`1616768`,
:bug:`1617150`, :bug:`1617216`, :bug:`1617267`, :bug:`1618478`, :bug:`1618811`,
:bug:`1618819`, :bug:`1619547`, :bug:`1619572`, :bug:`1620583`, :bug:`1622449`,
:bug:`1622456`, :bug:`1622977`, :bug:`1623011`, :bug:`1624992`
(:tokubug:`1014`), :bug:`1625176`, :bug:`1625187`, :bug:`1626500`,
:bug:`1628417`, :tokubug:`964`, and :ftbug:`735`.
