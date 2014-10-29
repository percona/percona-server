.. rn:: 5.6.21-70.0

==============================
 |Percona Server| 5.6.21-70.0 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.21-70.0 on October 30th, 2014 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.21-70.0/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.21 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-21.html>`_, including all the bug fixes in it, |Percona Server| 5.6.21-70.0 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.21-70.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.21-70.0>`_. 


New Features
============

 |Percona Server| has ported new :ref:`super-read-only` from *WebScaleSQL*.

 |Percona Server| has implemented :ref:`csv_engine_mode`. This feature also fixes the bug :bug:`1316042` (upstream :mysqlbug:`71091`).
 
 |Percona Server| now supports `Read Free Replication <https://github.com/Tokutek/tokudb-engine/wiki/Replication-Slave-Performance-on-TokuDB>`_ for TokuDB storage engine. 

 TokuDB storage engine package has been updated to version `7.5.2 <http://docs.tokutek.com/tokudb/tokudb-release-notes.html#tokudb-7-5-2>`_.

Bugs Fixed
==========

 Values of ``IP`` and ``DB`` fields in the :ref:`audit_log_plugin` were incorrect. Bug fixed :bug:`1379023`.

 Specifying the ``--malloc-lib`` during the server start would produce two ``LD_PRELOAD`` entries, if a system-wide jemalloc library was installed. Bug fixed :bug:`1382069`.

 In multi-threaded slave replication setup, an incomplete log event group (the one which doesn't end with ``COMMIT``/``ROLLBACK``/``XID``) in a relay log could have caused a replication stall. An incomplete log event group might occur as a result of one of the following events: 1) slave crash; 2) ``STOP SLAVE`` or ``FLUSH LOGS`` command issued at a specific moment; 3) server shutdown at a specific moment. Bug fixed :bug:`1331586` (upstream :mysqlbug:`73066`).

 Purging bitmaps exactly up to the last tracked LSN would abort :ref:`changed_page_tracking`. Bug fixed :bug:`1382336`.

 ``mysql_install_db`` script would silently ignore any mysqld startup failures. Bug fixed :bug:`1382782` (upstream :mysqlbug:`74440`)
 
Other bugs fixed: :bug:`1369950`, :bug:`1335590`, :bug:`1067103`, and :bug:`1282599`.
