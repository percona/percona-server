.. rn:: 5.6.12-60.4

==============================
 |Percona Server| 5.6.12-60.4
==============================

Percona is glad to announce the second Release Candidate release of |Percona Server| 5.6.12-60.4 on June 27th, 2013 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.12-60.4/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.6.12 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-12.html>`_, including all the bug fixes in it, |Percona Server| 5.6.12-60.4 is the second RC release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.12-60.4 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.12-60.4>`_.

New Features
============

 |Percona Server| has implemented support for supplementary groups for :ref:`pam_plugin`.

Bug Fixes
==========

 ``mysql_install_db`` did not work properly in debug and *Valgrind* builds. Bug fixed :bug:`1179359`.

 Fixed yum dependencies that were causing conflicts in CentOS 6.3 during installation. Bug fixed :bug:`1051874`.

 The RPM installer script had the :term:`datadir` hardcoded to :file:`/var/lib/mysql` instead of using ``my_print_defaults`` function to get the correct :term:`datadir` info. Bug fixed :bug:`1181753`.

 Fixed the upstream bug :mysqlbug:`68354` that could cause server to crash when performing update or join on ``Federated`` and ``MyISAM`` tables with one row, due to incorrect interaction between ``Federated`` storage engine and the optimizer. Bug fixed :bug:`1182572`.

 Fixed the compiler warnings caused by :ref:`atomic_fio` when building |Percona Server| on non-Linux platforms. Bug fixed :bug:`1189429`.

Other bugs fixed: bug fixed :bug:`1188516`, bug fixed :bug:`1130731` and bug fixed :bug:`1133266`.
