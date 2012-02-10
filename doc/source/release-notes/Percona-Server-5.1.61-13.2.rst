.. rn:: 5.1.61-13.2

=============================
 |Percona Server| 5.1.61-13.2
=============================

Percona is glad to announce the release of |Percona Server| 5.1.61-13.2 on February 9th, 2012 (Downloads are available from `Percona Server 5.1.61-13.2 downloads <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.61-13.2/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.1.61 <http://dev.mysql.com/doc/refman/5.1/en/news-5-1-61.html>`_, including all the bug fixes in it, |Percona Server| 5.1.61-13.2 is now the current stable release in the 5.1 series. All of |Percona| 's software is open-source and free, all the details of the release can be found in the `5.1.61-13.2 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.1.61-13.2>`_.

Bug Fixes
=========

  * :variable:`innodb_fake_changes` doesn't handle duplicate keys on REPLACE. In some cases it could cause infinite loop. Bug fixed: :bug:`898306` (*Mark Callaghan and Valentine Gostev*).
  
  * Broken Makefile - space instead of tab was causing compile error. Bug fixed: :bug:`911223` (*Oleg Tsarev*).

  * Unintentional change of innodb_version format in 5.1.60. Previously, it was reported as "1.0.x-x.x". In 5.1.60 it is reported as "1.0.17-rel13.1". Bug fixed: :bug:`917246` (*Alexey Kopytov*).

  * Merging `Kewpie <https://launchpad.net/kewpie>`_ into Percona Server tree. Bug fixed: :bug:`909431` (*Patrick Crews*).

  * The statistics line is not being calculated properly because the regexp line does not work well with the Percona Server format. Bug fixed :bug:`856910` (*Oleg Tsarev*).

  * ALTER TABLE ... IMPORT TABLESPACE is holding the data dictionary mutex during the entire import operation. This becomes a problem for innodb_expand_import, because that code scans the tablespace being imported, blocking all queries accessing any InnoDB tables during the scan. It does need to protect some data dictionary operations, but it is possible to release the mutex during the most expensive operation, i.e. the .ibd file scan. Bug fixed :bug:`901775` (*Alexey Kopytov*).

  * Compiler warnings with GCC 4.6 on oneiric hosts. Bug fixed :bug:`902467` (*Laurynas Biveinis*).

  * Percona Server 5.1.58 crashes on a specific query. Bug fixed :bug:`905711` (*Laurynas Biveinis*).

  * Subunit.pm was working incorrectly with months. Bug fixed: :bug:`911237` (*Oleg Tsarev*).
 
  * Removing some of the libraries from Percona-Server-shared-51 as they are already present in Percona-Server-server-51. Bug fixed: :bug:`924477` (*Ignacio Nin*).

  * Resolved the dependency issue in Debian lenny dev package. Bug fixed: :bug:`656933` (*Ignacio Nin*).

  * Resolved the dependency issues in Ubuntu 8.04.4 LTS (Hardy Heron) dev package. Bug fixed: :bug:`803151` (*Ignacio Nin*).

