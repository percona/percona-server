.. rn:: 5.5.48-37.8

==============================
 |Percona Server| 5.5.48-37.8
==============================

Percona is glad to announce the release of |Percona Server| 5.5.48-37.8 on March 4th, 2016. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.48-37.8/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.48 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-48.html>`_, including all the bug fixes in it, |Percona Server| 5.5.48-37.8 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.48-37.8 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.48-37.8>`_. 

Bugs Fixed
==========

 With :ref:`expanded_innodb_fast_index_creation` enabled, DDL queries involving |InnoDB| temporary tables would cause later queries on the same tables to produce warnings that their indexes were not found in the index translation table. Bug fixed :bug:`1233431`.

 Package upgrade on *Ubuntu* would run ``mysql_install_db`` even though data directory already existed. Bug fixed :bug:`1457614`.

 Starting |MySQL| with ``systemctl`` would fail with timeout if the socket was specified with a custom path. Bug fixed :bug:`1534825`.

 ``mysqldumpslow`` script has been removed because it was not compatible with |Percona Server| extended slow query log format. Please use `pt-query-digest <https://www.percona.com/doc/percona-toolkit/2.2/pt-query-digest.html>`_ from |Percona Toolkit| instead. Bug fixed :bug:`856910`.

 When ``cmake/make/make_binary_distribution`` workflow was used to produce binary tarballs it would produce tarballs with ``mysql-...`` naming instead of ``percona-server-...``. Bug fixed :bug:`1540385`.

Other bugs fixed: :bug:`1521120` and :bug:`1534246`.
