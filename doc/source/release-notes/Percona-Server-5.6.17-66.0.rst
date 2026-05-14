.. rn:: 5.6.17-66.0

==============================
 |Percona Server| 5.6.17-66.0 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.17-66.0 on June 11th, 2014 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.17-66.0/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.17 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-17.html>`_, including all the bug fixes in it, |Percona Server| 5.6.17-66.0 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.17-66.0 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.17-66.0>`_.

New Features
============

 |MySQL| benchmarks (``sql-bench`` directory in the |MySQL| distribution) has been made compatible with the TokuDB storage engine.

 |Percona Server| has ported *MariaDB* code enhancement for :ref:`start transaction with consistent snapshot <start_transaction_with_consistent_snapshot>`. This enhancement makes binary log positions consistent with |InnoDB| transaction snapshots.

 |Percona Server| has implemented ability to :ref:`clone a snapshot <snapshot_cloning>` created by ``START TRANSACTION WITH CONSISTENT SNAPSHOT`` in another session.

 TokuDB Storage engine is now available as a separate package that can be :ref:`installed <tokudb_installation>` along with the |Percona Server| 5.6.17-66.0. This feature is currently considered Release Candidate quality.

 |Percona Server| version 5.6 now includes :ref:`handlersocket_page` in addition to |Percona Server| 5.5.

Bugs Fixed
==========

 Fix for :bug:`1225189` introduced a regression in |Percona Server| :rn:`5.6.13-61.0` which could lead to an error when running ``mysql_install_db``. Bug fixed :bug:`1314596`.

 |InnoDB| could crash if workload contained writes to compressed tables. Bug fixed :bug:`1305364`.

 GUI clients such as *MySQL Workbench* could not authenticate with a user defined with ``auth_pam_compat`` plugin. Bug fixed :bug:`1166938`.

 Help in |Percona Server| 5.6 command line client was linking to |Percona Server| 5.1 manual. Bug fixed :bug:`1198775`.

 |InnoDB| redo log resizing could crash when :ref:`changed_page_tracking` was enabled. Bug fixed :bug:`1204072`.

 :ref:`audit_log_plugin` wasn't parsing escape characters correctly in the ``OLD`` format. Bug fixed :bug:`1313696`.

 |Percona Server| version was reported incorrectly in *Debian*/*Ubuntu* packages. Bug fixed :bug:`1319670`. 

Other bugs fixed: :bug:`1318537` (upstream :mysqlbug:`72615`), :bug:`1318874`, :bug:`1285618`, :bug:`1272732`, :bug:`1314568`, :bug:`1271178`, and :bug:`1323014`.


