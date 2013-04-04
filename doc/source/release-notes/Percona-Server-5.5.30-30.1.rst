.. rn:: 5.5.30-30.1

==============================
 |Percona Server| 5.5.30-30.1 
==============================

Percona is glad to announce the release of |Percona Server| 5.5.30-30.1 on March 7th, 2012 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.30-30.1/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.5.30 <http://dev.mysql.com/doc/relnotes/mysql/5.5/en/news-5-5-30.html>`_, including all the bug fixes in it, |Percona Server| 5.5.30-30.1 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.30-30.1 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.30-30.1>`_. 

Bug Fixes
=========

 Minor optimization cleanup by removing unnecessary ``current_thd`` calls. Bug fixed :bug:`1121794` (*Alexey Kopytov*).
 
 Fixed the regression introduced with the fix for bug :bug:`1083058` which caused unnecessary mutex re-acquisitions in adaptive flushing. Bug fixed :bug:`1117067` (*Laurynas Biveinis*).

 |Percona Server| would do unnecessary slow log stats accounting even with the slow log disabled. Bug fixed :bug:`1123915` (*Alexey Kopytov*).
 
 Optimization cleanup to avoid calls related to extended slow query log stats when this feature is disabled. Bug fixed :bug:`1123921` (*Alexey Kopytov*).

 The static ``srv_pass_corrupt_table`` variable could share CPU cache lines with |InnoDB| row counters, which resulted in high false sharing effect in high-concurrent workloads. Bug fixed :bug:`1125259` (*Alexey Kopytov*).

 Fixed the regression introduced with fix for bug :bug:`791030` in |Percona Server| :rn:`5.5.13-20.4` by implementing an optimized version of the same function.  Bug fixed :bug:`1130655` (*Alexey Kopytov*).

 Potentially improve server performance by implementing an optimized version of the ``my_strnxfrm_simple`` function. Bug fixed :bug:`1132350`, upstream bug fixed :mysqlbug:`68476` (*Alexey Kopytov*).

 Potentially improve server performance by implementing an optimized version of the ``skip_trailing_space`` function. Bug fixed :bug:`1132351`, upstream bug fixed :mysqlbug:`68477` (*Alexey Kopytov*).

Other bug fixes: bug fixed :bug:`1089265` (*Laurynas Biveinis*).
