.. rn:: 5.5.50-38.0

==============================
 |Percona Server| 5.5.50-38.0
==============================

Percona is glad to announce the release of |Percona Server| 5.5.50-38.0 on July 8th, 2016. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.50-38.0/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.50 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-50.html>`_, including all the bug fixes in it, |Percona Server| 5.5.50-38.0 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.50-38.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.50-38.0>`_. 

New Features
============

 |Percona Server| has implemented protocol support for :ref:`TLS 1.1 and TLS 1.2 <extended_tls_support>`. This implementation turns off TLS v1.0 support by default.

Bugs Fixed
==========

 Querying the :table:`GLOBAL_TEMPORARY_TABLES` table would cause server crash if temporary table owning threads would execute new queries. Bug fixed :bug:`1581949`.

 The :variable:`innodb_log_block_size` feature attempted to diagnose the situation where the logs have been created with a log block value that differs from the current :variable:`innodb_log_block_size` setting. But this diagnostics came too late, and a misleading error ``No valid checkpoints found`` was produced first, aborting the startup. Bug fixed :bug:`1155156`.

 ``AddressSanitizer`` build with ``LeakSanitizer`` enabled was failing at ``gen_lex_hash`` invocation. Bug fixed :bug:`1580993` (upstream :mysqlbug:`80014`).

 :file:`ssl.cmake` file was broken when custom OpenSSL build was used. Bug fixed :bug:`1582639` (upstream :mysqlbug:`61619`).

 ``mysqlbinlog`` did not free the existing connection before opening a new remote one. Bug fixed :bug:`1587840` (upstream :mysqlbug:`81675`).

 Fixed memory leaks in ``mysqltest``. Bugs fixed :bug:`1582718` and :bug:`1588318`.

 Fixed memory leaks in ``mysqlcheck``. Bug fixed :bug:`1582741`.

 Fixed memory leak in ``mysqlbinlog``. Bug fixed :bug:`1582761` (upstream :mysqlbug:`78223`).

 Fixed memory leaks in ``mysqldump``. Bug fixed :bug:`1587873` and :bug:`1588845` (upstream :mysqlbug:`81714`).

 Fixed memory leak in ``innochecksum``. Bug fixed :bug:`1587873`.

 Fixed memory leak in non-existing defaults file handling. Bug fixed :bug:`1588344`.

 Fixed memory leak in ``mysqlslap``. Bug fixed :bug:`1587873`.

Other bugs fixed: :bug:`1588169`, :bug:`1588386`, :bug:`1529885`, :bug:`1587757`, :bug:`1587426` (upstream, :mysqlbug:`81657`), :bug:`1587527`, :bug:`1588650`, and :bug:`1589819`.
