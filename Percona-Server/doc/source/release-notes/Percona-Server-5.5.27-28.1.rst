.. rn:: 5.5.27-28.1

===============================
 |Percona Server| 5.5.27-28.1
===============================

Percona is glad to announce the release of |Percona Server| 5.5.27-28.1 on September 5th, 2012 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.27-28.1/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.5.27 <http://dev.mysql.com/doc/refman/5.5/en/news-5-5-27.html>`_, including all the bug fixes in it, |Percona Server| 5.5.27-28.1 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.27-28.1 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.27-28.1>`_. 

Bug Fixes
=========

 |Percona Server| :rn:`5.5.27-28.0` would crash or deadlock in |XtraDB| buffer pool code. This was caused by incorrect mutex handling in porting of the recently introduced InnoDB code to |XtraDB|. Bug fixed :bug:`1038225` (*Laurynas Biveinis*).

 Variables :variable:`innodb_adaptive_flushing_method` and :variable:`innodb_flush_neighbor_pages` would not correctly translate some values internally. Bug fixed :bug:`1039384` (*Laurynas Biveinis*).

