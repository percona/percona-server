.. rn:: 5.6.25-73.1

==============================
 |Percona Server| 5.6.25-73.1 
==============================

Percona is glad to announce the release of |Percona Server| 5.6.25-73.1 on July 9th, 2015 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.25-73.1/>`_ and from the :doc:`Percona Software Repositories </installation>`).

Based on `MySQL 5.6.25 <http://dev.mysql.com/doc/relnotes/mysql/5.6/en/news-5-6-25.html>`_, including all the bug fixes in it, |Percona Server| 5.6.25-73.1 is the current GA release in the |Percona Server| 5.6 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.6.25-73.1 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.6.25-73.1>`_.

New Features
============

 TokuDB storage engine package has been updated to version `7.5.8 <https://github.com/Tokutek/tokudb-engine/wiki/Release-Notes-for-TokuDB-7.5.8>`_
 
New TokuDB Features
-------------------

* Exposed ft-index fanout as TokuDB option tokudb_fanout, (default=16 range=2-16384).
* Tokuftdump can now provide a summary info with new --summary option.
* Fanout has been serialized in the ft-header and ft_header.fanout value has been exposed in tokuftdump.
* New checkpoint status variables have been implemented:

  * CP_END_TIME - checkpoint end time, time spend in checkpoint end operation in seconds,
  * CP_LONG_END_COUNT - long checkpoint end count, count of end_checkpoint operations that exceeded 1 minute,
  * CP_LONG_END_TIME - long checkpoint end time, total time of long checkpoints in seconds.

* "Engine" status variables are now visible as "GLOBAL" status variables.

TokuDB Bugs Fixed
-----------------

* Fixed assertion with big transaction in ``toku_txn_complete_txn``.
* Fixed assertion that was caused when a transaction had rollback log nodes orphaned in the blocktable.
* Fixed ``ftcxx`` tests failures that were happening when it was run in parallel.
* Fixed multiple test failures for Debian/Ubuntu caused by assertion on ``setlocale()``.
* Status has been refactored to its own file/subsystem within ft-index code to make the it more accessible.
