.. rn:: 5.6.10-60.2

==============================
 |Percona Server| 5.6.10-60.2
==============================

Percona is glad to announce the Beta release of |Percona Server| 5.6.10-60.2 on February 27th, 2013 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.10-60.2/>`_ and from the EXPERIMENTAL `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.6.10 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-10.html>`_, including all the bug fixes in it, |Percona Server| 5.6.10-60.2 is the first BETA release in the Percona Server 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.10-60.2 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.10-60.2>`_.

New Features
=============

 :ref:`expanded_innodb_fast_index_creation` has been ported from |Percona Server| 5.5

 Ported the :ref:`threadpool` patch from |MariaDB|. This feature enables the server to keep the top performance even with the increased number of client connections.

Bug Fixes
==========

 Fixed the upstream :mysqlbug:`68116` that caused the server crash with assertion error when |InnoDB| monitor with verbose lock info was used under heavy load. This bug is affecting only ``-debug`` builds. Bug fixed :bug:`1100178` (*Laurynas Biveinis*).
