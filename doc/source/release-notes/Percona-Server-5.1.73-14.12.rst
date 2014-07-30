.. rn:: 5.1.73-14.12

===============================
 |Percona Server| 5.1.73-14.12 
===============================

Percona is glad to announce the release of |Percona Server| 5.1.73-14.12 on July 31st, 2014 (Downloads are available from `Percona Server 5.1.73-14.12 downloads <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.73-14.12>`_ and from the `Percona Software Repositories <http://www.percona.com/doc/percona-server/5.1/installation.html>`_).

Based on `MySQL 5.1.73 <http://dev.mysql.com/doc/relnotes/mysql/5.1/en/news-5-1-73.html>`_, this release will include all the bug fixes in it. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.1.73-14.12 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.1.73-14.12>`_.

.. note::

   Packages for Debian Wheezy and Ubuntu 14.04 (trusty) are not available for this release due to conflict with newer ``libmysqlclient18`` packages available in those releases. 

Bugs Fixed
==========
 
 |Percona Server| couldn't be built with *Bison* 3.0. Bug fixed :bug:`1262439` (upstream :mysqlbug:`71250`).

 :ref:`Ignoring Query Cache Comments <ignoring_comments>` feature could cause server crash. Bug fixed :bug:`705688`.

 Database administrator password could be seen in plain text when ``debconf-get-selections`` was executed. Bug fixed :bug:`1018291`.

 If |XtraDB| variable :variable:`innodb_dict_size` was set, the server could attempt to remove a used index from the in-memory InnoDB data dictionary, resulting in a server crash. Bugs fixed :bug:`1250018` and :bug:`758788`.

 Ported a fix from |MySQL| 5.5 for upstream bug :mysqlbug:`71315` that could cause a server crash if a malformed ``GROUP_CONCAT`` function call was followed by another ``GROUP_CONCAT`` call. Bug fixed :bug:`1266980`.

 MTR tests from binary tarball didn't work out of the box. Bug fixed :bug:`1158036`.

 |InnoDB| did not handle the cases of asynchronous and synchronous I/O requests completing partially or being interrupted. Bugs fixed :bug:`1262500` (upstream :mysqlbug:`54430`).

 |Percona Server| version was reported incorrectly in *Debian*/*Ubuntu* packages. Bug fixed :bug:`1319670`.

 |Percona Server| source files were referencing *Maatkit* instead of |Percona Toolkit|. Bug fixed :bug:`1174779`.

 The |XtraDB| version number in ``univ.i`` was incorrect. Bug fixed :bug:`1277383`.

Other bug fixes: :bug:`1272732`, :bug:`1167486`, and :bug:`1314568`.
