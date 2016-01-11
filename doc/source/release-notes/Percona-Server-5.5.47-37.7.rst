.. rn:: 5.5.47-37.7

==============================
 |Percona Server| 5.5.47-37.7
==============================

Percona is glad to announce the release of |Percona Server| 5.5.47-37.7 on January 12th, 2015. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.47-37.7/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.47 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-47.html>`_, including all the bug fixes in it, |Percona Server| 5.5.47-37.7 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.47-37.7 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.47-37.7>`_. 

Bugs Fixed
==========

 Running ``OPTIMIZE TABLE`` or ``ALTER TABLE`` without the ``ENGINE`` clause would silently change table engine if :variable:`enforce_storage_engine` variable was active. This could also result in system tables being changed to incompatible storage engines, breaking server operation. Bug fixed :bug:`1488055`.

Other bugs fixed: :bug:`1179451`, :bug:`1524763`, and :bug:`1530102`. 
