.. rn:: 5.5.42-37.1

==============================
 |Percona Server| 5.5.42-37.1
==============================

Percona is glad to announce the release of |Percona Server| 5.5.42-37.1 on March 4th, 2015. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.42-37.1/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.42 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-42.html>`_, including all the bug fixes in it, |Percona Server| 5.5.42-37.1 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.42-37.1 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.42-37.1>`_. 

Bugs Fixed
==========

 ``RPM`` pre-install script assumed that the ``PID`` file was always located in the :variable:`datadir`. If it was not, during installation, wrong assumption could be made if the server was running or not. Bug fixed :bug:`1201896`.
 
 ``SHOW GRANTS`` displayed only the privileges granted explicitly to the named account. Other effectively available privileges were not displayed. Fixed by implementing :ref:`extended_show_grants` feature. Bug fixed :bug:`1354988` (upstream :mysqlbug:`53645`).
 
 |InnoDB| lock monitor output was printed even if it was not requested. Bug fixed :bug:`1418996`.

 The stored procedure key was made consistent with other keys in the :ref:`slow_extended_55` by replacing space with an underscore. Bug fixed :bug:`1419230`.
 
Other bugs fixed: :bug:`1408232`, :bug:`1415843` (upstream :mysqlbug:`75642`), bug fixed :bug:`1407941`, and bug fixed :bug:`1424568` (upstream :bug:`75868`).
