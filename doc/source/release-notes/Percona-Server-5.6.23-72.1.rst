.. rn:: 5.6.23-72.1

==============================
 |Percona Server| 5.6.23-72.1 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.23-72.1 on March 4th, 2015 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.23-72.1/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.23 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-23.html>`_, including all the bug fixes in it, |Percona Server| 5.6.23-72.1 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.23-72.1 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.23-72.1>`_.

New Features
============

 TokuDB storage engine package has been updated to version `7.5.6 <http://docs.tokutek.com/tokudb/tokudb-release-notes.html#tokudb-version-7-x>`_.
 
Bugs Fixed
==========

 ``RPM`` pre-install script assumed that the ``PID`` file was always located in the :variable:`datadir`. If it was not, during installation, wrong assumption could be made if the server was running or not. Bug fixed :bug:`1201896`.

 ``SHOW GRANTS`` displayed only the privileges granted explicitly to the named account. Other effectively available privileges were not displayed. Fixed by implementing :ref:`extended_show_grants` feature. Bug fixed :bug:`1354988` (upstream :mysqlbug:`53645`).

 |InnoDB| lock monitor output was printed even if it was not requested. Bug fixed :bug:`1418996`.

 The stored procedure key was made consistent with other keys in the :ref:`slow_extended` by replacing space with an underscore. Bug fixed :bug:`1419230`.

 Some ``--big-test`` MTR tests were failing for |Percona Server| because they weren't updated. Bug fixed :bug:`1419827`.
 
Other bugs fixed: :bug:`1408232`, :bug:`1385036`, :bug:`1386363`, and :bug:`1420303`.

