.. rn:: 5.5.46-37.6

==============================
 |Percona Server| 5.5.46-37.6
==============================

Percona is glad to announce the release of |Percona Server| 5.5.46-37.6 on December 4th, 2015. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.46-37.6/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.46 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-46.html>`_, including all the bug fixes in it, |Percona Server| 5.5.46-37.6 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.46-37.6 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.46-37.6>`_. 

Bugs Fixed
==========

 An upstream fix for upstream bug :mysqlbug:`76135` might cause server to stall or hang. Bug fixed :bug:`1519094` (upstream :mysqlbug:`79185`).

 Fixed invalid memory accesses when :program:`mysqldump` was running with ``--innodb-optimize-keys`` option. Bug fixed :bug:`1517444`.

Other bugs fixed: :bug:`1517523`. 
