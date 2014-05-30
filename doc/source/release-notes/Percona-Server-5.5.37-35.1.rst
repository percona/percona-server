.. rn:: 5.5.37-35.1

==============================
 |Percona Server| 5.5.37-35.1 
==============================

Percona is glad to announce the release of |Percona Server| 5.5.37-35.1 on June 3rd, 2014. Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.37-35.1/>`_ and from the :doc:`Percona Software Repositories </installation>`.

Based on `MySQL 5.5.37 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-37.html>`_, including all the bug fixes in it, |Percona Server| 5.5.37-35.1 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.37-35.1 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.37-35.1>`_. 

Bugs Fixed
==========

 InnoDB could crash if workload contained writes to compressed tables. Bug fixed :bug:`1305364`.
 
 GUI clients such as *MySQL Workbench* could not authenticate with a user defined with ``auth_pam_compat`` plugin. Bug fixed :bug:`1166938`.

 Help in |Percona Server| 5.5 command line client was linking to |Percona Server| 5.1 manual. Bug fixed :bug:`1198775`.

 :ref:`audit_log_plugin` wasn't parsing escape characters correctly in the ``OLD`` format. Bug fixed :bug:`1313696`.

 |Percona Server| version was reported incorrectly in *Debian*/*Ubuntu* packages. Bug fixed :bug:`1319670`.

Other bugs fixed: :bug:`1272732`, :bug:`1219833`, :bug:`1271178`, and :bug:`1314568`.
