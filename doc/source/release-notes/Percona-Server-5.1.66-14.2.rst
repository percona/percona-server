.. rn:: 5.1.66-14.2

==============================
 |Percona Server| 5.1.66-14.2 
==============================

Percona is glad to announce the release of |Percona Server| 5.1.66-14.2 on January 15th, 2013 (Downloads are available from `Percona Server 5.1.66-14.2 downloads <http://www.percona.com/downloads/Percona-Server-5.1/Percona-Server-5.1.66-14.2/>`_ and from the `Percona Software Repositories <http://www.percona.com/doc/percona-server/5.1/installation.html>`_).

Based on `MySQL 5.1.66 <http://dev.mysql.com/doc/refman/5.1/en/news-5-1-66.html>`_, this release will include all the bug fixes in it. All of |Percona|'s software is open-source and free, all the details of the release can be found in the `5.1.66-14.2 milestone at Launchpad <https://launchpad.net/percona-server/+milestone/5.1.66-14.2>`_.

New Features
============

  Multiple bitmap file support for :ref:`changed_page_tracking` has been implemented.

  :ref:`handlersocket_page` has been updated to version 1.1.0 (rev. 83d8f3af176e1698acd9eb3ac5174700ace40fe0).

  :ref:`innodb_fast_checksum_page` feature has now been deprecated.

  :ref:`innodb_fake_changes_page` has been improved by fetching the sibling pages.

Bug Fixes
=========

  Fixed the upstream bug :mysqlbug:`66550` and the security vulnerability `CVE-2012-4414 <http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2012-4414>`_. This was caused because user-supplied identifiers (table names, field names, etc.) are not always properly quoted, so authorized users that have privileges to modify a table (any non-temporary table) can inject arbitrary SQL into the binary log and that could cause multiple SQL injection like vulnerabilities. This bug fix comes originally from MariaDB (see `MDEV-382 <https://mariadb.atlassian.net/browse/MDEV-382>`_). Bug fixed :bug:`1049871` (*Vlad Lesin*).

  Fixed the upstream bug :mysqlbug:`67685` and the security vulnerability `CVE-2012-5611 <http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2012-5611>`_. This vulnerability allowed remote authenticated users to execute arbitrary code via a long argument to the ``GRANT FILE`` command. This bug fix comes originally from MariaDB (see `MDEV-3884 <https://mariadb.atlassian.net/browse/MDEV-3884>`_). Bug fixed :bug:`1083377` (*Vlad Lesin*).

  ``Rows_read`` was calculated in a way which lead to a negative value being printed in the slow query log. Fixed by making ``Rows_read`` to be a synonym for ``Rows_examined`` in the slow query log. Bug fixed :bug:`830286` (*Alexey Kopytov*).

  Fixed the package dependencies for CentOS 6, that caused conflicts during the install. Bug fixed :bug:`908620` (*Ignacio Nin*).

  :variable:`innodb_fake_changes` would allocate too many extents on ``UPDATE``. In some cases this could cause infinite loop. Bug fixed :bug:`917942` (*Mark Callaghan*, *Laurynas Biveinis*).

  Crash-resistant replication would break with binlog XA transaction recovery. If a crash would happened between XA PREPARE and COMMIT stages, the prepared |InnoDB| transaction would not have the slave position recorded and thus would fail to update it once it is replayed during binlog crash recovery. Bug fixed :bug:`1012715` (*Laurynas Biveinis*).

  Although fake change transactions downgrade the requested exclusive (X) row locks to shared (S) locks, these S locks prevent X locks from being taken and block the real changes. This fix introduces a new option :variable:`innodb_locking_fake_changes` which, when set to ``FALSE``, makes fake transactions not to take any row locks. Bug fixed :bug:`1064326` (*Mark Callaghan*, *Laurynas Biveinis*).

  Fake changes were increasing the changed row and userstat counters. Bug fixed :bug:`1064333` (*Laurynas Biveinis*).

  Log tracking was initialized too late during the |InnoDB| startup.  Bug fixed :bug:`1076892` (*Laurynas Biveinis*).

  Temporary files created by binary log cache were not purged after transaction commit. Fixed by truncating the temporary file, if used for a binary log transaction cache, when committing or rolling back a statement or a transaction. Bug fixed :bug:`1070856` (*Alexey Kopytov*).

  There is no need to scan buffer pool for AHI entries after the B-trees for the tablespace have been dropped, as that will already clean them. Bug fixed :bug:`1076215` (*Laurynas Biveinis*).

  When :command:`mysqldump` was used with :option:`--innodb-optimize-keys`, it  did not handle composite indexes correctly when verifying if the optimization is applicable with respect to ``AUTO_INCREMENT`` columns. Bug fixed :bug:`1039536` (*Alexey Kopytov*).

  In cases where indexes with ``AUTO_INCREMENT`` columns where correctly detected, :command:`mysqldump` prevented all such keys from optimization, even though it is sufficient to skip just one (e.g. the first one). Bug fixed :bug:`1081003` (*Alexey Kopytov*).

  :ref:`slow_extended` code did not handle the case of individual statements in stored procedures correctly, this caused ``Query_time`` to increase for every query stored procedure logged to the slow query log. Bug fixed :bug:`719386` (*Alexey Kopytov*).

Other bug fixes: bug fixed :bug:`901060` (*Laurynas Biveinis*), bug fixed :bug:`1071877` (*Laurynas Biveinis*), bug fixed :bug:`1090596` (*Stewart Smith*),  bug fixed :bug:`1050466` (*Laurynas Biveinis*), bug fixed :bug:`890404` (*Laurynas Biveinis*), bug fixed :bug:`1061118` (*Hrvoje Matijakovic*).

