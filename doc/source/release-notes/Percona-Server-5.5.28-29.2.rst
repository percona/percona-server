.. rn:: 5.5.28-29.2

===============================
 |Percona Server| 5.5.28-29.2
===============================

Percona is glad to announce the release of |Percona Server| 5.5.28-29.2 on December 7th, 2012 (Downloads are available `here <http://www.percona.com/downloads/Percona-Server-5.5/Percona-Server-5.5.28-29.2/>`_ and from the `Percona Software Repositories <http://www.percona.com/docs/wiki/repositories:start>`_).

Based on `MySQL 5.5.28 <http://dev.mysql.com/doc/refman/5.5/en/news-5.5.28.html>`_, including all the bug fixes in it, |Percona Server| 5.5.28-29.2 is now the current stable release in the 5.5 series. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.5.28-29.2 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.5.28-29.2>`_. 

New Features
============

  Multiple bitmap file support for :ref:`changed_page_tracking` has been implemented.

  |Percona Server| now has an option to build the binary tarball with enabled debugging. New flag :option:`--dubug` has been added to the build script, that will create a build with the debug-enabled binaries. New binaries will have ``-debug`` appended in case it is a debug build, ie. ``mysqld-debug``.

  :ref:`handlersocket_page` has been updated to version 1.1.0 (rev. 83d8f3af176e1698acd9eb3ac5174700ace40fe0).

  ``innochecksum`` has been extended with an option to read file format information from a given InnoDB data file. As only the first page needs to be read to detect the format/version information, it can also be used on a running server. This information can be useful when doing the :ref:`innodb_expand_import_page`.
 
  :ref:`innodb_fake_changes_page` has been improved by fetching the sibling pages. 

  :ref:`innodb_fast_checksum_page` feature has now been deprecated. 

Bug Fixes
=========

  :variable:`innodb_fake_changes` didn't handle duplicate keys on ``REPLACE``. In some cases this could cause infinite loop. Bug fixed :bug:`898306` (*Mark Callaghan*, *Laurynas Biveinis*).

  Fixed the package dependencies for CentOS 6, that caused conflicts during the install. Bug fixed :bug:`908620` (*Ignacio Nin*).

  :variable:`innodb_fake_changes` would allocate too many extents on ``UPDATE``. In some cases this could cause infinite loop. Bug fixed :bug:`917942` (*Mark Callaghan*, *Laurynas Biveinis*).

  Fixed the upstream bug :mysqlbug:`67737`. ``mysqldump`` test would fail due to mixing STDOUT and STDERR. Bug fixed :bug:`959198` (*Stewart Smith*).

  Although fake change transactions downgrade the requested exclusive (X) row locks to shared (S) locks, these S locks prevent X locks from being taken and block the real changes. This fix introduces a new option :variable:`innodb_locking_fake_changes` which, when set to ``FALSE``, makes fake transactions not to take any row locks. Bug fixed :bug:`1064326` (*Mark Callaghan*, *Laurynas Biveinis*).

  Fake changes were increasing the changed row and userstat counters. Bug fixed :bug:`1064333` (*Laurynas Biveinis*).

  Log tracking was initialized too late during the |InnoDB| startup.  Bug fixed :bug:`1076892` (*Laurynas Biveinis*).

  Debuginfo *Debian* packages have been added for |Percona Server|. Bugs fixed :bug:`711062` and  :bug:`1043873` (*Ignacio Nin*).

  There is no need to scan buffer pool for AHI entries after the B-trees for the tablespace have been dropped, as that will already clean them. Bug fixed :bug:`1076215` (*Laurynas Biveinis*).

  :ref:`slow_extended` code did not handle the case of individual statements in stored procedures correctly. This caused ``Query_time`` to increase for every query stored procedure logged to the slow query log. Bug fixed :bug:`719386` (*Alexey Kopytov*).

Other bug fixes: bug fixed :bug:`890404` (*Laurynas Biveinis*), bug fixed :bug:`1071877` (*Laurynas Biveinis*), bug fixed :bug:`1050466` (*Laurynas Biveinis*).
