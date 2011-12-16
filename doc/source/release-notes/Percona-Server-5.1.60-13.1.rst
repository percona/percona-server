.. rn:: 5.1.60-13.1

=============================
 |Percona Server| 5.1.60-13.1
=============================

Percona is glad to announce the release of |Percona Server| 5.1.60-13.1 on December 16, 2011 (Downloads are available from `Percona Server 5.1.60-13.1 downloads <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.60-13.1/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.1.60 <http://dev.mysql.com/doc/refman/5.1/en/news-5-1-60.html>`_, including all the bug fixes in it, |Percona Server| 5.1.60-13.1 is now the current stable release in the 5.1 series. All of |Percona| 's software is open-source and free, all the details of the release can be found in the `5.1.60-13.1 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.1.60-13.1>`_.

Bug Fixes
=========

  * ``SHOW SLAVE STATUS`` could give incorrect output with master-master replication and using SET user variables. This could only occur with a sever having both master-master replication and ``--log-slave-updates`` enabled. This is also filed in MySQL bug tracker, but not fixed in upstream MySQL at the time of this Percona Server release. Bug Fixed: :bug:`860910` (*Alexey Kopytov*)

  * MyISAM repair-by-sort buffer cannot be more than 4GB even on 64bit architectures. With this bug fix, both the server option ``--myisam-sort-buffer-size`` and the |myisamchk| ``--sort-buffer-size`` can be set to values over 4GB on 64bit systems. For users with large MyISAM tables, this could be a great improvement in |myisamchk|, CREATE INDEX and ALTER TABLE performance. Bug Fixed: :bug:`878404` (*Alexey Kopytov*)

  * The atomic operations used in :ref:`Response Time Distribution <response_time_distribution>` on 32bit systems could (in theory) be optimized incorrectly by the compiler. This has not been observed in the wild and may only be an issue with future compilers. With this bug fixed, we have corrected the inline assembly to always produce correct compiled code even if future compilers implement new optimizations. Bug Fixed: :bug:`878022` (*Laurynas Biveinis*)

  * GCC 4.6 has expanded diagnostics and compiler warnings. We have audited and fixed these warnings for Percona Server 5.1, finding that the warnings were benign. Bug Fixed :bug:`878164` (*Laurynas Biveinis*)
